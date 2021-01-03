set(COMPONENT_ADD_INCLUDEDIRS .)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES core2forAWS no_codec led_pattern m5stackcore2_ipc streams audio_hal misc media_hal)

set(COMPONENT_SRCS ./audio_board_m5stackcore2.c ./va_board.c)

register_component()
