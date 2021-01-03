set(COMPONENT_ADD_INCLUDEDIRS include)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES misc)

set(COMPONENT_SRCS media_hal.c)

register_component()

# This fixes the circular dependecies error seen in cmake. The linking error is seen in a few more components, but just adding it here fixes all of them.
set_property(TARGET ${COMPONENT_LIB} APPEND PROPERTY LINK_INTERFACE_MULTIPLICITY 4)
