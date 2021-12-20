idf_component_register(
    SRC_DIRS "."
    INCLUDE_DIRS "include/"
    REQUIRES esp_adc_cal
    PRIV_REQUIRES i2c_manager core2foraws_common
)
