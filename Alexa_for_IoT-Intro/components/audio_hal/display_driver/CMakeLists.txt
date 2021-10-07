set(requires_list audio_hal core2forAWS)

idf_component_register(SRCS "${DISPLAY_DRIVER_PATH}/display_driver.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "${DISPLAY_DRIVER_PATH}"
                    REQUIRES ${requires_list})
