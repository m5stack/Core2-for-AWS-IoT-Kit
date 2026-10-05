/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../lib/audio/core2foraws_audio.c"

struct test_channel { bool allocated; bool enabled; };
static struct test_channel channel_storage;
static bool short_write;
static bool lock_timeout;
static esp_err_t transfer_error;
static esp_err_t disable_error;
static esp_err_t delete_error;
static esp_err_t mode_error;
static esp_err_t enable_error;
static esp_err_t power_error;
static unsigned int disable_calls;
static unsigned int pin_resets;
static size_t last_write_size;

SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *storage) { return storage; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t timeout)
{
    assert(timeout == 100);
    if (lock_timeout) return 0;
    assert(mutex->depth == 0);
    mutex->depth++;
    return pdTRUE;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex)
{
    assert(mutex->depth == 1);
    mutex->depth--;
    return pdTRUE;
}
void taskYIELD(void) { assert(!"First-use wait must block, not spin"); }
void vTaskDelay(TickType_t ticks)
{
    /* 1 tick for mutex first-use; otherwise the 10 ms SPM1423 wake must
       survive vTaskDelay returning up to one 10 ms tick early. */
    assert(ticks == 1 || (ticks - 1) * 10 >= SPM1423_WAKE_MS);
    if (atomic_load(&_audio_mutex_state) == AUDIO_MUTEX_INITIALIZING)
    {
        _audio_mutex = xSemaphoreCreateMutexStatic(&_audio_mutex_buf);
        atomic_store(&_audio_mutex_state, AUDIO_MUTEX_READY);
    }
}
void esp_rom_delay_us(unsigned int microseconds) { assert(microseconds > 100); }
esp_err_t gpio_reset_pin(int pin) { (void)pin; pin_resets++; return ESP_OK; }
esp_err_t core2foraws_common_error(esp_err_t error) { return error == ESP_OK ? ESP_OK : ESP_FAIL; }
esp_err_t core2foraws_power_speaker_enable(bool enabled)
{
    if (enabled) assert(channel_storage.enabled);
    return power_error;
}
esp_err_t i2s_new_channel(const i2s_chan_config_t *config, i2s_chan_handle_t *tx, i2s_chan_handle_t *rx)
{
    (void)config;
    assert(!channel_storage.allocated);
    channel_storage.allocated = true;
    *(tx != NULL ? tx : rx) = &channel_storage;
    return ESP_OK;
}
esp_err_t i2s_del_channel(i2s_chan_handle_t channel)
{
    assert(channel->allocated && !channel->enabled);
    if (delete_error != ESP_OK) return delete_error;
    channel->allocated = false;
    return ESP_OK;
}
esp_err_t i2s_channel_enable(i2s_chan_handle_t channel)
{
    if (enable_error != ESP_OK) return enable_error;
    channel->enabled = true;
    return ESP_OK;
}
esp_err_t i2s_channel_disable(i2s_chan_handle_t channel)
{
    assert(channel->enabled);
    disable_calls++;
    if (disable_error != ESP_OK) return disable_error;
    channel->enabled = false;
    return ESP_OK;
}
esp_err_t i2s_channel_init_std_mode(i2s_chan_handle_t channel, const i2s_std_config_t *config)
{
    assert(channel->allocated && config->clk_cfg.sample_rate == AUDIO_SAMPLING_FREQ);
    return mode_error;
}
esp_err_t i2s_channel_init_pdm_rx_mode(i2s_chan_handle_t channel, const i2s_pdm_rx_config_t *config)
{
    assert(channel->allocated && config->clk_cfg.sample_rate == AUDIO_SAMPLING_FREQ);
    return mode_error;
}
esp_err_t i2s_channel_write(i2s_chan_handle_t channel, const void *data, size_t size, size_t *written, uint32_t timeout_ms)
{
    assert(channel->enabled && data != NULL && timeout_ms == 1000);
    assert(_audio_mutex->depth == 1);
    last_write_size = size;
    *written = short_write ? size / 2 : size;
    return transfer_error;
}
esp_err_t i2s_channel_read(i2s_chan_handle_t channel, void *data, size_t size, size_t *received, uint32_t timeout_ms)
{
    assert(channel->enabled && timeout_ms == 1000 && _audio_mutex->depth == 1);
    *received = transfer_error == ESP_OK ? size : size / 2;
    memset(data, 0, *received);
    return transfer_error;
}

int main(void)
{
    uint8_t data[512] = {0};
    size_t received = 123;
    assert(core2foraws_audio_speaker_write(NULL, 1) == ESP_ERR_INVALID_ARG);
    assert(core2foraws_audio_mic_read(NULL, 1, &received) == ESP_ERR_INVALID_ARG && received == 0);
    atomic_store(&_audio_mutex_state, AUDIO_MUTEX_INITIALIZING);
    assert(core2foraws_audio_speaker_write(data, sizeof(data)) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_audio_speaker_enable(true) == ESP_OK);
    assert(core2foraws_audio_speaker_enable(true) == ESP_OK);
    assert(core2foraws_audio_mic_enable(true) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_audio_speaker_write(data, sizeof(data)) == ESP_OK);
    assert(core2foraws_audio_speaker_drain() == ESP_OK);
    assert(last_write_size > AUDIO_DMA_DESCRIPTORS * AUDIO_DMA_FRAMES * 4);
    short_write = true;
    assert(core2foraws_audio_speaker_drain() == ESP_FAIL);
    assert(core2foraws_audio_speaker_write(data, sizeof(data)) == ESP_FAIL);
    transfer_error = ESP_ERR_TIMEOUT;
    assert(core2foraws_audio_speaker_write(data, sizeof(data)) == ESP_ERR_TIMEOUT);
    assert(core2foraws_audio_speaker_enable(false) == ESP_OK);
    assert(core2foraws_audio_speaker_enable(false) == ESP_OK);
    assert(core2foraws_audio_mic_enable(true) == ESP_OK);
    assert(core2foraws_audio_speaker_enable(true) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_audio_mic_read((int8_t *)data, sizeof(data), &received) == ESP_ERR_TIMEOUT);
    assert(received == sizeof(data) / 2);
    transfer_error = ESP_OK;
    assert(core2foraws_audio_mic_read((int8_t *)data, sizeof(data), &received) == ESP_OK);
    assert(received == sizeof(data));
    lock_timeout = true;
    assert(core2foraws_audio_mic_read((int8_t *)data, sizeof(data), &received) == ESP_ERR_TIMEOUT && received == 0);
    assert(core2foraws_audio_mic_enable(false) == ESP_ERR_TIMEOUT && channel_storage.allocated);
    lock_timeout = false;
    assert(core2foraws_audio_mic_enable(false) == ESP_OK && !channel_storage.allocated);

    for (unsigned int microphone = 0; microphone < 2; ++microphone)
    {
        esp_err_t (*set_enabled)(bool) = microphone ? core2foraws_audio_mic_enable : core2foraws_audio_speaker_enable;
        esp_err_t (*other_enabled)(bool) = microphone ? core2foraws_audio_speaker_enable : core2foraws_audio_mic_enable;
        assert(set_enabled(true) == ESP_OK);
        unsigned int reset_count = pin_resets;
        disable_error = ESP_ERR_TIMEOUT;
        assert(set_enabled(false) == ESP_ERR_TIMEOUT && channel_storage.enabled);
        assert(set_enabled(true) == ESP_ERR_INVALID_STATE);
        assert(other_enabled(true) == ESP_ERR_INVALID_STATE);
        assert(pin_resets == reset_count);
        disable_error = ESP_OK;
        delete_error = ESP_ERR_INVALID_STATE;
        assert(set_enabled(false) == ESP_ERR_INVALID_STATE && channel_storage.allocated && !channel_storage.enabled);
        unsigned int stopped_count = disable_calls;
        assert(set_enabled(true) == ESP_ERR_INVALID_STATE);
        assert(other_enabled(true) == ESP_ERR_INVALID_STATE);
        assert(core2foraws_audio_speaker_write(data, sizeof(data)) == ESP_ERR_INVALID_STATE);
        assert(core2foraws_audio_mic_read((int8_t *)data, sizeof(data), &received) == ESP_ERR_INVALID_STATE);
        assert(pin_resets == reset_count);
        delete_error = ESP_OK;
        assert(set_enabled(false) == ESP_OK && !channel_storage.allocated);
        assert(disable_calls == stopped_count);

        mode_error = ESP_FAIL;
        delete_error = ESP_ERR_TIMEOUT;
        assert(set_enabled(true) == ESP_FAIL && channel_storage.allocated);
        assert(other_enabled(true) == ESP_ERR_INVALID_STATE);
        mode_error = ESP_OK;
        delete_error = ESP_OK;
        assert(set_enabled(false) == ESP_OK && !channel_storage.allocated);
        enable_error = ESP_FAIL;
        delete_error = ESP_ERR_TIMEOUT;
        assert(set_enabled(true) == ESP_FAIL && channel_storage.allocated);
        assert(other_enabled(true) == ESP_ERR_INVALID_STATE);
        enable_error = ESP_OK;
        delete_error = ESP_OK;
        assert(set_enabled(false) == ESP_OK && !channel_storage.allocated);
    }
    assert(core2foraws_audio_speaker_enable(true) == ESP_OK);
    power_error = ESP_ERR_TIMEOUT;
    unsigned int stopped_count = disable_calls;
    assert(core2foraws_audio_speaker_enable(false) == ESP_ERR_TIMEOUT && channel_storage.enabled);
    assert(disable_calls == stopped_count);
    assert(core2foraws_audio_mic_enable(true) == ESP_ERR_INVALID_STATE);
    power_error = ESP_OK;
    assert(core2foraws_audio_speaker_enable(false) == ESP_OK);
    power_error = ESP_ERR_TIMEOUT;
    assert(core2foraws_audio_speaker_enable(true) == ESP_ERR_TIMEOUT && channel_storage.allocated);
    assert(core2foraws_audio_mic_enable(true) == ESP_ERR_INVALID_STATE);
    power_error = ESP_OK;
    assert(core2foraws_audio_speaker_enable(false) == ESP_OK && !channel_storage.allocated);
    assert(_audio_mutex->depth == 0);
    puts("Audio timing, ownership, partial-init and teardown failure/retry tests passed");
    return 0;
}