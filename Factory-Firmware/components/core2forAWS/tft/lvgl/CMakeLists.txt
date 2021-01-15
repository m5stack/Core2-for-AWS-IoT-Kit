if(ESP_PLATFORM)

file(GLOB_RECURSE SOURCES lvgl/src/*.c)
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS . lvgl)

target_compile_definitions(${COMPONENT_LIB} INTERFACE LV_CONF_INCLUDE_SIMPLE=1)

if (CONFIG_LV_ATTRIBUTE_FAST_MEM_USE_IRAM)
    target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_ATTRIBUTE_FAST_MEM=IRAM_ATTR")
endif()

endif()
