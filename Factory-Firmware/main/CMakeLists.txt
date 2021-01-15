set(SOURCES main.c)
idf_component_register(SRCS main.c ledBarAnimation.c fft.c
                    INCLUDE_DIRS . images sounds
                    REQUIRES core2forAWS esp-cryptoauthlib)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)
add_compile_definitions(FIRMWARE_VERSION="1.0.1")