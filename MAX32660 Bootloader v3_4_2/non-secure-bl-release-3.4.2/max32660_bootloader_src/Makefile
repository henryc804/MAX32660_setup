################################################################################
 # Copyright (C) 2018 Maxim Integrated Products, Inc., All Rights Reserved.
 #
 # Permission is hereby granted, free of charge, to any person obtaining a
 # copy of this software and associated documentation files (the "Software"),
 # to deal in the Software without restriction, including without limitation
 # the rights to use, copy, modify, merge, publish, distribute, sublicense,
 # and/or sell copies of the Software, and to permit persons to whom the
 # Software is furnished to do so, subject to the following conditions:
 #
 # The above copyright notice and this permission notice shall be included
 # in all copies or substantial portions of the Software.
 #
 # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 # OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 # MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 # IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 # OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 # ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 # OTHER DEALINGS IN THE SOFTWARE.
 #
 # Except as contained in this notice, the name of Maxim Integrated
 # Products, Inc. shall not be used except as stated in the Maxim Integrated
 # Products, Inc. Branding Policy.
 #
 # The mere transfer of this software does not imply any licenses
 # of trade secrets, proprietary technology, copyrights, patents,
 # trademarks, maskwork rights, or any other form of intellectual
 # property whatsoever. Maxim Integrated Products, Inc. retains all
 # ownership rights.
 #
 ###############################################################################
TARGET =
BOARD =
PROJECT =
COMPILER = GCC
SRCS =
SUBDIRS =
PROJ_LIBS =
BOOTLOADER_DIR = .

LIBS_DIR=./Max32xxxLibraries
CMSIS_ROOT=$(LIBS_DIR)/CMSIS


# Config file can be choosen in .config, or can be pass as a parameter.
CONFIG_FILE =
PROJ_CFLAGS =

ifeq "${CONFIG_FILE}" ""
-include .config
include ./config/$(PROJECT)
else
include ${CONFIG_FILE}
PROJECT=$(basename $(notdir ${CONFIG_FILE}))
endif

ifeq "$(PROJECT)" ""
$(error Project Name must be specified in .config file or be passed as an external parameter like CONFIG_FILE=path/to/config/file)
endif

MAIN_APP_PATH ?= examples/Hello_World/build/max32660.bin
MAIN_APP_NAME = $(basename $(notdir $(MAIN_APP_PATH)))
MAIN_APP_BUILD_DIR = $(dir $(MAIN_APP_PATH))

TARGET_UC:=$(shell echo $(TARGET) | tr a-z A-Z)
PERIPH_DRIVER_DIR=$(LIBS_DIR)/$(TARGET_UC)PeriphDriver

OBJ-y =
BOARD_DIR=Boards/$(BOARD)
include $(BOARD_DIR)/board.mk

OBJ-y += bootloader.o
ifeq "$(BOOTLOADER_I2C)" "y"
OBJ-y += bootloader_i2c.o
PROJ_CFLAGS += -DBOOTLOADER_I2C
endif

ifeq "$(BOOTLOADER_UART)" "y"
PROJ_CFLAGS += -DBOOTLOADER_UART
ifeq "$(BOOTLOADER_BITBANG_UART)" "y"
OBJ-y += bootloader_sw_uart.o
PROJ_CFLAGS += -DBL_USE_BITBANG_UART
else
OBJ-y += bootloader_hw_uart.o
endif
else ifeq "$(BOOTLOADER_BITBANG_UART)" "y"
$(error BOOTLOADER_BITBANG_UART cannot be enabled when UART is disabled)
endif

ifeq "$(BOOTLOADER_SPI)" "y"
OBJ-y += bootloader_spi.o
PROJ_CFLAGS += -DBOOTLOADER_SPI
PROJ_CFLAGS += -DBL_SPI_PORT="$(BL_SPI_PORT)"
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/spi17y.c
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/spimss.c
endif

OBJ-y += bootloader_helper.c
OBJ-y += crc32.c
OBJ-y += utils.c
OBJ-y += utoa.c
OBJ-y += flash.c
OBJ-y += platform_tick.c

#Include only the peripheral sources that we need to reduce code size
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/spi.c
ifeq "$(BOOTLOADER_UART)" "y"
ifneq "$(BOOTLOADER_BITBANG_UART)" "y"
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/uart.c
else
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/tmr.c
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/icc.c
#Include hw uart if debugging messages are enabled
ifneq (,$(findstring BOOTLOADER_DEBUG_MSG,$(PROJ_CFLAGS)))
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/uart.c
else ifneq (,$(findstring BOOTLOADER_BUILD_INFO,$(PROJ_CFLAGS)))
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/uart.c
endif
endif
endif
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/gpio.c
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/mxc_sys.c
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/flc.c
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/mxc_lock.c
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/mxc_assert.c
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/mxc_delay.c
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/i2c.c

ifeq "$(TARGET)" "MAX32660"
OBJ-y += $(PERIPH_DRIVER_DIR)/Source/mxc_pins.c
endif

SUBDIRS += $(sort $(dir $(OBJ-y)))
VPATH += $(SUBDIRS)
IPATH += $(SUBDIRS)
IPATH += $(PERIPH_DRIVER_DIR)/Include

ifeq ($(DEBUG_PORT), $(CONSOLE_UART))
$(error DEBUG_PORT and CONSOLE_UART can not be equal. Please update them or disable DEBUG_PORT from config)
endif

ifdef DEBUG_PORT
PROJ_CFLAGS += -DDEBUG_PORT=$(DEBUG_PORT) -DDEBUG_UART_BAUD=$(DEBUG_UART_BAUD)
endif

PROJ_CFLAGS += -DCONSOLE_UART=$(CONSOLE_UART) -DCONSOLE_BAUD=$(CONSOLE_BAUD)
PROJ_CFLAGS += -DBOOTLOADER
PROJ_CFLAGS += -D$(TARGET)
PROJ_CFLAGS += -DTARGET_STR=\"$(TARGET)\"
ifneq "$(TARGET_REV)" ""
PROJ_CFLAGS += -D$(TARGET_REV)
endif
PROJ_CFLAGS += -D$(BOARD)
IPATH += $(BOARD_DIR)/../Include
IPATH += $(BOARD_DIR)/Include/

TARGET_UC:=$(shell echo $(TARGET) | tr a-z A-Z)
TARGET_LC:=$(shell echo $(TARGET) | tr A-Z a-z)

ifeq "$(MAKECMDGOALS)" "release_firmware"
OBJ-y += ${MAIN_APP_BUILD_DIR}/main_app.o
OBJ-y += ${MAIN_APP_BUILD_DIR}/boot_mem.o

LINKERFILE=$(BOOTLOADER_DIR)/$(COMPILER)/bootloader_with_fw_$(TARGET).ld
#Check LINKERFILE actually exists
ifeq ("$(wildcard $(LINKERFILE))","")
$(error Bootloader does not currently support $(TARGET). Comment out BOOTLOADER=y or select a different Project in .config)
endif


else ifeq "$(MAKECMDGOALS)" "firmware"
OBJ-y += ${MAIN_APP_BUILD_DIR}/main_app.o
OBJ-y += ${MAIN_APP_BUILD_DIR}/boot_mem.o

LINKERFILE=$(BOOTLOADER_DIR)/$(COMPILER)/bootloader_with_fw_$(TARGET).ld
#Check LINKERFILE actually exists
ifeq ("$(wildcard $(LINKERFILE))","")
$(error Bootloader does not currently support $(TARGET). Comment out BOOTLOADER=y or select a different Project in .config)
endif

else
ifeq "$(LINKER_SCRIPT)" "RELEASE"
LINKERFILE=$(BOOTLOADER_DIR)/$(COMPILER)/bootloader_$(TARGET).ld
else ifeq "$(LINKER_SCRIPT)" "DEBUG"
LINKERFILE=$(BOOTLOADER_DIR)/$(COMPILER)/bootloader_$(TARGET)_debug.ld
else
$(error Invalid Linker Script option: ${LINKER_SCRIPT} selected in config file)
endif
#Check LINKERFILE actually exists
ifeq ("$(wildcard $(LINKERFILE))","")
$(error Bootloader does not currently support $(TARGET). Comment out BOOTLOADER=y or select a different Project in .config)
endif
endif

ifeq "$(TARGET)" "MAX32660"
STARTUPFILE=$(BOOTLOADER_DIR)/startup_bootloader_max32660.S
else
STARTUPFILE=$(BOOTLOADER_DIR)/startup_bootloader.S
endif
SRCS += $(OBJ-y)

# Disable DEBUG
DEBUG=0
MXC_OPTIMIZE_CFLAGS = -Os -flto

## Enable Debug
#DEBUG=1
#MXC_OPTIMIZE_CFLAGS = -O0
include $(CMSIS_ROOT)/Device/Maxim/$(TARGET_UC)/Source/$(COMPILER)/$(TARGET_LC).mk

ifeq "$(TOOL_DIR)" ""
PREFIX=arm-none-eabi
else
PREFIX=$(TOOL_DIR)/arm-none-eabi
endif

$(info $(cyan)* GOALS: $(MAKECMDGOALS))
$(info $(cyan)* PROJ_CFLAGS: $(PROJ_CFLAGS)$(reset))
$(info $(cyan)* IPATH: $(IPATH)$(reset))
$(info $(cyan)* BOARD: $(BOARD), TARGET: $(TARGET), PROJECT: $(PROJECT)$(reset))
$(info $(blue)* LINKERFILE: $(LINKERFILE)$(reset))
$(info $(blue)* STARTUPFILE: $(STARTUPFILE)$(reset))
$(info $(blue)* OBJ-y: $(OBJ-y)$(reset))
$(info $(blue)* GOALS: $(MAKECMDGOALS)$(reset))

BUILD_INFO:=$(shell perl ./scripts/pl/build_info.pl 'build_info.h')
$(info Adding build info $(BUILD_INFO))

.PHONY: firmware

firmware: all
release_firmware: all release rename

## Combine bootloader, boot_mem and main_app in a single image
%main_app.o: $(MAIN_APP_BUILD_DIR)/$(MAIN_APP_NAME).bin
	${PREFIX}-ld -r -b binary $< -o $@
	${OBJCOPY} --rename-section .data=.main_app $@
	cp ${BUILD_DIR}/${PROJECT}.elf $(MAIN_APP_BUILD_DIR)/Bootloader_$(MAIN_APP_NAME).elf

%boot_mem.o: $(MAIN_APP_BUILD_DIR)/$(MAIN_APP_NAME)_bootmem.bin
	${PREFIX}-ld -r -b binary $< -o $@
	${OBJCOPY} --rename-section .data=.bootmem $@

all: rename
.PHONY: rename
rename: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).bin
	cp $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/output.elf
	cp $(BUILD_DIR)/$(PROJECT).bin $(BUILD_DIR)/output.bin

all: output_info

.NOTPARALLEL:output_info
.PHONY: output_info
output_info:
	@ls -la ${BUILD_DIR}/${PROJECT}.elf;
	@cp ${BUILD_DIR}/${PROJECT}.elf ${BUILD_DIR}/output.elf
	@${PREFIX}-size --format=SysV ${BUILD_DIR}/${PROJECT}.elf
	@${PREFIX}-size --format=Berkeley ${BUILD_DIR}/${PROJECT}.elf

.PHONY: distclean
distclean: clean
	#$(MAKE) -C ${PERIPH_DRIVER_DIR} clean
