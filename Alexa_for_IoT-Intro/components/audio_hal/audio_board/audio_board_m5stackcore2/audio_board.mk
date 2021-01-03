AUDIO_HAL_PATH := ../..
IPC_DRV_PATH := $(AUDIO_HAL_PATH)/dsp_driver/m5stackcore2_driver

# DSP driver
EXTRA_COMPONENT_DIRS += $(IPC_DRV_PATH)/components/

# LED driver
EXTRA_COMPONENT_DIRS += $(AUDIO_HAL_PATH)/led_driver/sk6812/

# codec
EXTRA_COMPONENT_DIRS += $(AUDIO_HAL_PATH)/esp_codec/no_codec/components/

# LED pattern
export LED_PATTERN_PATH = linear_5/$(LED_PATTERN_TYPE)
EXTRA_COMPONENT_DIRS += $(AUDIO_HAL_PATH)/led_pattern

# M5Stack is Half duplex board, Speaker and Mic cannot work together as they
# share the I2S channel, so this board needs to be treated specially
CFLAGS += -DHALF_DUPLEX_I2S_MODE

# Board
EXTRA_COMPONENT_DIRS += ./audio_board_m5stackcore2/

# va_dsp
-include $(IPC_DRV_PATH)/components/va_dsp/va_dsp.mk

# This var sets partition file to partitions_4mb_flash.csv
# Please take a look at example makefile.
PARTITIONS_4MB_FLASH_CSV = 1
