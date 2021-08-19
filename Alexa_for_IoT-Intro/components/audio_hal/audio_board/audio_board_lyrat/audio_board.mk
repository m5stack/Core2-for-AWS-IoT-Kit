AUDIO_HAL_PATH := $(AUDIO_BOARD_PATH)/../../
IPC_DRV_PATH := $(AUDIO_HAL_PATH)/dsp_driver/lyrat_driver

# Board
EXTRA_COMPONENT_DIRS += $(AUDIO_BOARD_PATH)/

# DSP driver
EXTRA_COMPONENT_DIRS += $(IPC_DRV_PATH)/components/

# LED driver
EXTRA_COMPONENT_DIRS += $(AUDIO_HAL_PATH)/led_driver/esp_ledc

# codec
EXTRA_COMPONENT_DIRS += $(AUDIO_HAL_PATH)/esp_codec/es8388/components/

# LED pattern
export LED_PATTERN_PATH = single/single_color/$(LED_PATTERN_TYPE)
EXTRA_COMPONENT_DIRS += $(AUDIO_HAL_PATH)/led_pattern

# va_dsp
-include $(IPC_DRV_PATH)/components/va_dsp/va_dsp.mk

# This var sets partition file to partitions_4mb_flash.csv
# Please take a look at example makefile.
PARTITIONS_4MB_FLASH_CSV = 1
