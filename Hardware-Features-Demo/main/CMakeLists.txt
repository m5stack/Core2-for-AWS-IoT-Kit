set(SOURCES main.c)
idf_component_register(SRCS main.c music.c atecc608_test.c sk6812_test.c fft.c mic_fft_test.c
                    INCLUDE_DIRS "includes"
                    REQUIRES core2forAWS esp-cryptoauthlib fatfs)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)