/*
 * Link-time shims for the prebuilt libraries in components/ (libaia, libavs, libgva, libdialogflow,
 * libesp-ssdp), which were built against ESP-IDF 4.2 and reference symbols that no longer exist in 4.3+.
 */
#include "esp_idf_version.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// FreeRTOS 10.4 turned xTaskNotify() into a macro over xTaskGenericNotify().
#undef xTaskNotify
BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction)
{
    return xTaskGenericNotify(xTaskToNotify, tskDEFAULT_INDEX_TO_NOTIFY, ulValue, eAction, NULL);
}

// Newer newlib dropped __locale_ctype_ptr(); the C-locale table is _ctype_.
extern const char _ctype_[];
const char *__locale_ctype_ptr(void)
{
    return _ctype_;
}

#endif
