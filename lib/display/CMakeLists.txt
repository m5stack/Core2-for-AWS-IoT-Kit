idf_component_register(
    SRCS "core2foraws_display.c"
    INCLUDE_DIRS "include/"
    REQUIRES lvgl lvgl_esp32_drivers
    PRIV_REQUIRES core2foraws_button
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_LVGL_H_INCLUDE_SIMPLE")