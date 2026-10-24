# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(PROJECT_VER "v1.2.0")
add_compile_definitions(PROJECT_VER="${PROJECT_VER}")

# firmware generated for esp_cryptoauth_utility
project(ecu_firmware)
