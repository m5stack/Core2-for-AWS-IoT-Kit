#pragma once
#include "esp_log.h"

typedef struct _rtc_data_t {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} rtc_date_t;

// Clear irq, Init 
void BM8563_Init();

void BM8563_SetTime(rtc_date_t* data);

void BM8563_GetTime(rtc_date_t* data);

// -1: disable
void BM8563_SetDateIRQ(int8_t minute, int8_t hour, int8_t day, int8_t week);

// sec, max time is 255 * 60
int16_t BM8563_SetTimerIRQ(int16_t value);

// sec, get timer reg value
int16_t BM8563_GetTimerTime();

// get irq status
uint8_t BM8563_GetIRQ();

// clear irq status
void BM8563_ClearIRQ();
