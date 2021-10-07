set(requires_list audio_hal core2forAWS)

idf_component_register(SRCS "${LED_DRIVER_PATH}/led_driver.c" "include/led_driver_utils.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "${LED_DRIVER_PATH}"
                    REQUIRES ${requires_list})
