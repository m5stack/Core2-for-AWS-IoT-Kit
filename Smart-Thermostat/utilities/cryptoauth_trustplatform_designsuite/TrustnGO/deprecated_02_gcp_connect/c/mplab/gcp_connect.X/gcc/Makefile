################################################################################
# Automatically-generated file. Do not edit!
################################################################################

ifdef SystemRoot
	SHELL = cmd.exe
	MK_DIR = mkdir
else
	ifeq ($(shell uname), Linux)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), CYGWIN)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), MINGW32)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), MINGW64)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), DARWIN)
		MK_DIR = mkdir -p
	endif
endif

# List the subdirectories for creating object files
SUB_DIRS +=  \
 \
hpl/tcc \
examples \
hpl/systick \
stdio_redirect \
hpl/dmac \
winc1500/socket/source \
hal/src \
stdio_redirect/gcc \
hpl/eic \
winc1500/common/source \
hpl/rtc \
samd21a/gcc/gcc \
winc1500/spi_flash/source \
winc1500/bsp/source \
hpl/pm \
hpl/sysctrl \
hpl/gclk \
hal/utils/src \
samd21a/gcc \
winc1500/driver/source \
winc1500/bus_wrapper/source \
hpl/sercom \
hpl/core

# List the object files
OBJS +=  \
hal/src/hal_io.o \
stdio_redirect/stdio_io.o \
hpl/systick/hpl_systick.o \
winc1500/common/source/nm_common.o \
winc1500/bsp/source/nm_bsp.o \
hal/src/hal_calendar.o \
winc1500/driver/source/nmi2c.o \
samd21a/gcc/gcc/startup_samd21.o \
winc1500/bus_wrapper/source/nm_bus_wrapper.o \
winc1500/driver/source/nmbus.o \
hpl/eic/hpl_eic.o \
hal/utils/src/utils_syscalls.o \
stdio_redirect/gcc/read.o \
hal/src/hal_spi_m_sync.o \
hal/src/hal_timer.o \
winc1500/spi_flash/source/spi_flash.o \
winc_init.o \
hal/src/hal_i2c_m_sync.o \
winc1500/driver/source/nmdrv.o \
hal/src/hal_delay.o \
winc1500/driver/source/m2m_ssl.o \
hpl/sysctrl/hpl_sysctrl.o \
hpl/core/hpl_init.o \
hal/utils/src/utils_list.o \
hpl/core/hpl_core_m0plus_base.o \
winc1500/driver/source/m2m_hif.o \
winc1500/socket/source/socket.o \
hal/utils/src/utils_assert.o \
hpl/dmac/hpl_dmac.o \
hpl/pm/hpl_pm.o \
hal/src/hal_usart_sync.o \
winc1500/driver/source/nmasic.o \
hpl/sercom/hpl_sercom.o \
hpl/gclk/hpl_gclk.o \
stdio_start.o \
hal/src/hal_init.o \
main.o \
samd21a/gcc/system_samd21.o \
winc1500/driver/source/m2m_ate_mode.o \
hpl/rtc/hpl_rtc.o \
winc1500/driver/source/m2m_periph.o \
examples/driver_examples.o \
driver_init.o \
winc1500/driver/source/m2m_crypto.o \
hal/src/hal_ext_irq.o \
winc1500/driver/source/nmspi.o \
hal/src/hal_gpio.o \
hal/utils/src/utils_event.o \
hal/src/hal_sleep.o \
winc1500/driver/source/m2m_wifi.o \
winc1500/driver/source/m2m_ota.o \
hpl/tcc/hpl_tcc.o \
winc1500/driver/source/nmuart.o \
atmel_start.o \
hal/src/hal_atomic.o \
stdio_redirect/gcc/write.o

OBJS_AS_ARGS +=  \
"hal/src/hal_io.o" \
"stdio_redirect/stdio_io.o" \
"hpl/systick/hpl_systick.o" \
"winc1500/common/source/nm_common.o" \
"winc1500/bsp/source/nm_bsp.o" \
"hal/src/hal_calendar.o" \
"winc1500/driver/source/nmi2c.o" \
"samd21a/gcc/gcc/startup_samd21.o" \
"winc1500/bus_wrapper/source/nm_bus_wrapper.o" \
"winc1500/driver/source/nmbus.o" \
"hpl/eic/hpl_eic.o" \
"hal/utils/src/utils_syscalls.o" \
"stdio_redirect/gcc/read.o" \
"hal/src/hal_spi_m_sync.o" \
"hal/src/hal_timer.o" \
"winc1500/spi_flash/source/spi_flash.o" \
"winc_init.o" \
"hal/src/hal_i2c_m_sync.o" \
"winc1500/driver/source/nmdrv.o" \
"hal/src/hal_delay.o" \
"winc1500/driver/source/m2m_ssl.o" \
"hpl/sysctrl/hpl_sysctrl.o" \
"hpl/core/hpl_init.o" \
"hal/utils/src/utils_list.o" \
"hpl/core/hpl_core_m0plus_base.o" \
"winc1500/driver/source/m2m_hif.o" \
"winc1500/socket/source/socket.o" \
"hal/utils/src/utils_assert.o" \
"hpl/dmac/hpl_dmac.o" \
"hpl/pm/hpl_pm.o" \
"hal/src/hal_usart_sync.o" \
"winc1500/driver/source/nmasic.o" \
"hpl/sercom/hpl_sercom.o" \
"hpl/gclk/hpl_gclk.o" \
"stdio_start.o" \
"hal/src/hal_init.o" \
"main.o" \
"samd21a/gcc/system_samd21.o" \
"winc1500/driver/source/m2m_ate_mode.o" \
"hpl/rtc/hpl_rtc.o" \
"winc1500/driver/source/m2m_periph.o" \
"examples/driver_examples.o" \
"driver_init.o" \
"winc1500/driver/source/m2m_crypto.o" \
"hal/src/hal_ext_irq.o" \
"winc1500/driver/source/nmspi.o" \
"hal/src/hal_gpio.o" \
"hal/utils/src/utils_event.o" \
"hal/src/hal_sleep.o" \
"winc1500/driver/source/m2m_wifi.o" \
"winc1500/driver/source/m2m_ota.o" \
"hpl/tcc/hpl_tcc.o" \
"winc1500/driver/source/nmuart.o" \
"atmel_start.o" \
"hal/src/hal_atomic.o" \
"stdio_redirect/gcc/write.o"

# List the directories containing header files
DIR_INCLUDES +=  \
-I"../" \
-I"../config" \
-I"../examples" \
-I"../hal/include" \
-I"../hal/utils/include" \
-I"../hpl/core" \
-I"../hpl/dmac" \
-I"../hpl/eic" \
-I"../hpl/gclk" \
-I"../hpl/pm" \
-I"../hpl/port" \
-I"../hpl/rtc" \
-I"../hpl/sercom" \
-I"../hpl/sysctrl" \
-I"../hpl/systick" \
-I"../hpl/tcc" \
-I"../hri" \
-I"../" \
-I"../config" \
-I"../winc1500" \
-I"../" \
-I"../config" \
-I"../stdio_redirect" \
-I"../" \
-I"../CMSIS/Core/Include" \
-I"../samd21a/include"

# List the dependency files
DEPS := $(OBJS:%.o=%.d)

DEPS_AS_ARGS +=  \
"winc1500/bsp/source/nm_bsp.d" \
"winc1500/driver/source/nmi2c.d" \
"hal/src/hal_calendar.d" \
"stdio_redirect/stdio_io.d" \
"hal/utils/src/utils_event.d" \
"hal/src/hal_io.d" \
"hpl/systick/hpl_systick.d" \
"hpl/eic/hpl_eic.d" \
"hal/utils/src/utils_syscalls.d" \
"winc1500/bus_wrapper/source/nm_bus_wrapper.d" \
"winc1500/spi_flash/source/spi_flash.d" \
"winc1500/driver/source/nmbus.d" \
"winc1500/driver/source/nmdrv.d" \
"hal/src/hal_i2c_m_sync.d" \
"winc_init.d" \
"hal/src/hal_timer.d" \
"hal/src/hal_spi_m_sync.d" \
"winc1500/socket/source/socket.d" \
"winc1500/driver/source/m2m_hif.d" \
"hpl/core/hpl_core_m0plus_base.d" \
"hal/utils/src/utils_list.d" \
"stdio_redirect/gcc/write.d" \
"hpl/dmac/hpl_dmac.d" \
"hal/utils/src/utils_assert.d" \
"winc1500/driver/source/m2m_ssl.d" \
"hal/src/hal_delay.d" \
"hpl/core/hpl_init.d" \
"hpl/sysctrl/hpl_sysctrl.d" \
"stdio_start.d" \
"hpl/gclk/hpl_gclk.d" \
"winc1500/driver/source/m2m_ota.d" \
"samd21a/gcc/gcc/startup_samd21.d" \
"hal/src/hal_init.d" \
"hal/src/hal_usart_sync.d" \
"stdio_redirect/gcc/read.d" \
"driver_init.d" \
"winc1500/common/source/nm_common.d" \
"winc1500/driver/source/m2m_crypto.d" \
"samd21a/gcc/system_samd21.d" \
"main.d" \
"examples/driver_examples.d" \
"winc1500/driver/source/m2m_periph.d" \
"hpl/rtc/hpl_rtc.d" \
"winc1500/driver/source/m2m_ate_mode.d" \
"winc1500/driver/source/m2m_wifi.d" \
"hal/src/hal_sleep.d" \
"hpl/sercom/hpl_sercom.d" \
"hal/src/hal_gpio.d" \
"winc1500/driver/source/nmspi.d" \
"atmel_start.d" \
"hal/src/hal_atomic.d" \
"winc1500/driver/source/nmasic.d" \
"hpl/pm/hpl_pm.d" \
"winc1500/driver/source/nmuart.d" \
"hpl/tcc/hpl_tcc.d" \
"hal/src/hal_ext_irq.d"

OUTPUT_FILE_NAME :=AtmelStart
QUOTE := "
OUTPUT_FILE_PATH +=$(OUTPUT_FILE_NAME).elf
OUTPUT_FILE_PATH_AS_ARGS +=$(OUTPUT_FILE_NAME).elf

vpath %.c ../
vpath %.s ../
vpath %.S ../

# All Target
all: $(SUB_DIRS) $(OUTPUT_FILE_PATH)

# Linker target

$(OUTPUT_FILE_PATH): $(OBJS)
	@echo Building target: $@
	@echo Invoking: ARM/GNU Linker
	$(QUOTE)arm-none-eabi-gcc$(QUOTE) -o $(OUTPUT_FILE_NAME).elf $(OBJS_AS_ARGS) -Wl,--start-group -lm -Wl,--end-group -mthumb \
-Wl,-Map="$(OUTPUT_FILE_NAME).map" --specs=nano.specs -Wl,--gc-sections -mcpu=cortex-m0plus \
 \
-T"../samd21a/gcc/gcc/samd21e18a_flash.ld" \
-L"../samd21a/gcc/gcc"
	@echo Finished building target: $@

	"arm-none-eabi-objcopy" -O binary "$(OUTPUT_FILE_NAME).elf" "$(OUTPUT_FILE_NAME).bin"
	"arm-none-eabi-objcopy" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  \
        "$(OUTPUT_FILE_NAME).elf" "$(OUTPUT_FILE_NAME).hex"
	"arm-none-eabi-objcopy" -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma \
        .eeprom=0 --no-change-warnings -O binary "$(OUTPUT_FILE_NAME).elf" \
        "$(OUTPUT_FILE_NAME).eep" || exit 0
	"arm-none-eabi-objdump" -h -S "$(OUTPUT_FILE_NAME).elf" > "$(OUTPUT_FILE_NAME).lss"
	"arm-none-eabi-size" "$(OUTPUT_FILE_NAME).elf"

	

# Compiler targets




%.o: %.c
	@echo Building file: $<
	@echo ARM/GNU C Compiler
	$(QUOTE)arm-none-eabi-gcc$(QUOTE) -x c -mthumb -DDEBUG -Os -ffunction-sections -mlong-calls -g3 -Wall -c -std=gnu99 \
-D__SAMD21E18A__ -mcpu=cortex-m0plus  \
$(DIR_INCLUDES) \
-MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"  -o "$@" "$<"
	@echo Finished building: $<

%.o: %.s
	@echo Building file: $<
	@echo ARM/GNU Assembler
	$(QUOTE)arm-none-eabi-as$(QUOTE) -x c -mthumb -DDEBUG -Os -ffunction-sections -mlong-calls -g3 -Wall -c -std=gnu99 \
-D__SAMD21E18A__ -mcpu=cortex-m0plus  \
$(DIR_INCLUDES) \
-MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"  -o "$@" "$<"
	@echo Finished building: $<

%.o: %.S
	@echo Building file: $<
	@echo ARM/GNU Preprocessing Assembler
	$(QUOTE)arm-none-eabi-gcc$(QUOTE) -x c -mthumb -DDEBUG -Os -ffunction-sections -mlong-calls -g3 -Wall -c -std=gnu99 \
-D__SAMD21E18A__ -mcpu=cortex-m0plus  \
$(DIR_INCLUDES) \
-MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"  -o "$@" "$<"
	@echo Finished building: $<

# Detect changes in the dependent files and recompile the respective object files.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

$(SUB_DIRS):
	$(MK_DIR) "$@"

clean:
	rm -f $(OBJS_AS_ARGS)
	rm -f $(OUTPUT_FILE_PATH)
	rm -f $(DEPS_AS_ARGS)
	rm -f $(OUTPUT_FILE_NAME).a $(OUTPUT_FILE_NAME).hex $(OUTPUT_FILE_NAME).bin \
        $(OUTPUT_FILE_NAME).lss $(OUTPUT_FILE_NAME).eep $(OUTPUT_FILE_NAME).map \
        $(OUTPUT_FILE_NAME).srec