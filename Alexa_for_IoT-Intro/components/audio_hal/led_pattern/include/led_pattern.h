// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#ifndef _LED_PATTERN_H_
#define _LED_PATTERN_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int led_state_delay;
    uint32_t led_state_val[12];
    bool led_state_loop;
} led_pattern_state_t;

typedef struct {
    uint8_t led_states_count;
    led_pattern_state_t *led_states;
} led_pattern_config_t;

typedef enum {
    LED_PATTERN_BOOTUP_1 = 0,
    LED_PATTERN_BOOTUP_2,
    LED_PATTERN_WW_ACTIVE,
    LED_PATTERN_WW_ONGOING,
    LED_PATTERN_WW_DEACTIVATE,
    LED_PATTERN_SPEAKER_VOL,
    LED_PATTERN_SPEAKER_MUTE,
    LED_PATTERN_SPEAKING,
    LED_PATTERN_MIC_OFF_END,
    LED_PATTERN_MIC_OFF_ON,
    LED_PATTERN_MIC_OFF_START,
    LED_PATTERN_ERROR,
    LED_PATTERN_BTCONNECT,
    LED_PATTERN_BTDISCONNECT,
    LED_PATTERN_NTF_QUEUED,
    LED_PATTERN_NTF_INCOMING,
    LED_PATTERN_THINKING,
    LED_PATTERN_ALERT_SHORT,
    LED_PATTERN_ALERT,
    LED_PATTERN_FACTORY_RESET,
    LED_PATTERN_OFF,
    LED_PATTERN_DND,
    LED_PATTERN_OTA,
    LED_PATTERN_PATTERN_MAX,
} led_pattern_t;

void led_pattern_init(led_pattern_config_t **led_pattern_config);

#endif /* _LED_PATTERN_H_ */
