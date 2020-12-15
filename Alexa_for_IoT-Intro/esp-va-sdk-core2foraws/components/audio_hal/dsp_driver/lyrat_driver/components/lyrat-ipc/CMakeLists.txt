set(COMPONENT_ADD_INCLUDEDIRS . include/)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES spi_flash misc audio_hal speech_recog streams codecs)

set(COMPONENT_SRCS ./lyrat_init.c)

register_component()
