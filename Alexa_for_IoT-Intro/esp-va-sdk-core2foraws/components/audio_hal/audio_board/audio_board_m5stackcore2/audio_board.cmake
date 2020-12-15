# Designed to be included from an IDF app's CMakeLists.txt file
#
cmake_minimum_required(VERSION 3.5)
set(AUDIO_HAL_PATH $ENV{AUDIO_BOARD_PATH}/../..)

if((NOT(DEFINED CORE2FORAWS_PATH)) AND (NOT(DEFINED ENV{CORE2FORAWS_PATH})))
    message(STATUS  "CORE2FORAWS_PATH not defined $ENV{CORE2FORAWS_PATH}")
    return()
endif()

if(NOT (DEFINED CORE2FORAWS_PATH))
    message(STATUS  "CORE2FORAWS_PATH $ENV{CORE2FORAWS_PATH}")
    set(CORE2FORAWS_PATH $ENV{CORE2FORAWS_PATH})
endif()

set(IPC_DRV_PATH ${AUDIO_HAL_PATH}/dsp_driver/m5stackcore2_driver)

set(LED_PATTERN_PATH "linear_5/${LED_PATTERN_TYPE}" CACHE INTERNAL "")

# M5Stack is Half duplex board, Speaker and Mic cannot work together as they
# share the I2S channel, so this board needs to be treated specially
add_definitions(-DHALF_DUPLEX_I2S_MODE)

list(APPEND EXTRA_COMPONENT_DIRS
    $ENV{AUDIO_BOARD_PATH}/
    ${IPC_DRV_PATH}/components/
    ${AUDIO_HAL_PATH}/esp_codec/no_codec/components/no_codec/
    ${IPC_DRV_PATH}/components/va_dsp/
    ${AUDIO_HAL_PATH}/led_pattern/
    ${CORE2FORAWS_PATH}/components/)

include(${IPC_DRV_PATH}/components/va_dsp/va_dsp.cmake)

# This var sets partition file to partitions_4mb_flash.csv. Please take a look at example's CMakeLists.txt.
set(PARTITIONS_4MB_FLASH_CSV 1)
