set(COMPONENT_ADD_INCLUDEDIRS .)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES media_hal)
set(COMPONENT_PRIV_REQUIRES audio_hal)

set(COMPONENT_SRCS ./no_codec.c ./media_hal_codec_init.c)

register_component()
