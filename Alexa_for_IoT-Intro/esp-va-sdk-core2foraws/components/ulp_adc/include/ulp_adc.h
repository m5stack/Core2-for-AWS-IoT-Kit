#ifndef _ULP_ADC_H_
#define _ULP_ADC_H_

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <esp_err.h>

typedef struct {
#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
    uint16_t abs_thres_l;
    uint16_t abs_thres_h;
#endif
    uint16_t rel_thres;
    uint32_t wakeup_period;
    QueueHandle_t ulp_wakeup_queue;
} ulp_adc_config_t;

typedef struct {
    int rel_adc_reading;
    int rel_adc_prev_reading;
#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
    int abs_adc_reading;
    int abs_adc_prev_reading;
#endif
} ulp_adc_read_t;


esp_err_t ulp_adc_setup(ulp_adc_config_t *ulp_adc_conf);

#endif /* _ULP_ADC_H_ */
