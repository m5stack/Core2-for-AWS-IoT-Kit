# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(VOICE_ASSISTANT "aia" CACHE INTERNAL "")
set(LED_PATTERN_TYPE "alexa" CACHE INTERNAL "")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(AUDIO_BOARD_PATH "${CMAKE_CURRENT_SOURCE_DIR}/components/audio_hal/audio_board/audio_board_m5_core2_aws")
include(${AUDIO_BOARD_PATH}/audio_board.cmake)

if(DEFINED ENV{CLOUD_AGENT_PATH})
    idf_build_set_property(CLOUD_AGENT 1)
    set(CLOUD_AGENT_PATH $ENV{CLOUD_AGENT_PATH})
    list(APPEND EXTRA_COMPONENT_DIRS ${CLOUD_AGENT_PATH}/components)
endif()

idf_build_set_property(COMPILE_OPTIONS "-DVOICE_ASSISTANT_AIA" APPEND)

idf_build_get_property(enable_4mb ENABLE_4MB_FLASH_PARTITION)
if(${enable_4mb})
    idf_build_set_property(COMPILE_OPTIONS "-DENABLE_4MB_FLASH_PARTITION" APPEND)
endif()

project(aia)

# SPIFFS specific configuration
idf_build_get_property(enable_spiffs ENABLE_DSP_SPIFFS_PARTITION)
if(${enable_spiffs})
    spiffs_create_partition_image(storage ${DSP_DRIVER_PATH}/dsp_firmware FLASH_IN_PROJECT)
endif()
