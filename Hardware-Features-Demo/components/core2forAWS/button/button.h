#pragma once

#include "stdio.h"

typedef enum {
    PRESS = (1 << 0),
    RELEASE = (1 << 1),
    LONGPRESS = (1 << 2),
} PressEvent;

typedef struct _Button_t  {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;

    uint8_t value;
    uint8_t last_value;
    uint32_t last_press_time;
    uint32_t long_press_time;
    uint8_t state;
    struct _Button_t* next;
} Button_t;

void Button_Init();
Button_t* Button_Attach(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
uint8_t Button_WasPressed(Button_t* button);
uint8_t Button_WasReleased(Button_t* button);
uint8_t Button_IsPress(Button_t* button);
uint8_t Button_IsRelease(Button_t* button);
uint8_t Button_WasLongPress(Button_t* button, uint32_t long_press_time);
