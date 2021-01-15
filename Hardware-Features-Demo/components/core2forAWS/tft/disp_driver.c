/**
 * @file disp_driver.c
 */

#include "disp_driver.h"
#include "disp_spi.h"

void disp_driver_init(void) {
    ili9341_init();
}

void disp_driver_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map) {
    ili9341_flush(drv, area, color_map);
}

