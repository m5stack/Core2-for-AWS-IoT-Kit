#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := aia
PROJECT_VER := 1.0.2
export VOICE_ASSISTANT = aia
export LED_PATTERN_TYPE = alexa
CPPFLAGS += -DVOICE_ASSISTANT_AIA

ifeq ((xtensa-esp32-elf-gcc --version | grep 'crosstool-ng-1.22.0-95') || false,)
  $(error Please use toolchain version: xtensa-esp32-elf-gcc (crosstool-NG crosstool-ng-1.22.0-95-g5a1864d) 5.2.0)
endif

include components/audio_hal/audio_board/audio_board_m5stackcore2/audio_board.mk

ifneq ($(CLOUD_AGENT_PATH),)
    EXTRA_COMPONENT_DIRS += $(CLOUD_AGENT_PATH)/components
endif

ifdef DSP_EN_SPI_FFS
MKSPIFFS_PATH=tools/mkspiffs
export PARTITION_TABLE_CSV_PATH:=$(CURDIR)/partitions_mkspiffs.csv
endif

ifdef PARTITIONS_4MB_FLASH_CSV
# Set partition to `partitions_4mb_flash.csv`
# This makes sure boards with smaller flash size of 4Mb work without issue.
# var `PARTITIONS_4MB_FLASH_CSV` is set in `audio_board.mk`
export PARTITION_TABLE_CSV_PATH:=$(CURDIR)/partitions_4mb_flash.csv
endif

include $(IDF_PATH)/make/project.mk

ifdef DSP_EN_SPI_FFS
.PHONY: print_spiffs spiffs_flash spiffs_make spiffs_clean
all: build/spiffs-alexa-img.bin print_spiffs
all_binaries: build/spiffs-alexa-img.bin print_spiffs
clean: spiffs_clean
spiffs_flash: spiffs_make flash

SPIFFS_OFFSET := 0x700000
SPIFFS_BIN := $(PROJECT_PATH)/build/spiffs-alexa-img.bin
DETECTED_OS := $(shell uname -s)
DETECTED_ARCHITECTURE := $(shell uname -m)

ifeq ($(DETECTED_OS),Windows)
    MKSPIFFS_EXEC_PATH := $(MKSPIFFS_PATH)/win32/mkspiffs.exe
endif
ifeq ($(DETECTED_OS),Darwin)
    MKSPIFFS_EXEC_PATH := $(MKSPIFFS_PATH)/osx/mkspiffs
endif
ifeq ($(DETECTED_OS),Linux)
	ifeq ($(DETECTED_ARCHITECTURE),x86_64)
		MKSPIFFS_EXEC_PATH := $(MKSPIFFS_PATH)/linux64/mkspiffs
	else
		MKSPIFFS_EXEC_PATH := $(MKSPIFFS_PATH)/linux32/mkspiffs
	endif
endif

build/spiffs-alexa-img.bin:
	$(MKSPIFFS_EXEC_PATH) -c $(IPC_DRV_PATH)/firmware -b 4096 -p 256 -s 0x100000 $@
spiffs_make: build/spiffs-alexa-img.bin
	$(eval ESPTOOL_ALL_FLASH_ARGS += $(SPIFFS_OFFSET) $(SPIFFS_BIN))

spiffs_clean:
	rm -f build/spiffs-alexa-img.bin

print_spiffs:
	echo "To also flash spiffs, run 'make spiffs_flash' or add the following line to the python flash command: "
	echo $(SPIFFS_OFFSET) $(SPIFFS_BIN)
endif
