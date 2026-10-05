# SPDX-FileCopyrightText: 2022 Rashed Talukder
# SPDX-FileCopyrightText: 2022 M5Stack
# SPDX-License-Identifier: MIT

cmake_minimum_required( VERSION 3.16.0 )

if( NOT "$ENV{CORE2FORAWS_HARDWARE_TESTS}" STREQUAL "1" )
    message( FATAL_ERROR
        "This test writes SD/NVS data and changes peripheral state. Set CORE2FORAWS_HARDWARE_TESTS=1 to acknowledge and build it." )
endif()

set( EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../.." )
include( $ENV{IDF_PATH}/tools/cmake/project.cmake )
project( core2foraws_hardware_tests )