set(SOURCES main.c)
idf_component_register(SRCS main.c home.c led_bar.c mpu.c mic.c sound.c clock.c power.c touch.c wifi.c crypto.c cta.c
                    INCLUDE_DIRS . images sounds includes
                    REQUIRES core2forAWS esp-cryptoauthlib fft nvs_flash)
