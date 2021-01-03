set(COMPONENT_ADD_INCLUDEDIRS .)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES misc audio_hal)

set(COMPONENT_SRCS ./is31fl3236.c)

register_component()
