#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp_err.h>
#include <esp_log.h>
#include "driver/adc.h"
#include "ulp_adc.h"
#include <ulp_adc_button.h>
#include "esp32/ulp.h"
#include "driver/rtc_cntl.h"
#include "soc/rtc_cntl_reg.h"

#define TAG       "ulp_adc"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_adc_button_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_adc_button_bin_end");

static void ulp_intr_handle(void *arg)
{
    ulp_adc_config_t *ulp_adc_conf = (ulp_adc_config_t *)arg;

    ulp_adc_read_t ulp_adc_read = {0};
    ulp_adc_read.rel_adc_reading = ulp_rel_adc_reading & UINT16_MAX;
    ulp_adc_read.rel_adc_prev_reading = ulp_rel_adc_prev_reading & UINT16_MAX;
#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
    ulp_adc_read.abs_adc_reading = ulp_abs_adc_reading & UINT16_MAX;
    ulp_adc_read.abs_adc_prev_reading = ulp_abs_adc_prev_reading & UINT16_MAX;
    ets_printf("Button ADC: %d \t Battery ADC: %d\n", ulp_adc_read.rel_adc_reading, ulp_adc_read.abs_adc_reading);
#else
    ets_printf("Button ADC: %d\n", ulp_adc_read.rel_adc_reading);
#endif
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(ulp_adc_conf->ulp_wakeup_queue, &ulp_adc_read, &xHigherPriorityTaskWoken);
}

static esp_err_t init_ulp_program(ulp_adc_config_t *ulp_adc_conf)
{
    if (!ulp_adc_conf->ulp_wakeup_queue) {
        ESP_LOGE(TAG, "Invalid Queue handle");
    }

    /* Load ULP binary to RTC memory */
    esp_err_t err = ulp_load_binary(0, ulp_main_bin_start,
            (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
    ESP_ERROR_CHECK(err);

    err |= adc1_config_channel_atten(CONFIG_ULP_REL_ADC_CHANNEL, ADC_ATTEN_DB_11);
#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
    err |= adc1_config_channel_atten(CONFIG_ULP_ABS_ADC_CHANNEL, ADC_ATTEN_DB_11);
#endif
    err |= adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_ulp_enable();

    ulp_rel_thres = ulp_adc_conf->rel_thres & UINT16_MAX;
#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
    ulp_abs_low_level = ulp_adc_conf->abs_thres_l & UINT16_MAX;
    ulp_abs_high_level = ulp_adc_conf->abs_thres_h & UINT16_MAX;
#endif

    ulp_set_wakeup_period(0, ulp_adc_conf->wakeup_period * 1000);

    return err;
}

static esp_err_t start_ulp_program()
{
    /* Start the program */
    esp_err_t err = ulp_run(&ulp_entry - RTC_SLOW_MEM);
    ESP_ERROR_CHECK(err);
    return err;
}

esp_err_t ulp_adc_setup(ulp_adc_config_t *ulp_adc_conf)
{
    esp_err_t ret = 0;
    ret |= rtc_isr_register(ulp_intr_handle, ulp_adc_conf, RTC_CNTL_ULP_CP_INT_ENA);

    REG_SET_BIT(RTC_CNTL_INT_ENA_REG, RTC_CNTL_ULP_CP_INT_ENA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error registering ULP wakeup interrupt handler");
        return ret;
    } else {
        ret |= init_ulp_program(ulp_adc_conf);
        ret |= start_ulp_program();
        return ret;
    }
}
