#include "stdbool.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/spi_common.h"
#include "esp_idf_version.h"

#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"

#include "core2forAWS.h"

#if CONFIG_SOFTWARE_EXPPORTS_SUPPORT
#include <driver/adc.h>
#include <driver/dac.h>
#include "esp_adc_cal.h"
#include "soc/dac_channel.h"

#define DEFAULT_VREF    1100
static esp_adc_cal_characteristics_t *adc_characterization;

#define ADC_CHANNEL ADC1_CHANNEL_0
#define ADC_WIDTH ADC_WIDTH_BIT_12
#define ADC_ATTENUATION ADC_ATTEN_DB_11
#define DAC_CHANNEL DAC_GPIO26_CHANNEL
#endif

static const char *TAG = "Core2forAWS";

void Core2ForAWS_Init(void) {    
#if CONFIG_SOFTWARE_ILI9342C_SUPPORT || CONFIG_SOFTWARE_SDCARD_SUPPORT
    spi_mutex = xSemaphoreCreateMutex();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = 23,
        .miso_io_num = 38,
        .sclk_io_num = 18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 320 * 32 * 3,
    };
    spi_bus_initialize(SPI_HOST_USE, &bus_cfg, SPI_DMA_CHAN);
#endif

#if CONFIG_SOFTWARE_ILI9342C_SUPPORT
    Core2ForAWS_PMU_Init(3300, 0, 0, 2700);
    Core2ForAWS_Display_Init();
#else
    Core2ForAWS_PMU_Init(0, 0, 0, 0);
#endif

#if CONFIG_SOFTWARE_FT6336U_SUPPORT
    FT6336U_Init();
#endif
    
#if CONFIG_SOFTWARE_BUTTON_SUPPORT
    Core2ForAWS_Button_Init();
#endif

#if CONFIG_SOFTWARE_SK6812_SUPPORT
    Core2ForAWS_Sk6812_Init();
#endif

#if CONFIG_SOFTWARE_MPU6886_SUPPORT
    MPU6886_Init();
#endif

#if CONFIG_SOFTWARE_RTC_SUPPORT
    BM8563_Init();
#endif

#if CONFIG_SOFTWARE_ATECC608_SUPPORT
    ATCA_STATUS ret = Atecc608_Init();
    if (ret != ATCA_SUCCESS){
        ESP_LOGE(TAG, "ATECC608 secure element initialization error!");
        abort();
    }
#endif
}

/* ===================================================================================================*/
/* --------------------------------------------- BUTTON ----------------------------------------------*/
#if CONFIG_SOFTWARE_BUTTON_SUPPORT
Button_t* button_left;
Button_t* button_middle;
Button_t* button_right;

void Core2ForAWS_Button_Init(void) {
    Button_Init();
    button_left = Button_Attach(0, 240, 106, 60);
    button_middle = Button_Attach(106, 240, 106, 60);
    button_right = Button_Attach(212, 240, 106, 60);
}
#endif
/* ----------------------------------------------- End -----------------------------------------------*/
/* ===================================================================================================*/

/* ===================================================================================================*/
/* --------------------------------------------- SDCARD ----------------------------------------------*/
#if CONFIG_SOFTWARE_SDCARD_SUPPORT
esp_err_t Core2ForAWS_SDcard_Mount(const char *mount_path, sdmmc_card_t **out_card) {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 512
    };
    sdmmc_card_t* card;

    host.slot = SPI_HOST_USE;

#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 1, 0)
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.host_id = host.slot;
#else
    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
#endif
    slot_config.gpio_cs = 4;
    esp_err_t ret;
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 1, 0)
    ret = esp_vfs_fat_sdspi_mount(mount_path, &host, &slot_config, &mount_config, &card);
#else
    ret = esp_vfs_fat_sdmmc_mount(mount_path, &host, &slot_config, &mount_config, &card);
#endif
    if (ret == ESP_OK && out_card != NULL) {
        *out_card = card;
    }
    return ret;
}

esp_err_t Core2ForAWS_SDcard_Unmount(const char *mount_path, sdmmc_card_t *out_card){
    return esp_vfs_fat_sdcard_unmount(mount_path, out_card);
}
#endif
/* ----------------------------------------------- End -----------------------------------------------*/
/* ===================================================================================================*/

void Core2ForAWS_Motor_SetStrength(uint8_t strength) {
    if (strength > 100) {
        strength = 100;
    }

    if (strength > 0){
        uint16_t volt = (uint32_t)strength * (AXP192_LDO_VOLT_MAX - AXP192_LDO_VOLT_MIN) / 100 + AXP192_LDO_VOLT_MIN;
        Axp192_SetLDO3Volt(volt);
        Axp192_EnableLDO3(1);
    } else {
        Axp192_EnableLDO3(0);
    }
}

void Core2ForAWS_Speaker_Enable(uint8_t state) {
    uint8_t value = state ? 1 : 0;
    Axp192_SetGPIO2Level(value);
}

/* ==================================================================================================*/
/* ---------------------------------------------- PMU -----------------------------------------------*/
float Core2ForAWS_PMU_GetBatVolt(void) {
    return Axp192_GetBatVolt();
}

float Core2ForAWS_PMU_GetBatCurrent(void) {
    return Axp192_GetBatCurrent();
}

void Core2ForAWS_PMU_SetPowerIn(uint8_t mode) {
    if (mode) {
        Axp192_SetGPIO0Mode(0);
        Axp192_EnableExten(0);
    } else {
        Axp192_EnableExten(1);
        Axp192_SetGPIO0Mode(1);
    }
}

void Core2ForAWS_PMU_Init(uint16_t ldo2_volt, uint16_t ldo3_volt, uint16_t dc2_volt, uint16_t dc3_volt) {
    uint8_t value = 0x00;
    value |= (ldo2_volt > 0) << AXP192_LDO2_EN_BIT;
    value |= (ldo3_volt > 0) << AXP192_LDO3_EN_BIT;
    value |= (dc2_volt > 0) << AXP192_DC2_EN_BIT;
    value |= (dc3_volt > 0) << AXP192_DC3_EN_BIT;
    value |= 0x01 << AXP192_DC1_EN_BIT;

    Axp192_Init();

    // value |= 0x01 << AXP192_EXT_EN_BIT;
    Axp192_SetLDO23Volt(ldo2_volt, ldo3_volt);
    // Axp192_SetDCDC1Volt(3300);
    Axp192_SetDCDC2Volt(dc2_volt);
    Axp192_SetDCDC3Volt(dc3_volt);
    Axp192_SetVoffVolt(3000);
    Axp192_SetChargeCurrent(CHARGE_Current_100mA);
    Axp192_SetChargeVoltage(CHARGE_VOLT_4200mV);
    Axp192_EnableCharge(1);
    Axp192_SetPressStartupTime(STARTUP_128mS);
    Axp192_SetPressPoweroffTime(POWEROFF_4S);
    Axp192_EnableLDODCExt(value);
    Axp192_SetGPIO4Mode(1);
    Axp192_SetGPIO2Mode(1);
    Axp192_SetGPIO2Level(0);

    Axp192_SetGPIO0Volt(3300);
    Axp192_SetAdc1Enable(0xfe);
    Axp192_SetGPIO1Mode(1);
    Core2ForAWS_PMU_SetPowerIn(0);
}
/* ----------------------------------------------- End -----------------------------------------------*/
/* ===================================================================================================*/

/* ===================================================================================================*/
/* --------------------------------------------- SK6812 ----------------------------------------------*/
#if CONFIG_SOFTWARE_SK6812_SUPPORT
pixel_settings_t px;

void Core2ForAWS_Sk6812_Init(void) {
    px.pixel_count = 10;
    px.brightness = 20;
    sprintf(px.color_order, "GRBW");
    px.nbits = 24;
    px.timings.t0h = (350);
    px.timings.t0l = (800);
    px.timings.t1h = (600);
    px.timings.t1l = (700);
    px.timings.reset = 80000;
    px.pixels = (uint8_t *)malloc((px.nbits / 8) * px.pixel_count);
    neopixel_init(GPIO_NUM_25, RMT_CHANNEL_0);
    np_clear(&px);
}

void Core2ForAWS_Sk6812_SetColor(uint16_t pos, uint32_t color) {
    np_set_pixel_color(&px, pos, color << 8);
}

void Core2ForAWS_Sk6812_SetSideColor(uint8_t side, uint32_t color) {
    if (side == SK6812_SIDE_LEFT) {
        for (uint8_t i = 5; i < 10; i++) {
            np_set_pixel_color(&px, i, color << 8);
        }
    } else {
        for (uint8_t i = 0; i < 5; i++) {
            np_set_pixel_color(&px, i, color << 8);
        }
    }
}

void Core2ForAWS_Sk6812_SetBrightness(uint8_t brightness) {
    px.brightness = brightness;
}

void Core2ForAWS_Sk6812_Show(void) {
    np_show(&px, RMT_CHANNEL_0);
}

void Core2ForAWS_Sk6812_Clear(void) {
    np_clear(&px);
}
#endif
/* ----------------------------------------------- End -----------------------------------------------*/
/* ===================================================================================================*/

/* ===================================================================================================*/
/* --------------------------------------------- DISPLAY ---------------------------------------------*/
#if CONFIG_SOFTWARE_ILI9342C_SUPPORT

#define DISPLAY_BRIGHTNESS_MIN_VOLT 2200
#define DISPLAY_BRIGHTNESS_MAX_VOLT 3300
#define LV_TICK_PERIOD_MS 1

SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter);
static void lv_tick_task(void *arg);

#if CONFIG_SOFTWARE_FT6336U_SUPPORT
static bool ft6336u_read(lv_indev_drv_t * drv, lv_indev_data_t * data);
#endif

void Core2ForAWS_Display_Init(void) {
    xGuiSemaphore = xSemaphoreCreateMutex();

    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    lv_init();
    
    disp_spi_add_device(SPI_HOST_USE);
    disp_driver_init();

    /* Use double buffered when not working with monochrome displays. 
	 * Application should allocate two buffers buf1 and buf2 of size
	 * (DISP_BUF_SIZE * sizeof(lv_color_t)) each
	 */

    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;
    lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); //Assuming max size of lv_color_t = 16bit, DISP_BUF_SIZE calculated from max horizontal display size 480
    lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); //Assuming max size of lv_color_t = 16bit, DISP_BUF_SIZE calculated from max horizontal display size 480
    
    /* Initialize the working buffer depending on the selected display */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Register an input device when enabled on the menuconfig */
#if CONFIG_SOFTWARE_FT6336U_SUPPORT
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = ft6336u_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    xSemaphoreGive(xGuiSemaphore);

    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 2, NULL, 1);
}

void Core2ForAWS_Display_SetBrightness(uint8_t brightness) {
    if (brightness > 100) {
        brightness = 100;
    }
    uint16_t volt = (uint32_t)brightness * (DISPLAY_BRIGHTNESS_MAX_VOLT - DISPLAY_BRIGHTNESS_MIN_VOLT) / 100 + DISPLAY_BRIGHTNESS_MIN_VOLT;
    Axp192_SetDCDC3Volt(volt);
}

void Core2ForAWS_LED_Enable(uint8_t enable) {
    uint8_t value = enable ? 0 : 1;
    Axp192_SetGPIO1Mode(value);
}

#if CONFIG_SOFTWARE_FT6336U_SUPPORT
static bool ft6336u_read(lv_indev_drv_t * drv, lv_indev_data_t * data) {
    bool valid = true;
    uint16_t x = 0;
    uint16_t y = 0;
    FT6336U_GetTouch(&x, &y, &valid);
    data->point.x = x;
    data->point.y = y;
    data->state = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
    return false;
}
#endif

static void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

/**
 * @brief The FreeRTOS task that periodically calls lv_task_handler
 * 
 * A FreeRTOS task function that calls [lv_task_handler](https://docs.lvgl.io/7.11/porting/task-handler.html)
 * after a tick (depends on task prioritization), which executes 
 * LVGL tasks to then pass to the display controller. Learn more 
 * about LVGL Tasks[https://docs.lvgl.io/7.11/overview/task.html].
 */
static void guiTask(void *pvParameter) {
    
    (void) pvParameter;


    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    /* A task should NEVER return */
    vTaskDelete(NULL);
}
#endif
/* ----------------------------------------------- End -----------------------------------------------*/
/* ===================================================================================================*/

/* ===================================================================================================*/
/* ----------------------------------------- Expansion Ports -----------------------------------------*/
#if CONFIG_SOFTWARE_EXPPORTS_SUPPORT


static esp_err_t check_pins(gpio_num_t pin, pin_mode_t mode){
    if (pin != PORT_B_ADC_PIN && pin != PORT_B_DAC_PIN && pin != PORT_C_UART_RX_PIN && pin != PORT_C_UART_TX_PIN){
        ESP_LOGE(TAG, "Only Port B (GPIO 26 and 36) is supported. Pin selected: %d", pin);
        return ESP_ERR_NOT_SUPPORTED;
    }
    if (mode == DAC && pin != PORT_B_DAC_PIN){
        ESP_LOGE(TAG, "DAC is only supported on GPIO 26.");
        return ESP_ERR_NOT_SUPPORTED;
    } 
    else if (mode == ADC && pin != PORT_B_ADC_PIN){
        ESP_LOGE(TAG, "ADC is only supported on GPIO 36.");
        return ESP_ERR_NOT_SUPPORTED;
    } else if (mode == UART && (pin != PORT_C_UART_TX_PIN && pin != PORT_C_UART_RX_PIN)){
        ESP_LOGE(TAG, "UART is only supported on GPIO 13 (receive) and GPIO 14 (transmit).");
        return ESP_ERR_NOT_SUPPORTED;
    }
    return ESP_OK;
}

esp_err_t Core2ForAWS_Port_PinMode(gpio_num_t pin, pin_mode_t mode){
    esp_err_t err = check_pins(pin, mode);
    if (err != ESP_OK){
        return err;
    }

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL<<PORT_B_DAC_PIN);

    if (mode == OUTPUT){
        io_conf.mode = GPIO_MODE_OUTPUT; 
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    }
    else if (mode == INPUT){
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    }
    else if (mode == ADC){
        err = adc1_config_width(ADC_WIDTH);
        if(err != ESP_OK){
            ESP_LOGE(TAG, "Error configuring ADC width on pin %d. Error code: %d", pin, err);
            return err;
        }
        
        err = adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTENUATION);
        if(err != ESP_OK){
            ESP_LOGE(TAG, "Error configuring ADC channel attenuation on pin %d. Error code: %d", pin, err);
        }
        
        adc_characterization = calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTENUATION, ADC_WIDTH, DEFAULT_VREF, adc_characterization);
    }
    else if (mode == DAC){
        dac_output_enable(DAC_CHANNEL);
    }
    else if (mode == UART){
        err = uart_driver_install(PORT_C_UART_NUM, UART_RX_BUF_SIZE, 0, 0, NULL, 0);
        if(err != ESP_OK){
            ESP_LOGE(TAG, "UART driver installation failed for UART num %d", PORT_C_UART_NUM);
            return err;
        }

        err = uart_set_pin(PORT_C_UART_NUM, PORT_C_UART_TX_PIN, PORT_C_UART_RX_PIN, 
        UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        if(err != ESP_OK){
            ESP_LOGE(TAG, "Failed to set pins %d, %d, to  UART %d.", PORT_C_UART_RX_PIN, PORT_C_UART_TX_PIN, PORT_C_UART_NUM);
        }
    }
    else if (mode == NONE){
        dac_output_disable(DAC_CHANNEL);
        gpio_reset_pin(pin);
        uart_driver_delete(UART_NUM_2);
    }
    else {
        ESP_LOGE(TAG, "Invalid mode selected for GPIO %d", PORT_B_DAC_PIN);
        return err;
    }
    err = gpio_config(&io_conf);//configure GPIO with the given settings
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Error configuring GPIO %d", PORT_B_DAC_PIN);
    }
    return err;
}

bool Core2ForAWS_Port_Read(gpio_num_t pin){
    ESP_ERROR_CHECK_WITHOUT_ABORT(check_pins(pin, INPUT));

    return gpio_get_level(PORT_B_DAC_PIN);
}

esp_err_t Core2ForAWS_Port_Write(gpio_num_t pin, bool level){
    esp_err_t err = check_pins(pin, OUTPUT);
    
    err = gpio_set_level(PORT_B_DAC_PIN, level);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Error setting GPIO %d state", PORT_B_DAC_PIN);
    }
    return err;
}

uint32_t Core2ForAWS_Port_B_ADC_ReadRaw(void){
    return adc1_get_raw(ADC_CHANNEL);
}

uint32_t Core2ForAWS_Port_B_ADC_ReadMilliVolts(void){
    uint32_t voltage;
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_adc_cal_get_voltage(ADC_CHANNEL, adc_characterization, &voltage));
    return voltage;
}

esp_err_t Core2ForAWS_Port_B_DAC_WriteMilliVolts(uint16_t mvolts){
    esp_err_t err = dac_output_voltage(DAC_CHANNEL, mvolts);
    return err;
}

esp_err_t Core2ForAWS_Port_C_UART_Begin(uint32_t baud){
    const uart_config_t uart_config = {
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    
    esp_err_t err = uart_param_config(PORT_C_UART_NUM, &uart_config);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to configure UART %d with the provided configuration.", PORT_C_UART_NUM);
        return err;
    }    

    return err;
}

int Core2ForAWS_Port_C_UART_Send(const char *message, size_t len){
    return uart_write_bytes(PORT_C_UART_NUM, message, len);
}

int Core2ForAWS_Port_C_UART_Receive(uint8_t *message_buffer){
    int rxBytes = 0;
    int cached_buffer_length = 0;

    esp_err_t err = uart_get_buffered_data_len(PORT_C_UART_NUM, (size_t*)&cached_buffer_length);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to get UART ring buffer length. Check if pins were set to UART and has been configured.");
        abort();
    }

    if (cached_buffer_length) {
        rxBytes = uart_read_bytes(PORT_C_UART_NUM, message_buffer, (size_t)&cached_buffer_length, pdMS_TO_TICKS(1000));
    }
    return rxBytes;
}

#endif
/* ----------------------------------------------- End -----------------------------------------------*/
/* ===================================================================================================*/