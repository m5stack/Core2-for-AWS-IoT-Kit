# Designed to be included from an IDF app's CMakeLists.txt file
#
cmake_minimum_required(VERSION 3.5)
set(AUDIO_HAL_PATH $ENV{AUDIO_BOARD_PATH}/../../)
set(IPC_DRV_PATH ${AUDIO_HAL_PATH}/dsp_driver/dspg_driver)
set(LED_PATTERN_PATH "radial_12/${LED_PATTERN_TYPE}" CACHE INTERNAL "")
list(APPEND EXTRA_COMPONENT_DIRS
    $ENV{AUDIO_BOARD_PATH}/
    ${IPC_DRV_PATH}/components/
    ${AUDIO_HAL_PATH}/led_driver/is31fl3236
    ${AUDIO_HAL_PATH}/led_pattern
    ${AUDIO_HAL_PATH}/esp_codec/es8388/components/)
include(${IPC_DRV_PATH}/components/va_dsp/va_dsp.cmake)

