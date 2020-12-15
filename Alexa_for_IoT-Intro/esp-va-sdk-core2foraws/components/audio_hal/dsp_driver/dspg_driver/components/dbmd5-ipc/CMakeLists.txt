idf_component_register(INCLUDE_DIRS "include")

add_prebuilt_library(dspg_ipc ${COMPONENT_DIR}/lib/libdspg-ipc.a REQUIRES spi_flash misc audio_hal)

target_link_libraries(${COMPONENT_LIB} INTERFACE dspg_ipc)
