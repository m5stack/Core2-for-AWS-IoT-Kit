/*
 * AWS IoT EduKit - Smart Thermostat v1.0.0
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Additions Copyright 2016 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
/**
 * @file main.c
 * @brief simple MQTT publish, subscribe, and device shadows for use with AWS IoT EduKit reference hardware.
 *
 * This example takes the parameters from the build configuration and establishes a connection to AWS IoT Core over MQTT.
 *
 * Some configuration is required. Visit https://edukit.workshop.aws
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/i2s.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "core2forAWS.h"
#include "lvgl/lvgl.h"
#include "ft6336u.h"
#include "microphone.h"
#include "mpu6886.h"
#include "axp192.h"
#include "cryptoauthlib.h"
#include "i2c_device.h"
#include "atecc608.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"

#include "display.h"
#include "fft.h"

static const char *TAG = "Smart_Spaces";

#define HEATING "HEATING"
#define COOLING "COOLING"
#define STANDBY "STANDBY"

#define STARTING_ROOMTEMPERATURE 0.0f
#define STARTING_SOUNDLEVEL 0x00
#define STARTING_HVACSTATUS STANDBY
#define STARTING_ROOMOCCUPANCY false

// Number of slices to split the microphone sample into
#define AUDIO_TIME_SLICES 60

#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 200

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event */
const int CONNECTED_BIT = BIT0;
const int DISCONNECTED_BIT = BIT1;

// in disp_spi.c
extern SemaphoreHandle_t xGuiSemaphore;
extern SemaphoreHandle_t spi_mutex;
// Used for reading/writing from SD_Card
extern void spi_poll();

/* CA Root certificate, device ("Thing") certificate and device
 * ("Thing") key.

   Example can be configured one of three ways:

   "Embedded Certs" are loaded from files in "certs/" and embedded into the app binary.

   "Filesystem Certs" are loaded from the filesystem (SD card, etc.)

   See example README for more details.
*/
#if defined(CONFIG_AWS_IOT_USE_HARDWARE_SECURE_ELEMENT)

extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");

#elif defined(CONFIG_EXAMPLE_EMBEDDED_CERTS)

extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t certificate_pem_crt_end[] asm("_binary_certificate_pem_crt_end");
extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_private_pem_key_end");

#elif defined(CONFIG_EXAMPLE_FILESYSTEM_CERTS)

static const char * DEVICE_CERTIFICATE_PATH = CONFIG_EXAMPLE_CERTIFICATE_PATH;
static const char * DEVICE_PRIVATE_KEY_PATH = CONFIG_EXAMPLE_PRIVATE_KEY_PATH;
static const char * ROOT_CA_PATH = CONFIG_EXAMPLE_ROOT_CA_PATH;

#else
#error "Invalid method for loading certs"
#endif

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[255] = AWS_IOT_MQTT_HOST;

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
uint32_t port = AWS_IOT_MQTT_PORT;

/*
Semaphore for sound levels
*/
SemaphoreHandle_t xMaxNoiseSemaphore;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        display_wifi_label_update(true);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, DISCONNECTED_BIT);
        display_wifi_label_update(false);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData) {
    ESP_LOGI(TAG, "Subscribe callback");
    ESP_LOGI(TAG, "%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);
}

void disconnect_callback_handler(AWS_IoT_Client *pClient, void *data) {
    ESP_LOGW(TAG, "MQTT Disconnect");
    display_textarea_add("Disconnected from AWS IoT Core...", NULL, NULL);

    IoT_Error_t rc = FAILURE;

    if(NULL == pClient) {
        return;
    }

    if(aws_iot_is_autoreconnect_enabled(pClient)) {
        ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
    } else {
        ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if(NETWORK_RECONNECTED == rc) {
            ESP_LOGW(TAG, "Manual Reconnect Successful");
        } else {
            ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
        }
    }
}

static bool shadowUpdateInProgress;

void ShadowUpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
                                const char *pReceivedJsonDocument, void *pContextData) {
    IOT_UNUSED(pThingName);
    IOT_UNUSED(action);
    IOT_UNUSED(pReceivedJsonDocument);
    IOT_UNUSED(pContextData);

    shadowUpdateInProgress = false;

    if(SHADOW_ACK_TIMEOUT == status) {
        ESP_LOGE(TAG, "Update timed out");
    } else if(SHADOW_ACK_REJECTED == status) {
        ESP_LOGE(TAG, "Update rejected");
    } else if(SHADOW_ACK_ACCEPTED == status) {
        ESP_LOGI(TAG, "Update accepted");
    }
}

void hvac_Callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext) {
    IOT_UNUSED(pJsonString);
    IOT_UNUSED(JsonStringDataLen);

    char * status = (char *) (pContext->pData);

    if(pContext != NULL) {
        ESP_LOGI(TAG, "Delta - hvacStatus state changed to %s", status);
    }

    if(strcmp(status, HEATING) == 0) {
        ESP_LOGI(TAG, "setting side LEDs to red");
        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT, 0xFF0000);
        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, 0xFF0000);
        Core2ForAWS_Sk6812_Show();
    } else if(strcmp(status, COOLING) == 0) {
        ESP_LOGI(TAG, "setting side LEDs to blue");
        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT, 0x0000FF);
        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, 0x0000FF);
        Core2ForAWS_Sk6812_Show();
    } else {
        ESP_LOGI(TAG, "clearing side LEDs");
        Core2ForAWS_Sk6812_Clear();
        Core2ForAWS_Sk6812_Show();
    }
}

void occupancy_Callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext) {
    IOT_UNUSED(pJsonString);
    IOT_UNUSED(JsonStringDataLen);

    if(pContext != NULL) {
        ESP_LOGI(TAG, "Delta - roomOccupancy state changed to %d", *(bool *) (pContext->pData));
    }
}

float temperature = STARTING_ROOMTEMPERATURE;
uint8_t soundBuffer = STARTING_SOUNDLEVEL;
uint8_t reportedSound = STARTING_SOUNDLEVEL;
char hvacStatus[7] = STARTING_HVACSTATUS;
bool roomOccupancy = STARTING_ROOMOCCUPANCY;


// helper function for working with audio data
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    long divisor = (in_max - in_min);
    if(divisor == 0){
        return -1; //AVR returns -1, SAM returns 0
    }
    return (x - in_min) * (out_max - out_min) / divisor + out_min;
}

void microphone_task(void *arg) {
    static int8_t i2s_readraw_buff[1024];
    size_t bytesread;
    int16_t *buffptr;
    double data = 0;

    Microphone_Init();
    uint8_t maxSound = 0x00;
    uint8_t currentSound = 0x00;

    for (;;) {
        maxSound = 0x00;
        fft_config_t *real_fft_plan = fft_init(512, FFT_REAL, FFT_FORWARD, NULL, NULL);
        i2s_read(I2S_NUM_0, (char *)i2s_readraw_buff, 1024, &bytesread, (100 / portTICK_RATE_MS));
        buffptr = (int16_t *)i2s_readraw_buff;
        for (uint16_t count_n = 0; count_n < real_fft_plan->size; count_n++) {
            real_fft_plan->input[count_n] = (float)map(buffptr[count_n], INT16_MIN, INT16_MAX, -1000, 1000);
        }
        fft_execute(real_fft_plan);

        for (uint16_t count_n = 1; count_n < AUDIO_TIME_SLICES; count_n++) {
            data = sqrt(real_fft_plan->output[2 * count_n] * real_fft_plan->output[2 * count_n] + real_fft_plan->output[2 * count_n + 1] * real_fft_plan->output[2 * count_n + 1]);
            currentSound = map(data, 0, 2000, 0, 256);
            if(currentSound > maxSound) {
                maxSound = currentSound;
            }
        }
        fft_destroy(real_fft_plan);

        // store max of sample in semaphore
        xSemaphoreTake(xMaxNoiseSemaphore, portMAX_DELAY);
        soundBuffer = maxSound;
        xSemaphoreGive(xMaxNoiseSemaphore);
    }
}

void aws_iot_task(void *param) {
    IoT_Error_t rc = FAILURE;

    char JsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
    size_t sizeOfJsonDocumentBuffer = sizeof(JsonDocumentBuffer) / sizeof(JsonDocumentBuffer[0]);

    jsonStruct_t temperatureHandler;
    temperatureHandler.cb = NULL;
    temperatureHandler.pKey = "temperature";
    temperatureHandler.pData = &temperature;
    temperatureHandler.type = SHADOW_JSON_FLOAT;
    temperatureHandler.dataLength = sizeof(float);

    jsonStruct_t soundHandler;
    soundHandler.cb = NULL;
    soundHandler.pKey = "sound";
    soundHandler.pData = &reportedSound;
    soundHandler.type = SHADOW_JSON_UINT8;
    soundHandler.dataLength = sizeof(uint8_t);

    jsonStruct_t hvacStatusActuator;
    hvacStatusActuator.cb = hvac_Callback;
    hvacStatusActuator.pKey = "hvacStatus";
    hvacStatusActuator.pData = &hvacStatus;
    hvacStatusActuator.type = SHADOW_JSON_STRING;
    hvacStatusActuator.dataLength = strlen(hvacStatus)+1;

    jsonStruct_t roomOccupancyActuator;
    roomOccupancyActuator.cb = occupancy_Callback;
    roomOccupancyActuator.pKey = "roomOccupancy";
    roomOccupancyActuator.pData = &roomOccupancy;
    roomOccupancyActuator.type = SHADOW_JSON_BOOL;
    roomOccupancyActuator.dataLength = sizeof(bool);

    ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    // initialize the mqtt client
    AWS_IoT_Client iotCoreClient;

    ShadowInitParameters_t sp = ShadowInitParametersDefault;
    sp.pHost = HostAddress;
    sp.port = port;
    sp.enableAutoReconnect = false;
    sp.disconnectHandler = disconnect_callback_handler;

#if defined(CONFIG_AWS_IOT_USE_HARDWARE_SECURE_ELEMENT)
    sp.pRootCA = (const char *)aws_root_ca_pem_start;
    sp.pClientCRT = "#";
    sp.pClientKey = "#0";
    
    #define CLIENT_ID_LEN (ATCA_SERIAL_NUM_SIZE * 2 + 1)
    char * clientId = malloc(CLIENT_ID_LEN);
    ATCA_STATUS ret = Atecc608_GetSerialString(clientId);
    if (ret != ATCA_SUCCESS){
        ESP_LOGE(TAG, "Failed to get device serial from secure element. Error: %i", ret);
        abort();
    }
    #define CLIENT_ID clientId

#elif defined(CONFIG_EXAMPLE_EMBEDDED_CERTS)
    
    sp.pClientCRT = (const char *)certificate_pem_crt_start;
    sp.pClientKey = (const char *)private_pem_key_start;
    sp.pRootCA = (const char *)aws_root_ca_pem_start;
    #define CLIENT_ID CONFIG_AWS_EXAMPLE_CLIENT_ID
    #define CLIENT_ID_LEN strlen(CLIENT_ID)

#elif defined(CONFIG_EXAMPLE_FILESYSTEM_CERTS)
    
    sp.pClientCRT = DEVICE_CERTIFICATE_PATH;
    sp.pClientKey = DEVICE_PRIVATE_KEY_PATH;
    sp.pRootCA = ROOT_CA_PATH;
    #define CLIENT_ID CONFIG_AWS_EXAMPLE_CLIENT_ID
    #define CLIENT_ID_LEN strlen(CLIENT_ID)

#endif

display_textarea_add("\n\nDevice client Id:\n>> %s <<\n", CLIENT_ID, (size_t *) CLIENT_ID_LEN);

#ifdef CONFIG_EXAMPLE_SDCARD_CERTS
    ESP_LOGI(TAG, "Mounting SD card...");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 3,
    };
    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(ret));
        abort();
    }
#endif

    /* Wait for WiFI to show as connected */
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);

    ESP_LOGI(TAG, "Shadow Init");


    rc = aws_iot_shadow_init(&iotCoreClient, &sp);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_shadow_init returned error %d, aborting...", rc);
        abort();
    }

    ShadowConnectParameters_t scp = ShadowConnectParametersDefault;
    scp.pMyThingName = CLIENT_ID;
    scp.pMqttClientId = CLIENT_ID;
    scp.mqttClientIdLen = (uint16_t) strlen(CLIENT_ID);

    ESP_LOGI(TAG, "Shadow Connect");
    rc = aws_iot_shadow_connect(&iotCoreClient, &scp);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_shadow_connect returned error %d, aborting...", rc);
        abort();
    }

    xTaskCreatePinnedToCore(&microphone_task, "microphone_task", 4096, NULL, 1, NULL, 1);

    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    rc = aws_iot_shadow_set_autoreconnect_status(&iotCoreClient, true);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d, aborting...", rc);
        abort();
    }

    // register delta callback for roomOccupancy
    rc = aws_iot_shadow_register_delta(&iotCoreClient, &roomOccupancyActuator);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Shadow Register Delta Error");
    }

    // register delta callback for hvacStatus
    rc = aws_iot_shadow_register_delta(&iotCoreClient, &hvacStatusActuator);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Shadow Register Delta Error");
    }

    // loop and publish changes
    while(NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc) {
        rc = aws_iot_shadow_yield(&iotCoreClient, 200);
        if(NETWORK_ATTEMPTING_RECONNECT == rc || shadowUpdateInProgress) {
            rc = aws_iot_shadow_yield(&iotCoreClient, 1000);
            // If the client is attempting to reconnect, or already waiting on a shadow update,
            // we will skip the rest of the loop.
            continue;
        }

        // START get sensor readings
        // sample temperature, convert to fahrenheit
        MPU6886_GetTempData(&temperature);
        temperature = (temperature * 1.8)  + 32 - 50;

        // sample from soundBuffer (latest reading from microphone)
        xSemaphoreTake(xMaxNoiseSemaphore, portMAX_DELAY);
        reportedSound = soundBuffer;
        xSemaphoreGive(xMaxNoiseSemaphore);

        // END get sensor readings

        ESP_LOGI(TAG, "=======================================================================================");
        ESP_LOGI(TAG, "On Device: roomOccupancy %s", roomOccupancy ? "true" : "false");
        ESP_LOGI(TAG, "On Device: hvacStatus %s", hvacStatus);
        ESP_LOGI(TAG, "On Device: temperature %f", temperature);
        ESP_LOGI(TAG, "On Device: sound %d", reportedSound);

        rc = aws_iot_shadow_init_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
        if(SUCCESS == rc) {
            rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 4, &temperatureHandler,
                                             &soundHandler, &roomOccupancyActuator, &hvacStatusActuator);
            if(SUCCESS == rc) {
                rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
                if(SUCCESS == rc) {
                    ESP_LOGI(TAG, "Update Shadow: %s", JsonDocumentBuffer);
                    rc = aws_iot_shadow_update(&iotCoreClient, CLIENT_ID, JsonDocumentBuffer,
                                               ShadowUpdateStatusCallback, NULL, 4, true);
                    shadowUpdateInProgress = true;
                }
            }
        }
        ESP_LOGI(TAG, "*****************************************************************************************");
        ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));

        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "An error occurred in the loop %d", rc);
    }

    ESP_LOGI(TAG, "Disconnecting");
    rc = aws_iot_shadow_disconnect(&iotCoreClient);

    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Disconnect error %d", rc);
    }

    vTaskDelete(NULL);
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void app_main()
{

    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    spi_mutex = xSemaphoreCreateMutex();
    xMaxNoiseSemaphore = xSemaphoreCreateMutex();
    Core2ForAWS_Init();
    MPU6886_Init();
    FT6336U_Init();
    Core2ForAWS_LCD_Init();
    Core2ForAWS_Button_Init();
    Core2ForAWS_Sk6812_Init();
    Core2ForAWS_Sk6812_Clear();
    Core2ForAWS_Sk6812_Show();
    Core2ForAWS_LCD_SetBrightness(80);
    Core2ForAWS_LED_Enable(0x01);
    
    display_init();
    
    initialise_wifi();

    #if defined (CONFIG_AWS_IOT_USE_HARDWARE_SECURE_ELEMENT)
    
    ATCA_STATUS ret = Atecc608_Init();
    if (ret != ATCA_SUCCESS){
        ESP_LOGE(TAG, "ATECC608 secure element initialization error");
        abort();
    }

    #endif
    
    xTaskCreatePinnedToCore(&aws_iot_task, "aws_iot_task", 4096*2, NULL, 5, NULL, 1);
    
}
