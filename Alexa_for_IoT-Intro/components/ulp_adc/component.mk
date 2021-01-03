ifdef CONFIG_ULP_COPROC_ENABLED

ULP_APP_NAME ?= ulp_adc_button

ULP_S_SOURCES = $(COMPONENT_PATH)/ulp/adc_button.S

ULP_EXP_DEP_OBJECTS := ulp_adc.o

# NOTE: Separate toolchain needs to be installed for compiling ULP code.
# Toolchain setup can be found at https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/ulp.html#installing-the-toolchain
#
include $(IDF_PATH)/components/ulp/component_ulp_common.mk

COMPONENT_SRCDIRS := ./
COMPONENT_ADD_INCLUDEDIRS := include
else
COMPONENT_OBJEXCLUDE := ulp_adc.o
endif
