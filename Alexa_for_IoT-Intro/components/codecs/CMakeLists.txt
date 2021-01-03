idf_component_register(INCLUDE_DIRS "include")

add_prebuilt_library(codec ${COMPONENT_DIR}/lib/libcodecs.a REQUIRES utils fatfs)

target_link_libraries(${COMPONENT_LIB} INTERFACE codec)
