#
# CMakeLists.txt file for cryptoauthlib
#
cmake_minimum_required(VERSION 3.5)
set(CRYPTOAUTHLIB_DIR "cryptoauthlib/lib")
set(COMPONENT_SRCDIRS       "${CRYPTOAUTHLIB_DIR}/atcacert"
                            "${CRYPTOAUTHLIB_DIR}/calib"
                            "${CRYPTOAUTHLIB_DIR}/crypto"
                            "${CRYPTOAUTHLIB_DIR}/crypto/hashes"
                            "${CRYPTOAUTHLIB_DIR}/host"
                            "${CRYPTOAUTHLIB_DIR}/mbedtls"
                            "${CRYPTOAUTHLIB_DIR}"
                            "${CRYPTOAUTHLIB_DIR}/../app/tng"
                            "port"
                            )

# The Core2 for AWS HAL (hal_core2foraws.c) replaces the upstream hal_esp32_i2c.c
set(COMPONENT_SRCS          "${COMPONENT_DIR}/cryptoauthlib/lib/hal/atca_hal.c"
                            "${COMPONENT_DIR}/cryptoauthlib/lib/hal/hal_freertos.c"
                            "${COMPONENT_DIR}/cryptoauthlib/third_party/hal/esp32/hal_core2foraws.c"
                            "${COMPONENT_DIR}/cryptoauthlib/third_party/hal/esp32/hal_esp32_timer.c"
                            "${COMPONENT_DIR}/cryptoauthlib/third_party/atca_mbedtls_patch.c"
                            )

set(COMPONENT_INCLUDEDIRS   "${CRYPTOAUTHLIB_DIR}/"
                            "${CRYPTOAUTHLIB_DIR}/hal"
                            "${CRYPTOAUTHLIB_DIR}/../app/tng"
                            "${CRYPTOAUTHLIB_DIR}/../third_party"
                            "port"
                            )

set(COMPONENT_REQUIRES      "mbedtls" "freertos"  "driver" "core2forAWS")

# Don't include the default interface configurations from cryptoauthlib
set(COMPONENT_EXCLUDE_SRCS "${CRYPTOAUTHLIB_DIR}/atca_cfgs.c")
set(COMPONENT_CFLAGS "ESP32")

idf_component_register(     SRC_DIRS        "${COMPONENT_SRCDIRS}"
                            INCLUDE_DIRS    "${COMPONENT_INCLUDEDIRS}"
                            REQUIRES        "${COMPONENT_REQUIRES}"
                            EXCLUDE_SRCS    "${COMPONENT_EXCLUDE_SRCS}"
                            )

target_sources(${COMPONENT_LIB} PRIVATE ${COMPONENT_SRCS})
target_compile_definitions(${COMPONENT_LIB} PRIVATE ${COMPONENT_CFLAGS})
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-pointer-sign -Wno-maybe-uninitialized -Wno-nonnull)

if (CONFIG_COMPILER_OPTIMIZATION_SIZE OR CONFIG_COMPILER_OPTIMIZATION_PERF)
    target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-stringop-overflow)
endif()

# hal_freertos.c includes FreeRTOS headers without the "freertos/" prefix
idf_component_get_property(freertos_dir freertos COMPONENT_DIR)
set_source_files_properties(${CRYPTOAUTHLIB_DIR}/hal/hal_freertos.c PROPERTIES COMPILE_FLAGS -I${freertos_dir}/include/freertos)

# Suppress -Wdiscarded-qualifiers for tng_atcacert_client.c
set_source_files_properties(${CRYPTOAUTHLIB_DIR}/../app/tng/tng_atcacert_client.c PROPERTIES COMPILE_FLAGS -Wno-discarded-qualifiers)
