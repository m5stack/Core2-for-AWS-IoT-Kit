set(COMPONENT_ADD_INCLUDEDIRS .)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES voice_assistant misc media_hal audio_hal m5stackcore2_ipc utils)

set(COMPONENT_SRCS ./va_dsp.c)

register_component()
