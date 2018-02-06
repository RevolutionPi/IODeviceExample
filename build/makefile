

BUILD = STM32F205RE_flash.ld

TOOLCHAIN_PATH = /home/karl/programs/gcc-arm-none-eabi-7-2017-q4-major/bin

CC      = "$(TOOLCHAIN_PATH)/arm-none-eabi-gcc"
LD      = $(CC)
AR      = "$(TOOLCHAIN_PATH)/arm-none-eabi-ar"
AS      = "$(TOOLCHAIN_PATH)/arm-none-eabi-as"
CP      = "$(TOOLCHAIN_PATH)/arm-none-eabi-objcopy"
OD	= "$(TOOLCHAIN_PATH)/arm-none-eabi-objdump"
SIZE	= "$(TOOLCHAIN_PATH)/arm-none-eabi-size"
DEL     = rm -f

INCLUDE_DIRS =  -I common/sw
INCLUDE_DIRS += -I compiler/sw/gnuArm
INCLUDE_DIRS += -I bsp/stm32f2xx/sw
INCLUDE_DIRS += -I bsp/stm32f2xx/sw/SysLib/inc
INCLUDE_DIRS += -I bsp/sw
INCLUDE_DIRS += -I interfaces/sw
INCLUDE_DIRS += -I firmware/sw
INCLUDE_DIRS += -I src
INCLUDE_DIRS += -I utilities/sw
INCLUDE_DIRS += -I ModGateCom/sw
INCLUDE_DIRS += -I modbus/sw

LIBRARY_DIRS = -L bsp/stm32f2xx/sw/SysLib
LIBS = -lc -lgcc

OPTLEVEL = 0

TARGET = PiDio
COMPILE_OPTS = -mcpu=cortex-m3 -mthumb -g -Wall -D __STM32GENERIC__

CFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS)
AFLAGS  = -ahls -mapcs-32 -o crt.o
ARFLAGS = cr
# -mthumb is important for the Linker to take the right libs (if -mthumb is not passed to the linker BLX instruction will be used and they cause faults)
LDFLAGS = -Wl,-Map=$@.map,--cref $(LIBRARY_DIRS) -T $(BUILD) -nostartfiles -mthumb
CPFLAGS = --gap-fill 0xFF -O binary
ODFLAGS	= -D -h -S


MAIN_OUT_ELF = $(TARGET).elf
MAIN_OUT_BIN = $(TARGET).bin

# Board Support Package
SRC += bsp/sw/bsp/bspError.c
SRC += bsp/sw/bsp/systick/systick.c
SRC += bsp/stm32f2xx/sw/bsp/stm32f2xx_vector.c
SRC += bsp/stm32f2xx/sw/bsp/bspInit.c
SRC += bsp/stm32f2xx/sw/bsp/spi/spi.c
SRC += bsp/stm32f2xx/sw/bsp/gpio/gpio.c
SRC += bsp/stm32f2xx/sw/bsp/timer/timer.c
SRC += bsp/stm32f2xx/sw/bsp/timer/timer_count_up.c
SRC += bsp/stm32f2xx/sw/bsp/flash/FlashDriver.c
SRC += bsp/stm32f2xx/sw/bsp/clock/clock.c
SRC += bsp/stm32f2xx/sw/bsp/uart/uart.c
SRC += bsp/stm32f2xx/sw/bsp/eeprom/eeprom.c
SRC += bsp/stm32f2xx/sw/bsp/led/Led.c
SRC += bsp/stm32f2xx/sw/bsp/setjmp/BspSetJmp.c
SRC += bsp/stm32f2xx/sw/bsp/mGateDecode/mGateDecode.c
SRC += bsp/stm32f2xx/sw/SysLib/CM3/core_cm3.c
SRC += bsp/stm32f2xx/sw/SysLib/src/stm32f2xx_gpio.c
SRC += bsp/stm32f2xx/sw/SysLib/src/stm32f2xx_rcc.c
SRC += bsp/stm32f2xx/sw/SysLib/src/stm32f2xx_flash.c
SRC += bsp/stm32f2xx/sw/SysLib/src/stm32f2xx_spi.c
SRC += bsp/stm32f2xx/sw/SysLib/src/misc.c

# modulares Gateway
SRC += ModGateCom/sw/ModGateRS485.c
SRC += ModGateCom/sw/PiBridgeSlave.c
SRC += ModGateCom/sw/stm32fxxx/PiBridgeSlave_ll.c

# Utilities
SRC += utilities/sw/kbUtilities.c
SRC += utilities/sw/kbAlloc.c
SRC += common/sw/platformError.c

# main and glue code
SRC += src/main.c
SRC += src/application.c
SRC += src/eepromDefaultValue.c
SRC += src/PiSlaveApplication.c
SRC += src/PiDio.c
SRC += src/FW_Desc.c


# Define all object files from SRC Files
OBJ = $(SRC:%.c=%.o)
DEP = $(SRC:%.c=%.d)

# all
all: $(MAIN_OUT_ELF) $(MAIN_OUT_BIN)


# main
$(MAIN_OUT_ELF): $(OBJ) $(BUILD)
	$(CC) $(LDFLAGS) $(OBJ) $(LIBS) --output $@

$(DEP): %.d: %.c makefile
	$(CC) -MM -MT $*.o -MT $*.d $(CFLAGS) $< > $@

include $(DEP)

$(OBJ): %.o : %.c makefile
	$(CC) -c -O$(OPTLEVEL) $(CFLAGS) $< -o $@


size:  $(MAIN_OUT_ELF)
	$(SIZE) --totals $(OBJ)
	@echo Total with linked libs
	$(SIZE) $(TARGET).elf

# bei CP nur die .text Section nehmen, da sonst das Binary riesig wird (wg. SRAM)
#$(CP) $(CPFLAGS) --only-section .text .rodata .storePara $< $@
$(MAIN_OUT_BIN): $(MAIN_OUT_ELF)
	$(CP) $(CPFLAGS) $< $@

# $(OD) $(ODFLAGS) $(MAIN_OUT_ELF) > $(TARGET).lss

clean:
	 $(DEL) $(MAIN_OUT_ELF)
	 $(DEL) $(MAIN_OUT_BIN)
	 $(DEL) $(DEP)
	 $(DEL) $(OBJ)
	 $(DEL) *.lss
	 $(DEL) *.map

