# Designed to be included from an IDF app's CMakeLists.txt file
cmake_minimum_required(VERSION 3.5)
set(AUDIO_HAL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/components/audio_hal)

set(IPC_DRV_PATH ${AUDIO_HAL_PATH}/dsp_driver/m5stackcore2_driver)

set(LED_PATTERN_PATH "linear_5/${LED_PATTERN_TYPE}" CACHE INTERNAL "")

list(APPEND EXTRA_COMPONENT_DIRS
    ${AUDIO_HAL_PATH}/audio_board/audio_board_m5stackcore2/
    ${IPC_DRV_PATH}/components/
    ${AUDIO_HAL_PATH}/esp_codec/no_codec/components/no_codec/
    ${IPC_DRV_PATH}/components/va_dsp/
    ${AUDIO_HAL_PATH}/led_pattern/)

include(${IPC_DRV_PATH}/components/va_dsp/va_dsp.cmake)
