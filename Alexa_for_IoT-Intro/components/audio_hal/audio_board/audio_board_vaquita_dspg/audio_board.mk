AUDIO_HAL_PATH := $(AUDIO_BOARD_PATH)/../../
IPC_DRV_PATH := $(AUDIO_HAL_PATH)/dsp_driver/dspg_driver

# Board
EXTRA_COMPONENT_DIRS += $(AUDIO_BOARD_PATH)/

# DSP driver
EXTRA_COMPONENT_DIRS += $(IPC_DRV_PATH)/components/

# LED driver
EXTRA_COMPONENT_DIRS += $(AUDIO_HAL_PATH)/led_driver/neo_pixel_led

# codec
EXTRA_COMPONENT_DIRS += $(AUDIO_HAL_PATH)/esp_codec/es8311/components/

# LED pattern
export LED_PATTERN_PATH = linear_5/$(LED_PATTERN_TYPE)
EXTRA_COMPONENT_DIRS += $(AUDIO_HAL_PATH)/led_pattern

# va_dsp
include $(IPC_DRV_PATH)/components/va_dsp/va_dsp.mk
