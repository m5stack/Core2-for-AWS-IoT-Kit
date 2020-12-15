set(COMPONENT_ADD_INCLUDEDIRS include)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES utils)

set(COMPONENT_SRCS src/UUIDGeneration.c)

register_component()
