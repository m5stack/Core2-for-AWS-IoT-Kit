# SPDX-FileCopyrightText: 2022 Rashed Talukder
# SPDX-FileCopyrightText: 2022 M5Stack
# SPDX-License-Identifier: MIT

#
# CMakeLists.txt file for Core2 for AWS IoT Kit Board Support Package
#
cmake_minimum_required( VERSION 3.16.0 )

set( CORE2FORAWS_BSP_VERSION "3.0.0" )

set( CORE2FORAWS_SRCS                   "${COMPONENT_DIR}/core2foraws.c" )
set( CORE2FORAWS_INCLUDE_DIRS           "${COMPONENT_DIR}/include" )

set( CORE2FORAWS_REQUIRES               "esp_adc"
                                        "mbedtls"
                                        "freertos"
                                        "esp_driver_gpio"
                                        "esp_driver_i2c"
                                        "esp_driver_i2s"
                                        "esp_driver_rmt"
                                        "esp_driver_spi"
                                        "esp_driver_uart"
                                        "esp_driver_dac"
                                        "esp_timer"
                                        "esp-cryptoauthlib"
                                        "fatfs"
                                        )
set( CORE2FORAWS_PRIV_INCLUDEDIRS       )
set( CORE2FORAWS_PRIV_REQUIRES          "nvs_flash"
                                        "esp_wifi"
                                        "esp_event"
                                        )
set( CORE2FORAWS_EXCLUDE_SRCS           )
set( CORE2FORAWS_LIB                    "${COMPONENT_DIR}/lib" )


if( CONFIG_SOFTWARE_BSP_SUPPORT )
    set( CORE2FORAWS_BSP_SRCS                   "${CORE2FORAWS_LIB}/common/core2foraws_common.c"
                                                "${CORE2FORAWS_LIB}/common/core2foraws_i2c.c"
                                                "${CORE2FORAWS_LIB}/board/core2foraws_board.c"
                                                "${CORE2FORAWS_LIB}/power/core2foraws_power.c"
                                                "${CORE2FORAWS_LIB}/power/axp192.c"
                                                )
    set( CORE2FORAWS_BSP_INCLUDE_DIRS           "${CORE2FORAWS_LIB}/common/include"
                                                "${CORE2FORAWS_LIB}/board/include"
                                                "${CORE2FORAWS_LIB}/power/include"
                                                )
    list( APPEND CORE2FORAWS_SRCS               ${CORE2FORAWS_BSP_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS       ${CORE2FORAWS_BSP_INCLUDE_DIRS} )
    list( APPEND CORE2FORAWS_PRIV_INCLUDEDIRS   "${CORE2FORAWS_LIB}/board" )
endif()


if( CONFIG_SOFTWARE_AUDIO_SUPPORT )
    set( CORE2FORAWS_AUDIO_SRCS             "${CORE2FORAWS_LIB}/audio/core2foraws_audio.c" )
    set( CORE2FORAWS_AUDIO_INCLUDE_DIRS     "${CORE2FORAWS_LIB}/audio/include/" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_AUDIO_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_AUDIO_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_BUTTON_SUPPORT )
    set( CORE2FORAWS_BUTTON_SRCS            "${CORE2FORAWS_LIB}/button/core2foraws_button.c" )
    set( CORE2FORAWS_BUTTON_INCLUDE_DIRS    "${CORE2FORAWS_LIB}/button/include/" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_BUTTON_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_BUTTON_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_CRYPTO_SUPPORT )
    set( CORE2FORAWS_CRYPTO_SRCS            "${CORE2FORAWS_LIB}/crypto/core2foraws_crypto.c"
                                            "${CORE2FORAWS_LIB}/crypto/core2foraws_crypto_hal.c"
                                            )
    set( CORE2FORAWS_CRYPTO_INCLUDE_DIRS    "${CORE2FORAWS_LIB}/crypto/include" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_CRYPTO_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_CRYPTO_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_DISPLAY_SUPPORT )
    set( CORE2FORAWS_DISPLAY_SRCS           "${CORE2FORAWS_LIB}/display/core2foraws_display.c" )
    set( CORE2FORAWS_DISPLAY_INCLUDE_DIRS   "${CORE2FORAWS_LIB}/display/include" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_DISPLAY_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_DISPLAY_INCLUDE_DIRS} )
    list( APPEND CORE2FORAWS_REQUIRES       "esp_lcd" )
endif()


if( CONFIG_SOFTWARE_EXPANSION_PORTS_SUPPORT )
    set( CORE2FORAWS_EXPPORTS_SRCS          "${CORE2FORAWS_LIB}/expports/core2foraws_expports.c" )
    set( CORE2FORAWS_EXPPORTS_INCLUDE_DIRS  "${CORE2FORAWS_LIB}/expports/include/")

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_EXPPORTS_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_EXPPORTS_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_MOTION_SUPPORT )
    set( CORE2FORAWS_MOTION_SRCS                "${CORE2FORAWS_LIB}/motion/core2foraws_motion.c" 
                                                "${CORE2FORAWS_LIB}/motion/mpu6886.c"
                                                "${CORE2FORAWS_LIB}/motion/bmi270.c"
                                                "${CORE2FORAWS_LIB}/motion/bmi270_config.c"
                                                )
    set( CORE2FORAWS_MOTION_INCLUDE_DIRS        "${CORE2FORAWS_LIB}/motion/include"
                                                )
    list( APPEND CORE2FORAWS_SRCS               ${CORE2FORAWS_MOTION_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS       ${CORE2FORAWS_MOTION_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_RGB_LED_SUPPORT )
    file( GLOB CORE2FORAWS_RGB_LED_SRCS     "${CORE2FORAWS_LIB}/rgb_led/*.c" )
    set( CORE2FORAWS_RGB_LED_INCLUDE_DIRS   "${CORE2FORAWS_LIB}/rgb_led/include" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_RGB_LED_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_RGB_LED_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_RTC_SUPPORT )
    set( CORE2FORAWS_RTC_SRCS               "${CORE2FORAWS_LIB}/rtc/core2foraws_rtc.c" )
    set( CORE2FORAWS_RTC_INCLUDE_DIRS       "${CORE2FORAWS_LIB}/rtc/include" )
    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_RTC_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_RTC_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_SDCARD_SUPPORT )
    set( CORE2FORAWS_SD_SRCS               "${CORE2FORAWS_LIB}/sd/core2foraws_sd.c" )
    set( CORE2FORAWS_SD_INCLUDE_DIRS       "${CORE2FORAWS_LIB}/sd/include" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_SD_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_SD_INCLUDE_DIRS} )
endif()

if( CONFIG_SOFTWARE_WIFI_SUPPORT )
    set( CORE2FORAWS_WIFI_SRCS              "${CORE2FORAWS_LIB}/wifi/core2foraws_wifi.c" )
    set( CORE2FORAWS_WIFI_INCLUDE_DIRS      "${CORE2FORAWS_LIB}/wifi/include" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_WIFI_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_WIFI_INCLUDE_DIRS} )

    if( CONFIG_CORE2FORAWS_WIFI_PROVISIONING )
        list( APPEND CORE2FORAWS_SRCS           "${CORE2FORAWS_LIB}/wifi/core2foraws_wifi_prov.c" )
        list( APPEND CORE2FORAWS_PRIV_REQUIRES  "network_provisioning" "protocomm" )
        if( CONFIG_CORE2FORAWS_WIFI_PROV_CONSOLE_QR )
            list( APPEND CORE2FORAWS_PRIV_REQUIRES  "qrcode" )
        endif()
        if( CONFIG_CORE2FORAWS_WIFI_RELEASE_BLE_WHEN_PROVISIONED )
            list( APPEND CORE2FORAWS_PRIV_REQUIRES  "bt" )
        endif()
    endif()
endif()

idf_component_register( SRCS                ${CORE2FORAWS_SRCS}
                        INCLUDE_DIRS        ${CORE2FORAWS_INCLUDE_DIRS}
                        PRIV_INCLUDE_DIRS   ${CORE2FORAWS_PRIV_INCLUDEDIRS}
                        PRIV_REQUIRES       ${CORE2FORAWS_PRIV_REQUIRES}
                        EXCLUDE_SRCS        ${CORE2FORAWS_EXCLUDE_SRCS}
                        REQUIRES            ${CORE2FORAWS_REQUIRES}
                        )

# Old LVGL API names fail to build instead of resolving through aliases
target_compile_definitions( ${COMPONENT_LIB} PRIVATE LV_DISABLE_API_MAPPING )
target_compile_definitions( ${COMPONENT_LIB} PUBLIC CORE2FORAWS_BSP_VERSION="${CORE2FORAWS_BSP_VERSION}" )

# Override esp-cryptoauthlib's default I2C HAL so the ATECC608 uses the BSP's
# shared internal I2C bus instead of creating its own.
if( CONFIG_SOFTWARE_CRYPTO_SUPPORT )
    target_link_options( ${COMPONENT_LIB} PUBLIC
        -Wl,-u,__wrap_hal_i2c_init
        -Wl,--wrap=hal_i2c_init
        -Wl,--wrap=hal_i2c_post_init
        -Wl,--wrap=hal_i2c_send
        -Wl,--wrap=hal_i2c_receive
        -Wl,--wrap=hal_i2c_release
        -Wl,--wrap=hal_i2c_change_baud
        -Wl,--wrap=hal_i2c_control
    )
endif()






