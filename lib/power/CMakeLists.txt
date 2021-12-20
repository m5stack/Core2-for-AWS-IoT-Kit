idf_component_register(
    SRCS "core2foraws_power.c"
    INCLUDE_DIRS "include/"
    PRIV_REQUIRES axp192 i2c_manager core2foraws_common
)