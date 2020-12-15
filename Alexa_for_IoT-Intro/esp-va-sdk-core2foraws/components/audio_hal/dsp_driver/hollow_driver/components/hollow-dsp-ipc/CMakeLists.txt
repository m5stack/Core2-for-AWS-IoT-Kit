set(COMPONENT_ADD_INCLUDEDIRS . include/)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES spi_flash misc audio_hal)

set(COMPONENT_SRCS ./hollow_dsp_init.c)
set(COMPONENT_EMBED_FILES firmware.bin)

register_component()
