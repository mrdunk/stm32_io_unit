TARGET:=Demo
TOOLCHAIN_PATH:=~/sat/bin
TOOLCHAIN_PREFIX:=arm-none-eabi
OPTLVL:=0 # Optimization level, can be [0, 1, 2, 3, s].

PROJECT_NAME:=$(notdir $(lastword $(CURDIR)))
#TOP:=$(shell readlink -f "../..")
TOP:=$(shell readlink -f "./")
DISCOVERY:=$(TOP)/Utilities
STMLIB:=$(TOP)/Libraries
STD_PERIPH:=$(STMLIB)/STM32F10x_StdPeriph_Driver
CORE:=$(STMLIB)/CMSIS/CM3/CoreSupport
DEVICE:=$(STMLIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x
STARTUP:=$(STMLIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x/startup/gcc_ride7
LINKER_SCRIPT:="$(CURDIR)/stm32.ld"

INCLUDE=-I"$(CURDIR)"
INCLUDE+=-I"$(CURDIR)/inc"
INCLUDE+=-I"$(CORE)"
INCLUDE+=-I"$(DEVICE)"
INCLUDE+=-I"$(STD_PERIPH)/inc"
INCLUDE+=-I"$(DISCOVERY)"
#INCLUDE+=-I"/home/duncan/Working/EmbeddedArm/gcc-arm-none-eabi-4_8-2014q1/arm-none-eabi/include/"
#INCLUDE+=-I"/home/duncan/Working/EmbeddedArm/gcc-arm-none-eabi-4_8-2014q1/lib/gcc/arm-none-eabi/4.8.3/include/"

# vpath is used so object files are written to the current directory instead
# of the same directory as their source files
vpath %.c $(CURDIR)/src $(STD_PERIPH)/src $(DEVICE) $(CORE) $(DISCOVERY) 
vpath %.s $(STARTUP)

ASRC=startup_stm32f10x_md_vl.s

# Project Source Files
SRC=main.c
SRC+=stm32f10x_it.c

#CMSIS
SRC+=system_stm32f10x.c

# Standard Peripheral Source Files
SRC+=stm32f10x_pwr.c
SRC+=stm32f10x_rcc.c
SRC+=misc.c
SRC+=stm32f10x_exti.c
SRC+=stm32f10x_gpio.c
SRC+=stm32f10x_tim.c
SRC+=ds18s20.c
SRC+=stm32_io.c
SRC+=stm32f100_io_pins.c

# _sbrk used by malloc.
SRC+=syscalls.c

CDEFS=-DSTM32F10X_MD_VL
CDEFS+=-DUSE_STDPERIPH_DRIVER

MCUFLAGS=-mthumb -march=armv7-m
COMMONFLAGS=-O$(OPTLVL) -Wall -Werror
COMMONFLAGS+= -g
CFLAGS=$(COMMONFLAGS) $(MCUFLAGS) $(INCLUDE) $(CDEFS) \
#        -nodefaultlibs -nostdinc \


#LDLIBS=-lc
LDLIBS=
LDFLAGS=$(COMMONFLAGS) $(MCUFLAGS) -fno-exceptions -ffunction-sections -fdata-sections \
        -nostartfiles -Wl,--gc-sections,-T$(LINKER_SCRIPT) \
#        -nodefaultlibs \
#        -lc -lnosys -specs=nosys.specs

#####
#####

OBJ = $(SRC:%.c=%.o) $(ASRC:%.s=%.o)

CC=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gcc
LD=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gcc
OBJCOPY=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-objcopy
AS=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-as
AR=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-ar
GDB=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gdb


all: $(OBJ)
	$(CC) -o $(TARGET).elf $(LDFLAGS) $(OBJ)	$(LDLIBS)
	$(OBJCOPY) -O ihex   $(TARGET).elf $(TARGET).hex
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin

.PHONY: clean

clean:
	rm -f $(OBJ)
	rm -f $(TARGET).elf
	rm -f $(TARGET).hex
	rm -f $(TARGET).bin
