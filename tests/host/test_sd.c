/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdio.h>
#include <string.h>

static FILE *test_fopen(const char *path, const char *mode);
static int test_fclose(FILE *file);
#define fopen test_fopen
#define fclose test_fclose
#include "../../lib/sd/core2foraws_sd.c"
#undef fopen
#undef fclose

static StaticSemaphore_t spi_storage;
SemaphoreHandle_t core2foraws_common_spi_semaphore = &spi_storage;
static sdmmc_card_t card_storage;
static int spi_calls;
static int fail_spi_call;
static int open_files;
static bool fail_close;

static FILE *test_fopen(const char *path, const char *mode)
{
    (void)mode;
    assert(strcmp(path, "/sd_card/test.txt") == 0);
    FILE *file = tmpfile();
    assert(file != NULL);
    open_files++;
    return file;
}
static int test_fclose(FILE *file)
{
    assert(spi_storage.depth == 1 && open_files == 1);
    int result = fclose(file);
    open_files--;
    return fail_close ? EOF : result;
}
SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *storage) { return storage; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t timeout)
{
    (void)timeout;
    assert(mutex->depth == 0);
    if (mutex == &spi_storage && ++spi_calls == fail_spi_call) return 0;
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
    assert(ticks == 1 && atomic_load(&_sd_mutex_state) == 1);
    _sd_mutex = xSemaphoreCreateMutexStatic(&_sd_mutex_storage);
    atomic_store(&_sd_mutex_state, 2);
}
esp_err_t core2foraws_common_spi_bus_init(void) { return ESP_OK; }
esp_err_t esp_vfs_fat_sdspi_mount(const char *path, const sdmmc_host_t *host, const sdspi_device_config_t *slot, const esp_vfs_fat_mount_config_t *config, sdmmc_card_t **card)
{
    (void)path; (void)host; (void)slot;
    assert(!config->format_if_mount_failed);
    *card = &card_storage;
    return ESP_OK;
}
esp_err_t esp_vfs_fat_sdcard_unmount(const char *path, sdmmc_card_t *card)
{
    (void)path;
    assert(card == &card_storage && open_files == 0);
    return ESP_OK;
}

int main(void)
{
    size_t written = 0;
    char data[8] = "stale";
    atomic_store(&_sd_mutex_state, 1);
    assert(core2foraws_sd_read("/test.txt", data, sizeof(data)) == ESP_ERR_INVALID_STATE);
    assert(data[0] == '\0');
    assert(core2foraws_sd_mount() == ESP_OK);
    assert(core2foraws_sd_write("/test.txt", "hello", &written) == ESP_OK);
    assert(written == 5 && open_files == 0);
    fail_close = true;
    assert(core2foraws_sd_write("/test.txt", "hello", &written) == ESP_FAIL);
    assert(open_files == 0 && _pending_close == NULL);
    assert(core2foraws_sd_read("/test.txt", data, sizeof(data)) == ESP_FAIL);
    fail_close = false;

    fail_spi_call = spi_calls + 3;
    assert(core2foraws_sd_write("/test.txt", "hello", &written) == ESP_ERR_TIMEOUT);
    assert(open_files == 1 && _pending_close != NULL);
    fail_spi_call = spi_calls + 1;
    assert(core2foraws_sd_unmount() == ESP_ERR_TIMEOUT);
    assert(open_files == 1 && _sd_card != NULL);
    fail_spi_call = 0;
    assert(core2foraws_sd_unmount() == ESP_OK);
    assert(open_files == 0 && _pending_close == NULL && _sd_card == NULL);
    assert(_sd_mutex->depth == 0 && spi_storage.depth == 0);
    puts("SD close-error propagation and deferred-close retry tests passed");
    return 0;
}