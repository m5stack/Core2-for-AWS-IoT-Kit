set(COMPONENT_ADD_INCLUDEDIRS .)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES audio_hal misc media_hal dbmd5-ipc is31fl3236 led_pattern)

set(COMPONENT_SRCS ./audio_board_dspg.c ./va_board.c)

register_component()
