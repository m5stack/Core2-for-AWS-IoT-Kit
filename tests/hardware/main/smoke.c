/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <nvs.h>
#include "core2foraws.h"
#include "axp192.h"

_Static_assert(POWER_RAIL_DISPLAY_BACKLIGHT != POWER_RAIL_ESP32,
               "Power test must not write the MCU rail");
_Static_assert(POWER_RAIL_VIBRATOR != POWER_RAIL_ESP32,
               "Power test must not write the MCU rail");

static const char *TAG = "BSP_TEST";
static unsigned int checks;
static TaskHandle_t test_owner;
static atomic_uint wifi_failures;
static atomic_bool redraw_stop;
static atomic_uint redraw_count;
static atomic_uint motion_failures;

static void motion_range_worker(void *argument)
{
    (void)argument;
    for (unsigned int iteration = 0; iteration < 100; ++iteration)
    {
        if (core2foraws_motion_accel_range_set((motion_accel_range_t)(iteration % 4)) != ESP_OK)
            atomic_fetch_add(&motion_failures, 1);
        if (core2foraws_motion_gyro_range_set((motion_gyro_range_t)(iteration % 4)) != ESP_OK)
            atomic_fetch_add(&motion_failures, 1);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    if (core2foraws_motion_accel_range_set(MOTION_ACCEL_RANGE_8G) != ESP_OK)
        atomic_fetch_add(&motion_failures, 1);
    if (core2foraws_motion_gyro_range_set(MOTION_GYRO_RANGE_2000DPS) != ESP_OK)
        atomic_fetch_add(&motion_failures, 1);
    xTaskNotifyGive(test_owner);
    vTaskDelete(NULL);
}

static void wifi_lifecycle_worker(void *argument)
{
    (void)argument;
    for (unsigned int iteration = 0; iteration < 10; ++iteration)
    {
        if (core2foraws_wifi_init() != ESP_OK) atomic_fetch_add(&wifi_failures, 1);
        vTaskDelay(1);
        if (core2foraws_wifi_deinit() != ESP_OK) atomic_fetch_add(&wifi_failures, 1);
    }
    xTaskNotifyGive(test_owner);
    vTaskDelete(NULL);
}

static void wifi_provisioning_reader(void *argument)
{
    (void)argument;
    char payload[CORE2FORAWS_WIFI_PROV_PAYLOAD_LEN];
    for (unsigned int iteration = 0; iteration < 100; ++iteration)
    {
        esp_err_t result = core2foraws_wifi_provisioning_payload_get(payload, sizeof(payload));
        if (result != ESP_OK && result != ESP_ERR_INVALID_STATE && result != ESP_ERR_TIMEOUT)
            atomic_fetch_add(&wifi_failures, 1);
        if (result == ESP_OK && strstr(payload, "\"transport\":\"ble\"") == NULL)
            atomic_fetch_add(&wifi_failures, 1);
        vTaskDelay(1);
    }
    xTaskNotifyGive(test_owner);
    vTaskDelete(NULL);
}

#define EXPECT_RESULT(expression, expected) do { \
    esp_err_t actual = (expression); \
    ++checks; \
    if (actual != (expected)) { \
        ESP_LOGE(TAG, "FAIL line %d: %s returned 0x%x, expected 0x%x", \
                 __LINE__, #expression, actual, (expected)); \
        return ESP_FAIL; \
    } \
} while (0)

static void draw_status(unsigned int cycle)
{
    lvgl_port_lock(0);
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x176b47), 0);
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text_fmt(label, "BSP test %u", cycle);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_center(label);
    lv_obj_invalidate(screen);
    lvgl_port_unlock();
}

static void sd_redraw_worker(void *argument)
{
    (void)argument;
    while (!atomic_load(&redraw_stop))
    {
        draw_status(atomic_fetch_add(&redraw_count, 1));
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    xTaskNotifyGive(test_owner);
    vTaskDelete(NULL);
}

static esp_err_t run_sd_checks(void)
{
    const size_t payload_size = 16384;
    const char *file_name = "/bsp-review-test.txt";
    char *payload = heap_caps_malloc(payload_size + 1, MALLOC_CAP_SPIRAM);
    char *readback = heap_caps_malloc(payload_size + 1, MALLOC_CAP_SPIRAM);
    if (payload == NULL || readback == NULL)
    {
        free(payload);
        free(readback);
        return ESP_ERR_NO_MEM;
    }
    for (size_t offset = 0; offset < payload_size; ++offset)
        payload[offset] = '!' + offset % 90;
    payload[payload_size] = '\0';

    nvs_handle_t test_nvs = 0;
    esp_err_t result = core2foraws_sd_mount();
    if (result != ESP_OK) goto cleanup;
    result = nvs_open("bsp_review", NVS_READWRITE, &test_nvs);
    if (result != ESP_OK) goto unmount;
    atomic_store(&redraw_stop, false);
    atomic_store(&redraw_count, 0);
    if (xTaskCreatePinnedToCore(sd_redraw_worker, "sdRedraw", 4096,
                               NULL, 2, NULL, 1) != pdPASS)
    {
        result = ESP_ERR_NO_MEM;
        goto unmount;
    }
    int64_t started = esp_timer_get_time();
    for (unsigned int iteration = 0; iteration < 20; ++iteration)
    {
        size_t written = 0;
        result = core2foraws_sd_write(file_name, payload, &written);
        if (result != ESP_OK || written != payload_size)
        {
            result = ESP_FAIL;
            break;
        }
        result = core2foraws_sd_read(file_name, readback, payload_size + 1);
        if (result != ESP_OK || memcmp(payload, readback, payload_size + 1) != 0)
        {
            result = ESP_FAIL;
            break;
        }
        checks += 2;
        result = nvs_set_u32(test_nvs, "iteration", iteration);
        if (result != ESP_OK) break;
        result = nvs_commit(test_nvs);
        if (result != ESP_OK) break;
        nvs_close(test_nvs);
        test_nvs = 0;
        result = nvs_open("bsp_review", NVS_READWRITE, &test_nvs);
        if (result != ESP_OK) break;
        uint32_t persisted_iteration = UINT32_MAX;
        result = nvs_get_u32(test_nvs, "iteration", &persisted_iteration);
        if (result != ESP_OK || persisted_iteration != iteration)
        {
            result = ESP_FAIL;
            break;
        }
        ++checks;
    }
    atomic_store(&redraw_stop, true);
    if (ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(5000)) == 0)
    {
        result = ESP_ERR_TIMEOUT;
        goto unmount;
    }
    if (result != ESP_OK) goto unmount;
    ESP_LOGI(TAG, "20 SD 16KiB write/read + NVS commit cycles took %lld us; redraw requests=%u",
             (long long)(esp_timer_get_time() - started), atomic_load(&redraw_count));

    result = core2foraws_sd_unmount();
    if (result != ESP_OK) goto cleanup;
    result = core2foraws_sd_mount();
    if (result != ESP_OK) goto cleanup;
    result = core2foraws_sd_read(file_name, readback, payload_size + 1);
    if (result != ESP_OK || memcmp(payload, readback, payload_size + 1) != 0)
    {
        result = ESP_FAIL;
        goto unmount;
    }
    ++checks;
    if (xSemaphoreTake(core2foraws_common_spi_semaphore, pdMS_TO_TICKS(1000)) != pdTRUE)
    {
        result = ESP_ERR_TIMEOUT;
        goto unmount;
    }
    esp_err_t timeout_result = core2foraws_sd_read(file_name, readback, payload_size + 1);
    xSemaphoreGive(core2foraws_common_spi_semaphore);
    if (timeout_result != ESP_ERR_TIMEOUT || readback[0] != '\0')
    {
        result = ESP_FAIL;
        goto unmount;
    }
    ++checks;
    if (xSemaphoreTake(core2foraws_common_spi_semaphore, pdMS_TO_TICKS(1000)) != pdTRUE)
    {
        result = ESP_ERR_TIMEOUT;
        goto unmount;
    }
    int remove_result = remove("/sd_card/bsp-review-test.txt");
    xSemaphoreGive(core2foraws_common_spi_semaphore);
    if (remove_result != 0) result = ESP_FAIL;
    if (result == ESP_OK)
        ESP_LOGI(TAG, "SD remount durability and SPI timeout passed; test file removed");
unmount:
    {
        esp_err_t unmount_result = core2foraws_sd_unmount();
        if (result == ESP_OK) result = unmount_result;
    }
cleanup:
    if (test_nvs != 0)
    {
        esp_err_t erase_result = nvs_erase_all(test_nvs);
        if (erase_result == ESP_OK) erase_result = nvs_commit(test_nvs);
        nvs_close(test_nvs);
        if (result == ESP_OK) result = erase_result;
    }
    free(payload);
    free(readback);
    return result;
}

static esp_err_t run_power_checks(void)
{
    uint16_t mcu_mv, backlight_mv;
    bool mcu_enabled, backlight_enabled, vibration_enabled;
    uint8_t gpio_signals;
    EXPECT_RESULT(core2foraws_power_rail_mv_get(POWER_RAIL_ESP32, &mcu_mv), ESP_OK);
    EXPECT_RESULT(core2foraws_power_rail_state_get(POWER_RAIL_ESP32, &mcu_enabled), ESP_OK);
    EXPECT_RESULT(core2foraws_power_rail_mv_get(POWER_RAIL_DISPLAY_BACKLIGHT, &backlight_mv), ESP_OK);
    EXPECT_RESULT(core2foraws_power_rail_state_get(POWER_RAIL_DISPLAY_BACKLIGHT, &backlight_enabled), ESP_OK);
    EXPECT_RESULT(core2foraws_power_rail_state_get(POWER_RAIL_VIBRATOR, &vibration_enabled), ESP_OK);
    EXPECT_RESULT(core2foraws_power_axp_reg_get(AXP192_GPIO20_SIGNAL_STATUS, &gpio_signals), ESP_OK);
    if (!mcu_enabled) return ESP_FAIL;
    esp_err_t result = ESP_OK;

#define POWER_STEP(expression) do { \
    result = (expression); \
    ++checks; \
    if (result != ESP_OK) goto restore; \
} while (0)

    const uint8_t levels[] = { 0, 25, 100 };
    const uint16_t expected_mv[] = { 0, 2475, 3300 };
    for (size_t level = 0; level < sizeof(levels); ++level)
    {
        POWER_STEP(core2foraws_power_backlight_set(levels[level]));
        bool enabled;
        POWER_STEP(core2foraws_power_rail_state_get(POWER_RAIL_DISPLAY_BACKLIGHT, &enabled));
        if (enabled != (levels[level] != 0)) { result = ESP_FAIL; goto restore; }
        if (enabled)
        {
            uint16_t actual_mv;
            POWER_STEP(core2foraws_power_rail_mv_get(POWER_RAIL_DISPLAY_BACKLIGHT, &actual_mv));
            if (actual_mv != expected_mv[level]) { result = ESP_FAIL; goto restore; }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    for (unsigned int enabled = 0; enabled < 2; ++enabled)
    {
        uint8_t actual_signals;
        POWER_STEP(core2foraws_power_led_enable(enabled != 0));
        POWER_STEP(core2foraws_power_axp_reg_get(AXP192_GPIO20_SIGNAL_STATUS, &actual_signals));
        if (((actual_signals & 0x02) == 0) != (enabled != 0)) { result = ESP_FAIL; goto restore; }
    }
    POWER_STEP(core2foraws_power_vibration_enable(true));
    vTaskDelay(pdMS_TO_TICKS(150));
    bool actual_vibration;
    POWER_STEP(core2foraws_power_rail_state_get(POWER_RAIL_VIBRATOR, &actual_vibration));
    if (!actual_vibration) { result = ESP_FAIL; goto restore; }
restore:
    {
        esp_err_t restore_result = core2foraws_power_vibration_enable(vibration_enabled);
        if (result == ESP_OK) result = restore_result;
        restore_result = core2foraws_power_led_enable((gpio_signals & 0x02) == 0);
        if (result == ESP_OK) result = restore_result;
        restore_result = core2foraws_power_rail_mv_set(POWER_RAIL_DISPLAY_BACKLIGHT, backlight_mv);
        if (result == ESP_OK) result = restore_result;
        restore_result = core2foraws_power_rail_state_set(POWER_RAIL_DISPLAY_BACKLIGHT, backlight_enabled);
        if (result == ESP_OK) result = restore_result;
    }
#undef POWER_STEP
    if (result != ESP_OK) return result;
    uint16_t final_mcu_mv;
    bool final_mcu_enabled;
    EXPECT_RESULT(core2foraws_power_rail_mv_get(POWER_RAIL_ESP32, &final_mcu_mv), ESP_OK);
    EXPECT_RESULT(core2foraws_power_rail_state_get(POWER_RAIL_ESP32, &final_mcu_enabled), ESP_OK);
    if (mcu_mv != final_mcu_mv || !final_mcu_enabled) return ESP_FAIL;
    ESP_LOGI(TAG, "Power backlight/LED/vibration tests passed; MCU unchanged at %umV; peripheral state restored", mcu_mv);
    return ESP_OK;
}

static esp_err_t run_checks(void)
{
    test_owner = xTaskGetCurrentTaskHandle();
    EXPECT_RESULT(core2foraws_init(), ESP_OK);
    EXPECT_RESULT(core2foraws_init(), ESP_OK);
    ESP_LOGI(TAG, "Repeated BSP initialization passed");
    core2foraws_board_info_t board;
    EXPECT_RESULT(core2foraws_board_info_get(&board), ESP_OK);
    if (board.model == BOARD_MODEL_UNKNOWN || board.lcd == BOARD_LCD_UNKNOWN) return ESP_FAIL;
    ESP_LOGI(TAG, "Board %s: %s IMU, %s microphone, %s LCD", board.model_name,
             board.imu_name, board.microphone_name, board.lcd_name);
    draw_status(0);
    vTaskDelay(pdMS_TO_TICKS(250));

    EXPECT_RESULT(run_power_checks(), ESP_OK);
    float battery_volts, accel_x, accel_y, accel_z, temperature;
    struct tm rtc_time;
    char serial_number[CRYPTO_SERIAL_STR_SIZE];
    EXPECT_RESULT(core2foraws_power_batt_volts_get(&battery_volts), ESP_OK);
    EXPECT_RESULT(core2foraws_motion_accel_get(&accel_x, &accel_y, &accel_z), ESP_OK);
    EXPECT_RESULT(core2foraws_motion_temperature_get(&temperature), ESP_OK);
    EXPECT_RESULT(core2foraws_rtc_utc_time_get(&rtc_time), ESP_OK);
    EXPECT_RESULT(core2foraws_crypto_serial_get(serial_number), ESP_OK);
    if (!isfinite(battery_volts) || !isfinite(accel_x) ||
        !isfinite(accel_y) || !isfinite(accel_z) || !isfinite(temperature)) return ESP_FAIL;
    ESP_LOGI(TAG, "Sensors battery=%.3fV accel=(%.3f,%.3f,%.3f) temperature=%.2fC",
             battery_volts, accel_x, accel_y, accel_z, temperature);

    EXPECT_RESULT(core2foraws_i2c_init(CORE2FORAWS_I2C_EXTERNAL), ESP_OK);
    i2c_master_dev_handle_t device = NULL;
    EXPECT_RESULT(core2foraws_i2c_device_add(CORE2FORAWS_I2C_EXTERNAL, 0x42, 100000, &device), ESP_OK);
    EXPECT_RESULT(core2foraws_i2c_device_remove(device), ESP_OK);
    uint8_t byte;
    EXPECT_RESULT(core2foraws_i2c_read(CORE2FORAWS_I2C_EXTERNAL, device, 0, &byte, 1), ESP_ERR_INVALID_STATE);
    EXPECT_RESULT(core2foraws_i2c_deinit(CORE2FORAWS_I2C_EXTERNAL), ESP_OK);

    for (unsigned int cycle = 0; cycle < 10; ++cycle)
    {
        bool level;
        EXPECT_RESULT(core2foraws_expports_digital_read(PORT_A_SDA_PIN, &level), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_digital_read(PORT_A_SCL_PIN, &level), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_i2c_begin(), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_pin_reset(PORT_A_SCL_PIN), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_i2c_begin(), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_i2c_close(), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_digital_read(PORT_C_UART_RX_PIN, &level), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_digital_read(PORT_C_UART_TX_PIN, &level), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_uart_begin(115200), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_pin_reset(PORT_C_UART_TX_PIN), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_uart_begin(9600), ESP_OK);
        EXPECT_RESULT(core2foraws_expports_pin_reset(PORT_C_UART_RX_PIN), ESP_OK);
        EXPECT_RESULT(core2foraws_rgb_led_init(), ESP_OK);
        EXPECT_RESULT(core2foraws_rgb_led_brightness_set(5), ESP_OK);
        EXPECT_RESULT(core2foraws_rgb_led_single_color_set(cycle, 0x00ff00), ESP_OK);
        EXPECT_RESULT(core2foraws_rgb_led_write(), ESP_OK);
        EXPECT_RESULT(core2foraws_rgb_led_clear(), ESP_OK);
        EXPECT_RESULT(core2foraws_rgb_led_write(), ESP_OK);
        EXPECT_RESULT(core2foraws_rgb_led_deinit(), ESP_OK);
    }
    EXPECT_RESULT(core2foraws_rgb_led_init(), ESP_OK);
    EXPECT_RESULT(core2foraws_rgb_led_brightness_set(100), ESP_OK);
    ESP_LOGI(TAG, "10 RGB and paired I2C/UART teardown/reopen cycles passed");

    static int8_t samples[4096];
    for (unsigned int cycle = 0; cycle < 20; ++cycle)
    {
        size_t received = 0;
        EXPECT_RESULT(core2foraws_audio_mic_enable(true), ESP_OK);
        EXPECT_RESULT(core2foraws_audio_speaker_enable(true), ESP_ERR_INVALID_STATE);
        EXPECT_RESULT(core2foraws_audio_mic_read(samples, sizeof(samples), &received), ESP_OK);
        if (received != sizeof(samples)) return ESP_FAIL;
        EXPECT_RESULT(core2foraws_audio_mic_enable(false), ESP_OK);
        EXPECT_RESULT(core2foraws_audio_speaker_enable(true), ESP_OK);
        EXPECT_RESULT(core2foraws_audio_mic_enable(true), ESP_ERR_INVALID_STATE);
        memset(samples, 0, sizeof(samples));
        EXPECT_RESULT(core2foraws_audio_speaker_write((uint8_t *)samples, sizeof(samples)), ESP_OK);
        EXPECT_RESULT(core2foraws_audio_speaker_enable(false), ESP_OK);
    }
    ESP_LOGI(TAG, "20 microphone/speaker lifecycle cycles passed");

    if (xSemaphoreTake(core2foraws_common_spi_semaphore, pdMS_TO_TICKS(1000)) != pdTRUE) return ESP_FAIL;
    draw_status(1);
    vTaskDelay(pdMS_TO_TICKS(CORE2FORAWS_SPI_LOCK_TIMEOUT_MS + 200));
    EXPECT_RESULT(core2foraws_display_deinit(), ESP_ERR_TIMEOUT);
    if (core2foraws_display_ptr == NULL) return ESP_FAIL;
    xSemaphoreGive(core2foraws_common_spi_semaphore);
    EXPECT_RESULT(lvgl_port_resume(), ESP_OK);
    draw_status(2);
    vTaskDelay(pdMS_TO_TICKS(250));
    ESP_LOGI(TAG, "SPI contention and teardown timeout recovery passed");

    for (unsigned int cycle = 0; cycle < 3; ++cycle)
    {
        EXPECT_RESULT(core2foraws_display_deinit(), ESP_OK);
        EXPECT_RESULT(core2foraws_display_init(), ESP_OK);
        draw_status(cycle + 3);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    ESP_LOGI(TAG, "3 display teardown/reinitialization cycles passed");

    EXPECT_RESULT(run_sd_checks(), ESP_OK);
    for (unsigned int worker = 0; worker < 2; ++worker)
    {
        if (xTaskCreatePinnedToCore(wifi_lifecycle_worker, "wifiRace", 6144,
                                   NULL, 2, NULL, worker) != pdPASS) return ESP_FAIL;
    }
    for (unsigned int worker = 0; worker < 2; ++worker)
    {
        if (ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(30000)) == 0) return ESP_FAIL;
    }
    if (atomic_load(&wifi_failures) != 0) return ESP_FAIL;
    ESP_LOGI(TAG, "20 concurrent Wi-Fi init/deinit cycles passed");
    EXPECT_RESULT(core2foraws_wifi_init(), ESP_OK);
    char saved_ssid[MAX_SSID_LEN + 1];
    if (core2foraws_wifi_saved_ssid_get(saved_ssid) == ESP_OK)
        (void)core2foraws_wifi_reconnect(15000);
    else
        EXPECT_RESULT(core2foraws_wifi_provisioning_start(), ESP_OK);
    core2foraws_wifi_state_t wifi_state;
    EXPECT_RESULT(core2foraws_wifi_state_get(&wifi_state), ESP_OK);
    ESP_LOGI(TAG, "Wi-Fi connected=%d",
             wifi_state == CORE2FORAWS_WIFI_STATE_CONNECTED);
    core2foraws_common_heap_stats_t initial_heap;
    EXPECT_RESULT(core2foraws_common_heap_report(TAG, &initial_heap), ESP_OK);
    if (xTaskCreatePinnedToCore(motion_range_worker, "motionRange", 4096,
                               NULL, 2, NULL, 0) != pdPASS) return ESP_FAIL;
    int64_t started = esp_timer_get_time();
    for (unsigned int cycle = 0; cycle < 1000; ++cycle)
    {
        EXPECT_RESULT(core2foraws_motion_accel_get(&accel_x, &accel_y, &accel_z), ESP_OK);
        float roll, pitch, yaw;
        EXPECT_RESULT(core2foraws_motion_gyro_get(&roll, &pitch, &yaw), ESP_OK);
        if (!isfinite(accel_x) || !isfinite(accel_y) || !isfinite(accel_z) ||
            !isfinite(roll) || !isfinite(pitch) || !isfinite(yaw)) return ESP_FAIL;
        EXPECT_RESULT(core2foraws_power_batt_volts_get(&battery_volts), ESP_OK);
        if (cycle % 100 == 0)
        {
            EXPECT_RESULT(core2foraws_crypto_serial_get(serial_number), ESP_OK);
            EXPECT_RESULT(core2foraws_rtc_utc_time_get(&rtc_time), ESP_OK);
            draw_status(cycle);
        }
        vTaskDelay(1);
    }
    ESP_LOGI(TAG, "1000 shared-I2C iterations took %lld us", (long long)(esp_timer_get_time() - started));
    if (ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(5000)) == 0) return ESP_FAIL;
    if (atomic_load(&motion_failures) != 0) return ESP_FAIL;
    ESP_LOGI(TAG, "100 concurrent motion range updates passed; defaults restored");
    core2foraws_common_heap_stats_t final_heap;
    EXPECT_RESULT(core2foraws_common_heap_report(TAG, &final_heap), ESP_OK);
    ESP_LOGI(TAG, "Internal free change=%ld bytes", (long)final_heap.internal_free - (long)initial_heap.internal_free);
    if (xTaskCreatePinnedToCore(wifi_provisioning_reader, "wifiReadRace", 4096,
                               NULL, 2, NULL, 1) != pdPASS) return ESP_FAIL;
    vTaskDelay(2);
    EXPECT_RESULT(core2foraws_wifi_deinit(), ESP_OK);
    if (ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(5000)) == 0) return ESP_FAIL;
    if (atomic_load(&wifi_failures) != 0) return ESP_FAIL;
    ESP_LOGI(TAG, "100 provisioning reads racing Wi-Fi teardown passed");
    if (!heap_caps_check_integrity_all(true)) return ESP_FAIL;
    return ESP_OK;
}

void app_main(void)
{
    esp_err_t result = run_checks();
    ESP_LOGI(TAG, "RESULT %s checks=%u", result == ESP_OK ? "PASS" : "FAIL", checks);
}