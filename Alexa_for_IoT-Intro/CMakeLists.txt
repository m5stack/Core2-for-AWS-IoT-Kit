
# (Automatically converted from project Makefile by convert_to_cmake.py.)

# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(VOICE_ASSISTANT "aia" CACHE INTERNAL "")
set(LED_PATTERN_TYPE "alexa" CACHE INTERNAL "")

include(components/audio_hal/audio_board/audio_board_m5stackcore2/audio_board.cmake)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

if(DEFINED ENV{CLOUD_AGENT_PATH})
    idf_build_set_property(CLOUD_AGENT 1)
    set(CLOUD_AGENT_PATH $ENV{CLOUD_AGENT_PATH})
    list(APPEND EXTRA_COMPONENT_DIRS ${CLOUD_AGENT_PATH}/components)
endif()

idf_build_set_property(COMPILE_OPTIONS "-DVOICE_ASSISTANT_AIA" APPEND)

if(DEFINED PARTITIONS_4MB_FLASH_CSV)
    set(ENV{PARTITION_TABLE_CSV_PATH} CMAKE_CURRENT_SOURCE_DIR/partitions_4mb_flash.csv)
endif()

# SPIFFS specific configuration
if(DEFINED DSP_EN_SPI_FFS)
    set(ENV{PARTITION_TABLE_CSV_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/partitions_mkspiffs.csv)
    set(MKSPIFFS_PATH tools/mkspiffs)

    execute_process(COMMAND uname -s OUTPUT_VARIABLE DETECTED_OS)
    if(${DETECTED_OS} STRGREATER_EQUAL "Windows")
        set(MKSPIFFS_EXEC_PATH ${MKSPIFFS_PATH}/win32/mkspiffs.exe)
    elseif(${DETECTED_OS} STRGREATER_EQUAL "Darwin")
        set(MKSPIFFS_EXEC_PATH "${MKSPIFFS_PATH}/osx/mkspiffs")
    elseif(${DETECTED_OS} STRGREATER_EQUAL "Linux")
        execute_process(COMMAND uname -m OUTPUT_VARIABLE DETECTED_ARCHITECTURE)
        if(${DETECTED_ARCHITECTURE} STRGREATER_EQUAL x86_64)
            set(MKSPIFFS_EXEC_PATH ${MKSPIFFS_PATH}/linux64/mkspiffs)
        else()
            set(MKSPIFFS_EXEC_PATH ${MKSPIFFS_PATH}/linux32/mkspiffs)
        endif()
    endif()

    set(SPIFFS_OFFSET 0x700000)
    set(SPIFFS_BIN ${CMAKE_CURRENT_SOURCE_DIR}/build/spiffs-alexa-img.bin)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/build/spiffs-alexa-img.bin
        COMMAND ${MKSPIFFS_EXEC_PATH} -c ${IPC_DRV_PATH}/firmware -b 4096 -p 256 -s 0x100000 ${CMAKE_CURRENT_SOURCE_DIR}/build/spiffs-alexa-img.bin)
    add_custom_target(spiffs_make ALL
        DEPENDS build/spiffs-alexa-img.bin)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/build/spiffs-alexa-img.bin")
endif()

project(AWS_IoT_EduKit-Intro_to_Alexa VERSION 1.1.0)

if (DEFINED DSP_EN_SPI_FFS)
    esptool_py_custom_target(spiffs_flash spiffs_flash "spiffs_make")
    add_dependencies(flash spiffs_flash)
    configure_file(
        "flash_spiffs_flash_args.in"
        "${CMAKE_BINARY_DIR}/flash_spiffs_flash_args"
        )
endif()
