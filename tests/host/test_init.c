/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../core2foraws.c"

enum { BUS, POWER, DISPLAY, BUTTON, MOTION, RTC, CRYPTO, RGB, WIFI, MODULE_COUNT };
static int calls[MODULE_COUNT];
static int call_count;
static int fail_module = -1;

static esp_err_t record_call(int module)
{
    assert(call_count < MODULE_COUNT);
    calls[call_count++] = module;
    return module == fail_module ? ESP_ERR_TIMEOUT : ESP_OK;
}

esp_err_t core2foraws_i2c_init(int bus)
{
    assert(bus == CORE2FORAWS_I2C_INTERNAL);
    return record_call(BUS);
}

#define INIT_STUB(name, module) \
    esp_err_t core2foraws_##name##_init(void) { return record_call(module); }
INIT_STUB(power, POWER)
INIT_STUB(display, DISPLAY)
INIT_STUB(button, BUTTON)
INIT_STUB(motion, MOTION)
INIT_STUB(rtc, RTC)
INIT_STUB(crypto, CRYPTO)
INIT_STUB(rgb_led, RGB)
INIT_STUB(wifi, WIFI)

esp_err_t core2foraws_common_error(esp_err_t error)
{
    return error == ESP_OK ? ESP_OK : ESP_FAIL;
}

int main(void)
{
#ifdef CONFIG_SOFTWARE_BSP_SUPPORT
    const int expected[] = { BUS, POWER, MOTION, RTC, CRYPTO, DISPLAY, BUTTON, RGB, WIFI };
    assert(core2foraws_init() == ESP_OK);
    assert(call_count == MODULE_COUNT);
    assert(memcmp(calls, expected, sizeof(expected)) == 0);

    for (int module = BUS; module < MODULE_COUNT; ++module)
    {
        call_count = 0;
        fail_module = module;
        esp_err_t result = core2foraws_init();
        if (module == BUS || module == POWER)
        {
            assert(result == ESP_ERR_TIMEOUT);
            assert(call_count == module + 1);
        }
        else
        {
            assert(result == ESP_FAIL);
            assert(call_count == MODULE_COUNT);
        }
    }
#else
    assert(core2foraws_init() == ESP_OK);
    assert(call_count == 0);
#endif
    puts("BSP initialization fault-injection tests passed");
    return 0;
}