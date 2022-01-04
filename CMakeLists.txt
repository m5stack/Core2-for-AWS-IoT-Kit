#
# CMakeLists.txt file for Core2 for AWS IoT EduKit Board Support Package
#
cmake_minimum_required( VERSION 3.10.2 )

MACRO( GET_HEADER_DIRS return_list start_dir )
    FILE( GLOB_RECURSE new_list "${start_dir}/*.h" )
    SET( dir_list "" )
    FOREACH( file_path ${new_list} )
    GET_FILENAME_COMPONENT( dir_path ${file_path} PATH )
    SET( dir_list ${dir_list} ${dir_path} )
    ENDFOREACH()
    LIST( REMOVE_DUPLICATES dir_list )
    SET( ${return_list} ${dir_list} )
ENDMACRO()

set( CORE2FORAWS_SRCS                   "${COMPONENT_DIR}/core2foraws.c" )
set( CORE2FORAWS_INCLUDE_DIRS           "${COMPONENT_DIR}/include" )
set( CORE2FORAWS_REQUIRES               "esp_adc_cal"
                                        "mbedtls"
                                        "freertos"
                                        "driver"
                                        "esp-cryptoauthlib"
                                        "fatfs"
                                        )
set( CORE2FORAWS_PRIV_INCLUDEDIRS       )
set( CORE2FORAWS_PRIV_REQUIRES          "nvs_flash"
                                        "wifi_provisioning"
                                        )
set( CORE2FORAWS_EXCLUDE_SRCS           )
set( CORE2FORAWS_LIB                    "${COMPONENT_DIR}/lib" )


if( CONFIG_SOFTWARE_BSP_SUPPORT )
    set( I2C_MANAGER_SRCS                   "${CORE2FORAWS_LIB}/common/lib/i2c_manager/i2c_manager/i2c_manager.c" )
    set( I2C_MANAGER_INCLUDE_DIRS           "${CORE2FORAWS_LIB}/common/lib/i2c_manager/i2c_manager"
                                            "${CORE2FORAWS_LIB}/common/lib/i2c_manager/i2cdev" 
                                            )
    list( APPEND CORE2FORAWS_SRCS           ${I2C_MANAGER_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${I2C_MANAGER_INCLUDE_DIRS})

    set( CORE2FORAWS_BSP_SRCS                   "${CORE2FORAWS_LIB}/common/core2foraws_common.c"
                                                "${CORE2FORAWS_LIB}/power/core2foraws_power.c"
                                                "${CORE2FORAWS_LIB}/power/lib/axp192/axp192.c"
                                                "${CORE2FORAWS_LIB}/wifi/core2foraws_wifi.c"
                                                )
    set( CORE2FORAWS_BSP_INCLUDE_DIRS           "${CORE2FORAWS_LIB}/common/include" 
                                                "${CORE2FORAWS_LIB}/power/include"
                                                "${CORE2FORAWS_LIB}/power/lib/axp192"
                                                "${CORE2FORAWS_LIB}/wifi/include"
                                                )
    list( APPEND CORE2FORAWS_SRCS               ${CORE2FORAWS_BSP_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS       ${CORE2FORAWS_BSP_INCLUDE_DIRS} )
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
    set( CORE2FORAWS_CRYPTO_SRCS            "${CORE2FORAWS_LIB}/crypto/core2foraws_crypto.c" )
    set( CORE2FORAWS_CRYPTO_INCLUDE_DIRS    "${CORE2FORAWS_LIB}/crypto/include" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_CRYPTO_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_CRYPTO_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_DISPLAY_SUPPORT )
    file( GLOB_RECURSE LVGL_SRCS            "${CORE2FORAWS_LIB}/display/lib/lvgl/src/*.c" )
    GET_HEADER_DIRS( LVGL_INCLUDE_DIRS      "${CORE2FORAWS_LIB}/display/lib/lvgl/src" )
    GET_HEADER_DIRS( LVGL_ESP_INCLUDE_DIRS  "${CORE2FORAWS_LIB}/display/lib/lvgl_esp32_drivers" )

    set( CORE2FORAWS_DISPLAY_SRCS           "${CORE2FORAWS_LIB}/display/core2foraws_display.c" 
                                            ${LVGL_SRCS}
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl_esp32_drivers/lvgl_helpers.c"
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl_esp32_drivers/lvgl_i2c/i2c_manager.c"
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl_esp32_drivers/lvgl_tft/disp_driver.c"
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl_esp32_drivers/lvgl_tft/esp_lcd_backlight.c"
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl_esp32_drivers/lvgl_tft/ili9341.c"
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl_esp32_drivers/lvgl_tft/disp_spi.c"
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl_esp32_drivers/lvgl_touch/touch_driver.c"
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl_esp32_drivers/lvgl_touch/ft6x36.c"
                                            )
    set( CORE2FORAWS_DISPLAY_INCLUDE_DIRS   "${CORE2FORAWS_LIB}/display/include" 
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl"
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl/src"
                                            ${LVGL_INCLUDE_DIRS}
                                            "${CORE2FORAWS_LIB}/display/lib/lvgl_esp32_drivers"
                                            ${LVGL_ESP_INCLUDE_DIRS}
                                            )
    
    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_DISPLAY_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_DISPLAY_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_EXPPORTS_SUPPORT )
    set( CORE2FORAWS_EXPPORTS_SRCS          "${CORE2FORAWS_LIB}/expports/core2foraws_expports.c" )
    set( CORE2FORAWS_EXPPORTS_INCLUDE_DIRS  "${CORE2FORAWS_LIB}/expports/include/" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_EXPPORTS_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_EXPPORTS_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_MOTION_SUPPORT )
    set( CORE2FORAWS_MOTION_SRCS                "${CORE2FORAWS_LIB}/motion/core2foraws_motion.c" 
                                                "${CORE2FORAWS_LIB}/motion/lib/MPU6886-idf/mpu6886.c"
                                                )
    set( CORE2FORAWS_MOTION_INCLUDE_DIRS        "${CORE2FORAWS_LIB}/motion/include"
                                                "${CORE2FORAWS_LIB}/motion/lib/MPU6886-idf"
                                                )
    list( APPEND CORE2FORAWS_SRCS               ${CORE2FORAWS_MOTION_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS       ${CORE2FORAWS_MOTION_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_RGB_LED_SUPPORT )
    file( GLOB CORE2FORAWS_RGB_LED_SRCS     "${CORE2FORAWS_LIB}/rgb_led/*.c"
                                            "${CORE2FORAWS_LIB}/rgb_led/lib/esp-idf-lib/components/lib8tion/*.c"
                                            "${CORE2FORAWS_LIB}/rgb_led/lib/esp-idf-lib/components/color/*.c"
                                            "${CORE2FORAWS_LIB}/rgb_led/lib/esp-idf-lib/components/led_strip/*.c"
                                            )
    set( CORE2FORAWS_RGB_LED_INCLUDE_DIRS   "${CORE2FORAWS_LIB}/rgb_led/include"
                                            "${CORE2FORAWS_LIB}/rgb_led/lib/esp-idf-lib/components/esp_idf_lib_helpers" 
                                            "${CORE2FORAWS_LIB}/rgb_led/lib/esp-idf-lib/components/lib8tion"
                                            "${CORE2FORAWS_LIB}/rgb_led/lib/esp-idf-lib/components/lib8tion/lib8tion"
                                            "${CORE2FORAWS_LIB}/rgb_led/lib/esp-idf-lib/components/color"
                                            "${CORE2FORAWS_LIB}/rgb_led/lib/esp-idf-lib/components/led_strip"
                                            )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_RGB_LED_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_RGB_LED_INCLUDE_DIRS} )
endif()


if( CONFIG_SOFTWARE_RTC_SUPPORT )
    set( CORE2FORAWS_RTC_SRCS               "${CORE2FORAWS_LIB}/rtc/core2foraws_rtc.c"
                                            "${CORE2FORAWS_LIB}/rtc/lib/bm8563/bm8563.c"
                                            )
    set( CORE2FORAWS_RTC_INCLUDE_DIRS       "${CORE2FORAWS_LIB}/rtc/include"
                                            "${CORE2FORAWS_LIB}/rtc/lib/bm8563"
                                            )
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
    file( GLOB_RECURSE QRCODE_SRCS          "$ENV{IDF_PATH}/examples/common_components/qrcode/*.c" )
    set( QRCODE_INCLUDE_DIRS                "$ENV{IDF_PATH}/examples/common_components/qrcode"
                                            "$ENV{IDF_PATH}/examples/common_components/qrcode/include"
                                            )
    set( CORE2FORAWS_WIFI_SRCS              "${CORE2FORAWS_LIB}/wifi/core2foraws_wifi.c" )
    set( CORE2FORAWS_WIFI_INCLUDE_DIRS      "${CORE2FORAWS_LIB}/wifi/include" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_WIFI_SRCS}
                                            ${QRCODE_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_WIFI_INCLUDE_DIRS}
                                            ${QRCODE_INCLUDE_DIRS} )
endif()

if( CONFIG_SOFTWARE_DISPLAY_SUPPORT AND CONFIG_SOFTWARE_WIFI_SUPPORT )
    file( GLOB_RECURSE CORE2FORAWS_DISPLAY_WIFI_SRCS    "${CORE2FORAWS_LIB}/display/lib/lv_lib_qrcode/*.c" )
    set( CORE2FORAWS_DISPLAY_WIFI_INCLUDE_DIRS          "${CORE2FORAWS_LIB}/display/lib/lv_lib_qrcode" )

    list( APPEND CORE2FORAWS_SRCS           ${CORE2FORAWS_DISPLAY_WIFI_SRCS} )
    list( APPEND CORE2FORAWS_INCLUDE_DIRS   ${CORE2FORAWS_DISPLAY_WIFI_INCLUDE_DIRS} )
endif()

idf_component_register( SRCS                ${CORE2FORAWS_SRCS}
                        INCLUDE_DIRS        ${CORE2FORAWS_INCLUDE_DIRS}
                        PRIV_INCLUDE_DIRS   ${CORE2FORAWS_PRIV_INCLUDEDIRS}
                        PRIV_REQUIRES       ${CORE2FORAWS_PRIV_REQUIRES}
                        EXCLUDE_SRCS        ${CORE2FORAWS_EXCLUDE_SRCS}
                        REQUIRES            ${CORE2FORAWS_REQUIRES}
                        )


if( CONFIG_SOFTWARE_BSP_SUPPORT )
    target_compile_definitions( ${COMPONENT_LIB} PUBLIC     "-DCONFIG_I2C_MANAGER_0_ENABLED=y"
                                                            "-DCONFIG_I2C_MANAGER_0_SDA=21"
                                                            "-DCONFIG_I2C_MANAGER_0_SCL=22"
                                                            "-DCONFIG_I2C_MANAGER_0_FREQ_HZ=100000"
                                                            "-DCONFIG_I2C_MANAGER_0_TIMEOUT=940"
                                                            "-DCONFIG_I2C_MANAGER_0_LOCK_TIMEOUT=1000"
                                                            "-DCONFIG_I2C_MANAGER_0_PULLUPS=y"
                                                            )

    target_compile_definitions( ${COMPONENT_LIB} PRIVATE    "-DCONFIG_AXP192_GPIO0_LDOIO0_VOLTAGE_BIT74=0x00"
                                                            "-DCONFIG_AXP192_LDO23_VOLTAGE_BIT74=0x00"
                                                            "-DCONFIG_AXP192_LDO23_VOLTAGE_BIT30=0x00"
                                                            "-DCONFIG_AXP192_LDO23_VOLTAGE_BIT74_3V0=y"
                                                            "-DCONFIG_AXP192_LDO23_VOLTAGE_BIT30_3V0=y"
                                                            "-DCONFIG_AXP192_ADC_ENABLE_1_BIT7=0x00"
                                                            "-DCONFIG_AXP192_ADC_ENABLE_1_BIT6=0x00"
                                                            "-DCONFIG_AXP192_ADC_ENABLE_1_BIT5=0x00"
                                                            "-DCONFIG_AXP192_ADC_ENABLE_1_BIT4=0x00"
                                                            "-DCONFIG_AXP192_ADC_ENABLE_1_BIT3=0x00"
                                                            "-DCONFIG_AXP192_ADC_ENABLE_1_BIT2=0x00"
                                                            "-DCONFIG_AXP192_ADC_ENABLE_1_BIT1=0x00"
                                                            "-DCONFIG_AXP192_ADC_ENABLE_1_BIT0=0x00"
                                                            "-DCONFIG_AXP192_CHARGE_CONTROL_1_BIT7=0x00"
                                                            "-DCONFIG_AXP192_CHARGE_CONTROL_1_BIT65=0x00"
                                                            "-DCONFIG_AXP192_CHARGE_CONTROL_1_BIT30=0x00"
                                                            "-DCONFIG_AXP192_CHARGE_CONTROL_1_BIT4=0x00"
                                                            "-DCONFIG_AXP192_EXTEN_DCDC2_CONTROL_BIT2=0x00"
                                                            "-DCONFIG_AXP192_EXTEN_DCDC2_CONTROL_BIT0=0x00"
                                                            "-DCONFIG_AXP192_DCDC13_LDO23_CONTROL_BIT6=0x00"
                                                            "-DCONFIG_AXP192_DCDC13_LDO23_CONTROL_BIT4=0x00"
                                                            "-DCONFIG_AXP192_DCDC13_LDO23_CONTROL_BIT3=0x00"
                                                            "-DCONFIG_AXP192_DCDC13_LDO23_CONTROL_BIT2=0x00"
                                                            "-DCONFIG_AXP192_GPIO0_CONTROL_BIT20=0x07"
                                                            "-DCONFIG_AXP192_DCDC13_LDO23_CONTROL_BIT1=0x00"
                                                            "-DCONFIG_AXP192_DCDC13_LDO23_CONTROL_BIT0=0x00"
                                                            )
endif()


if( CONFIG_SOFTWARE_EXPPORTS_SUPPORT )
    target_compile_definitions( ${COMPONENT_LIB} PRIVATE    "-DCONFIG_I2C_MANAGER_1_ENABLED=y"
                                                            "-DCONFIG_I2C_MANAGER_1_SDA=32"
                                                            "-DCONFIG_I2C_MANAGER_1_SCL=33"
                                                            )
endif()


if( CONFIG_SOFTWARE_RGB_LED_SUPPORT )
    target_compile_definitions( ${COMPONENT_LIB} PRIVATE "-DCONFIG_LED_STRIP_FLUSH_TIMEOUT=1000" )
    target_compile_definitions( ${COMPONENT_LIB} PRIVATE "-DCONFIG_LED_STRIP_PAUSE_LENGTH=50" )
endif()

if( CONFIG_SOFTWARE_DISPLAY_SUPPORT )
    target_compile_definitions(${COMPONENT_LIB} PUBLIC  "-DCONFIG_LV_HOR_RES_MAX=320"
                                                        "-DCONFIG_LV_VER_RES_MAX=240"
                                                        "-DLV_CONF_SKIP=y"
                                                        "-DCONFIG_LV_COLOR_DEPTH_16=y"
                                                        "-DCONFIG_LV_COLOR_DEPTH=16"
                                                        "-DCONFIG_LV_I2C=y"
                                                        "-DCONFIG_LV_PREDEFINED_DISPLAY_M5CORE2=y"
                                                        "-DCONFIG_LV_TFT_DISPLAY_CONTROLLER_ILI9341=y"
                                                        "-DCONFIG_LV_TFT_DISPLAY_PROTOCOL_SPI=y"
                                                        "-DCONFIG_LV_DISPLAY_ORIENTATION_PORTRAIT=y"
                                                        "-DCONFIG_LV_DISPLAY_ORIENTATION=0"
                                                        "-DCONFIG_LV_TFT_DISPLAY_SPI3_HOST=y"
                                                        "-DCONFIG_LV_TFT_DISPLAY_SPI_TRANS_MODE_DIO=y"
                                                        "-DCONFIG_LV_TFT_DISPLAY_SPI_HALF_DUPLEX=y"
                                                        "-DCONFIG_LV_DISP_BACKLIGHT_OFF=y"
                                                        "-DCONFIG_LV_DISP_SPI_MOSI=23"
                                                        "-DCONFIG_LV_DISPLAY_USE_SPI_MISO=y"
                                                        "-DCONFIG_LV_DISP_SPI_MISO=38"
                                                        "-DCONFIG_LV_DISP_SPI_INPUT_DELAY_NS=0"
                                                        "-DCONFIG_LV_DISP_SPI_CLK=18"
                                                        "-DCONFIG_LV_DISPLAY_USE_SPI_CS=y"
                                                        "-DCONFIG_LV_DISP_SPI_CS=5"
                                                        "-DCONFIG_LV_DISPLAY_USE_DC=y"
                                                        "-DCONFIG_LV_DISP_PIN_DC=15"
                                                        "-DCONFIG_LV_DISP_PIN_BUSY=35"
                                                        "-DCONFIG_LV_I2C_DISPLAY_PORT=0"
                                                        "-DCONFIG_LV_TOUCH_CONTROLLER=2"
                                                        "-DCONFIG_LV_TOUCH_CONTROLLER_FT6X06=y"
                                                        "-DCONFIG_LV_I2C_TOUCH=y"
                                                        "-DCONFIG_LV_I2C_TOUCH_PORT_0=y"
                                                        "-DCONFIG_LV_I2C_TOUCH_PORT=0"
                                                        "-DLV_CONF_INCLUDE_SIMPLE"
                                                        "-DLV_LVGL_H_INCLUDE_SIMPLE"
                                                        )
endif()