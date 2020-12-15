set(COMPONENT_SRCS "main.c" "blink.c" "display.c")
set(COMPONENT_ADD_INCLUDEDIRS "." "./includes")

register_component()

if(CONFIG_EXAMPLE_EMBEDDED_CERTS)
    target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
    target_add_binary_data(${COMPONENT_TARGET} "certs/certificate.pem.crt" TEXT)
    target_add_binary_data(${COMPONENT_TARGET} "certs/private.pem.key" TEXT)
endif()

if(CONFIG_AWS_IOT_USE_HARDWARE_SECURE_ELEMENT)
    target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
endif()