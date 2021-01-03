// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#include <string.h>
#include <aia.h>
#include <aws_iot_version.h>
#include <aws_iot_mqtt_client_interface.h>
#include <aws_iot_shadow_interface.h>
#include <va_mem_utils.h>
#include "app_aws_iot.h"

/* NOTE: By default MQTT is used, thing shadow is not used.
To use Thing shadow, set APP_AWS_IOT_USE_SHADOW to 1. Then you can use the APIs below.
Also make sure you call ais_shadow_init() instead of ais_mqtt_init() from app_main.c. */
#define APP_AWS_IOT_USE_SHADOW 0

static const char *TAG = "[app_aws_iot]";

static bool app_aws_iot_init_done = false;
static AWS_IoT_Client *client = NULL;
static char *app_aws_iot_thing_name = NULL;

void app_aws_iot_set_thing_name(char *thing_name)
{
    app_aws_iot_thing_name = thing_name;
}

char *app_aws_iot_get_thing_name()
{
    return app_aws_iot_thing_name;
}

#if APP_AWS_IOT_USE_SHADOW  /* APP_AWS_IOT_USE_SHADOW == 1 */

#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 200

static bool shadow_update_flag = false;
static jsonStruct_t *test_thing;
static bool test_thing_state = true;

int app_aws_iot_shadow_update_handler();

/*
static void app_aws_iot_thing_callback_handler(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext)
{
    if (JsonStringDataLen > 0) {
        printf("%s: Thing callback. Received data: %.*s\n", TAG, JsonStringDataLen, pJsonString);
    } else {
        printf("%s: Thing callback. No data received.\n", TAG);
    }
    if(pContext != NULL) {
        printf("%s: Thing state changed to: %d\n", TAG, *(bool *)pContext->pData);
    }
    app_aws_iot_shadow_update_handler();
}

static void app_aws_iot_shadow_status_callback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status, const char *pReceivedJsonDocument, void *pContextData)
{
    if(status == SHADOW_ACK_TIMEOUT) {
        ESP_LOGE(TAG, "Update timed out");
    } else if(status == SHADOW_ACK_REJECTED) {
        ESP_LOGE(TAG, "Update rejected");
    } else if(status == SHADOW_ACK_ACCEPTED) {
        printf("%s: Update accepted for thing: %s", TAG, pThingName);
        if (pReceivedJsonDocument) {
            printf(". Received data: %s", pReceivedJsonDocument);
        }
        printf("\n");
    } else {
        ESP_LOGE(TAG, "Update status: %d", status);
    }
}
*/

void app_aws_iot_set_shadow_update()
{
    shadow_update_flag = true;
}

int app_aws_iot_shadow_register_delta()
{
    if (alexa_mqtt_is_connected() == false) {
        return -1;
    }
    client = alexa_mqtt_get_client();

    /* Perform Resgistration */
    /*
    IoT_Error_t rc = FAILURE;
    test_thing = (jsonStruct_t *)va_mem_alloc(sizeof(jsonStruct_t), VA_MEM_EXTERNAL);
    test_thing->pKey = "test_thing_state";
    test_thing->pData = &test_thing_state;
    test_thing->dataLength = sizeof(bool);
    test_thing->type = SHADOW_JSON_BOOL;
    test_thing->cb = app_aws_iot_thing_callback_handler;
    printf("%s: Registering delta: %s\n", TAG, test_thing->pKey);

    rc = aws_iot_shadow_register_delta(client, test_thing);
    if (rc != SUCCESS) {
        return -1;
    }
    */
    return 0;
}

int app_aws_iot_shadow_update_handler()
{
    if (alexa_mqtt_is_connected() == false) {
        return -1;
    }
    client = alexa_mqtt_get_client();

    /* Perform Update */
    /*
    IoT_Error_t rc = FAILURE;
    char send_buf[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
    size_t send_buf_size = sizeof(send_buf) / sizeof(send_buf[0]);

    rc = aws_iot_shadow_init_json_document(send_buf, send_buf_size);
    if(rc != SUCCESS) {
    ESP_LOGE(TAG, "Error in inittialising json document: %d", rc);
        return -1;
    }
    rc = aws_iot_shadow_add_reported(send_buf, send_buf_size, 1, test_thing);
    if(rc != SUCCESS) {
        ESP_LOGE(TAG, "Error in adding reported: %d", rc);
        return -1;
    }
    rc = aws_iot_finalize_json_document(send_buf, send_buf_size);
    if(rc != SUCCESS) {
        ESP_LOGE(TAG, "Error in finalising json document: %d", rc);
        return -1;
    }

    if (app_aws_iot_thing_name == NULL) {
        app_aws_iot_set_thing_name("test_thing");
        ESP_LOGE(TAG, "thing_name is NULL. Setting default thing_name as: %s", app_aws_iot_get_thing_name());
    }
    printf("%s: Updating thing: %s. Sending data: %s\n", TAG, app_aws_iot_thing_name, send_buf);
    rc = aws_iot_shadow_update(client, app_aws_iot_thing_name, send_buf, app_aws_iot_shadow_status_callback, NULL, 4, true);
    if (rc != SUCCESS) {
        ESP_LOGE(TAG, "Error in updating: %d", rc);
        return -1;
    }
    */
    return 0;
}

#else  /* APP_AWS_IOT_USE_SHADOW == 0 */

static bool publish_flag = false;

/*
static void app_aws_iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData)
{
    if (params->payloadLen > 0) {
        printf("%s: Subscription callback for topic: %.*s. Data received: %.*s\n", TAG, topicNameLen, topicName, params->payloadLen, (char *)params->payload);
    } else {
        printf("%s: Subsrciption callback for topic: %.*s. No data received.\n", TAG, topicNameLen, topicName);
    }
}
*/

void app_aws_iot_set_publish()
{
    publish_flag = true;
}

int app_aws_iot_subscribe_handler()
{
    if (alexa_mqtt_is_connected() == false) {
        return -1;
    }
    client = alexa_mqtt_get_client();

    /* Perform Subscribe */
    /*
    IoT_Error_t rc = FAILURE;
    char *topic = "test_topic";
    printf("%s: Subscribing to topic: %s\n", TAG, topic);

    rc = aws_iot_mqtt_subscribe(client, topic, strlen(topic), QOS0, app_aws_iot_subscribe_callback_handler, NULL);
    if (rc != SUCCESS) {
        return -1;
    }
    */
    return 0;
}

int app_aws_iot_publish_handler()
{
    if (alexa_mqtt_is_connected() == false) {
        return -1;
    }
    client = alexa_mqtt_get_client();

    /* Perform Publish */
    /*
    IoT_Error_t rc = FAILURE;
    char *topic = "test_topic";
    char *data = "test_data";
    uint32_t datalen = strlen(data);
    printf("%s: Publishing to topic: %s. Data: %.*s\n", TAG, topic, datalen, data);

    IoT_Publish_Message_Params params;
    params.qos = QOS0;
    params.isRetained = 0;
    params.payload = data;
    params.payloadLen = datalen;

    rc = aws_iot_mqtt_publish(client, topic, strlen(topic), &params);
    if (rc != SUCCESS) {
        return -1;
    }
    */
    return 0;
}

#endif /* APP_AWS_IOT_USE_SHADOW */

void app_aws_iot_init()
{
#if APP_AWS_IOT_USE_SHADOW /* APP_AWS_IOT_USE_SHADOW  == 1 */
    if (app_aws_iot_shadow_register_delta() != 0) {
        ESP_LOGE(TAG, "Error registering delta");
    }
#else /* APP_AWS_IOT_USE_SHADOW == 0 */
    /* NOTE: Subscription needs to happen only once. */
    if (app_aws_iot_subscribe_handler() != 0) {
        ESP_LOGE(TAG, "Error subscribing");
    }
#endif /* APP_AWS_IOT_USE_SHADOW */
}

void app_aws_iot_callback()
{
    if (alexa_mqtt_is_connected() == false) {
        return;
    }
    if (app_aws_iot_init_done == false) {
        app_aws_iot_init();
        app_aws_iot_init_done = true;
    }

#if APP_AWS_IOT_USE_SHADOW /* APP_AWS_IOT_USE_SHADOW == 1 */
    if (shadow_update_flag) {
        if (app_aws_iot_shadow_update_handler() == 0) {
            shadow_update_flag = false;
        }
    }
#else /* APP_AWS_IOT_USE_SHADOW == 0 */
    if (publish_flag) {
        if (app_aws_iot_publish_handler() == 0) {
            publish_flag = false;
        }
    }
#endif /* APP_AWS_IOT_USE_SHADOW */
}
