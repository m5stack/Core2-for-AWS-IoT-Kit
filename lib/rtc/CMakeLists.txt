idf_component_register(
    SRCS "core2foraws_rtc.c"
    INCLUDE_DIRS "include/"
    PRIV_REQUIRES bm8563 i2c_manager core2foraws_common
)
