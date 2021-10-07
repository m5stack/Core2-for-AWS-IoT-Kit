set(requires_list audio_hal esp_adc_cal core2forAWS)

idf_component_register(SRCS "${BUTTON_DRIVER_PATH}/button_driver.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "${BUTTON_DRIVER_PATH}"
                    REQUIRES ${requires_list})
