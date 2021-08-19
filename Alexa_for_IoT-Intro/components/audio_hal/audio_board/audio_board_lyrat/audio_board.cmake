# Designed to be included from an IDF app's CMakeLists.txt file
#
cmake_minimum_required(VERSION 3.5)
set(AUDIO_HAL_PATH $ENV{AUDIO_BOARD_PATH}/../../)
set(IPC_DRV_PATH ${AUDIO_HAL_PATH}/dsp_driver/lyrat_driver)
set(LED_PATTERN_PATH "single/single_color/${LED_PATTERN_TYPE}" CACHE INTERNAL "")
list(APPEND EXTRA_COMPONENT_DIRS
    $ENV{AUDIO_BOARD_PATH}/
    ${IPC_DRV_PATH}/components/
    ${AUDIO_HAL_PATH}/led_driver/esp_ledc
    ${AUDIO_HAL_PATH}/led_pattern
    ${AUDIO_HAL_PATH}/esp_codec/es8388/components/)
include(${IPC_DRV_PATH}/components/va_dsp/va_dsp.cmake)

# This var sets partition file to partitions_4mb_flash.csv. Please take a look at example's CMakeLists.txt.
set(PARTITIONS_4MB_FLASH_CSV 1)
