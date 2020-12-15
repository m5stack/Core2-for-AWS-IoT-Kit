idf_component_register()

add_prebuilt_library(va_dsp ${COMPONENT_DIR}/lib/libva_dsp.a REQUIRES voice_assistant misc media_hal audio_hal dbmd5-ipc utils)

target_link_libraries(${COMPONENT_LIB} INTERFACE va_dsp)
