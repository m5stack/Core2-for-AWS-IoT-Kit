set(COMPONENT_ADD_INCLUDEDIRS . include/)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES streams )
set(COMPONENT_PRIV_REQUIRES core2forAWS spi_flash misc audio_hal speech_recog streams codecs)

set(COMPONENT_SRCS ./m5stackcore2_dsp_init.c)

register_component()
