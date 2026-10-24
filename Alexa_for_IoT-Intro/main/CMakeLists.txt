set(COMPONENT_ADD_INCLUDEDIRS .)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS ./app_auth.c ./app_main.c ./app_mqtt.c ./app_va_cb.c ./idf44_compat.c)

register_component()

# With BSS in external RAM, IDF >= 4.3 does not generate these BT linker symbols but
# wifi_provisioning's BLE scheme still pulls in esp_bt_mem_release(); equal values make it a no-op.
if(NOT (IDF_VERSION_MAJOR EQUAL 4 AND IDF_VERSION_MINOR LESS 3) AND CONFIG_BT_ENABLED AND CONFIG_ESP_ALLOW_BSS_SEG_EXTERNAL_MEMORY)
    target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--defsym=_bt_bss_start=0" "-Wl,--defsym=_bt_bss_end=0")
endif()
