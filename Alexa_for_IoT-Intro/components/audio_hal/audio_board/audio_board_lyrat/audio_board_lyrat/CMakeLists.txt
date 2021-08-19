set(COMPONENT_ADD_INCLUDEDIRS .)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES audio_hal misc media_hal lyrat-ipc led_pattern)

set(COMPONENT_SRCS ./audio_board_lyrat.c ./va_board.c)

register_component()
