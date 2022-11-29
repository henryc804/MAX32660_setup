###############################################################################
 # Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
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
 # $Date: 2018-01-26 15:56:10 -0600 (Fri, 26 Jan 2018) $
 # $Revision: 33058 $
 #
 ###############################################################################

# This is the name of the build output file
BOOTLOADER_DIR=../..
BOOTLOADER_BUILD_DIR=$(BOOTLOADER_DIR)/build
MSBL_DIR=../../msblGen
ifeq ($(OS),Windows_NT)
MSBL_GEN = $(MSBL_DIR)/msblGen.exe
else
MSBL_GEN = $(MSBL_DIR)/msblGen
endif
BL_CONFIG_FILE=EvKit_V1_BL

ifeq "$(PROJECT)" ""
PROJECT=max32660
endif

# Specify the target processor
ifeq "$(TARGET)" ""
TARGET=MAX32660
endif

# Create Target name variables
TARGET_UC:=$(shell echo $(TARGET) | tr a-z A-Z)
TARGET_LC:=$(shell echo $(TARGET) | tr A-Z a-z)

# Select 'GCC' or 'IAR' compiler
COMPILER=GCC

# Specify the board used
ifeq "$(BOARD)" ""
BOARD=EvKit_V1
endif

# This is the path to the CMSIS root directory
ifeq "$(MAXIM_PATH)" ""
LIBS_DIR=../Libraries
else
LIBS_DIR=/$(subst \,/,$(subst :,,$(MAXIM_PATH))/Firmware/$(TARGET_UC)/Libraries)
endif
CMSIS_ROOT=$(LIBS_DIR)/CMSIS

# Source files for this test (add path to VPATH below)
SRCS  = main.c

# Where to find source files for this test
VPATH=.

# Where to find header files for this test
IPATH = .

# Enable assertion checking for development
PROJ_CFLAGS+=-DMXC_ASSERT_ENABLE

# Specify the target revision to override default
# "A2" in ASCII
# TARGET_REV=0x4132

# Use this variables to specify and alternate tool path
#TOOL_DIR=/opt/gcc-arm-none-eabi-4_8-2013q4/bin

# Use these variables to add project specific tool options
PROJ_CFLAGS+=-DCONSOLE_UART=1
#PROJ_CFLAGS+=-DCONSOLE_UART=0
PROJ_CFLAGS+=-DCONSOLE_BAUD=115200
#PROJ_CFLAGS+=--specs=nano.specs
#PROJ_LDFLAGS+=--specs=nano.specs

# This variable defines the size of array that is added before boot memory
FULL_FLASH_APP_SIGN_SIZE=16
PROJ_CFLAGS+=-DFULL_FLASH_APP_SIGN_SIZE=$(FULL_FLASH_APP_SIGN_SIZE)
PROJ_LDFLAGS+=-Wl,--defsym=FULL_FLASH_APP_SIGN_SIZE=$(FULL_FLASH_APP_SIGN_SIZE)
COMBINER_LDFLAGS+=--defsym=FULL_FLASH_APP_SIGN_SIZE=$(FULL_FLASH_APP_SIGN_SIZE)

# Point this variable to a startup file to override the default file
#STARTUPFILE=start.S

MXC_OPTIMIZE_CFLAGS=-O0
#MXC_OPTIMIZE_CFLAGS=-Os

################################################################################
# Include external library makefiles here

# Include the BSP
BOARD_DIR=$(LIBS_DIR)/Boards/$(BOARD)
include $(BOARD_DIR)/board.mk

# Include the peripheral driver
PERIPH_DRIVER_DIR=$(LIBS_DIR)/$(TARGET_UC)PeriphDriver
include $(PERIPH_DRIVER_DIR)/periphdriver.mk

################################################################################
# Include the rules for building for this target. All other makefiles should be
# included before this one.
include $(CMSIS_ROOT)/Device/Maxim/$(TARGET_UC)/Source/$(COMPILER)/$(TARGET_LC).mk

# Reading LINKER_SCRIPT variable value from used configuration file
LINKER_SCRIPT=$(shell cat ${BOOTLOADER_DIR}/config/${BL_CONFIG_FILE} | grep LINKER_SCRIPT | grep -v "\#" | cut -f2 -d'=')

# Point this variable to a linker file to override the default file
ifeq "$(LINKER_SCRIPT)" "RELEASE"
LINKERFILE=max32660.ld
else ifeq "$(LINKER_SCRIPT)" "DEBUG"
LINKERFILE=max32660_debug.ld
else
$(error Invalid Linker Script option: ${LINKER_SCRIPT} selected in config file)
endif

UNIFY_SCRIPT = $(BOOTLOADER_DIR)/scripts/py/combine_fw_with_bl.py
BOOTLOADER_PATH = $(BOOTLOADER_DIR)/build/${BL_CONFIG_FILE}.bin

ifneq "$(COMBINER_LDFLAGS)" ""
COMBINER_LDARGS="-f=$(COMBINER_LDFLAGS)"
endif

${MSBL_GEN}:
	@echo "********** Building msblGen **********"
	@rm -rf ${MSBL_DIR}/build
	@mkdir -p ${MSBL_DIR}/build
	@cmake -G "Unix Makefiles" -H${MSBL_DIR} -B${MSBL_DIR}/build
	@make -C ${MSBL_DIR}/build

.PHONY: msblGenProducts
msblGenProducts: ${MSBL_GEN} $(BUILD_DIR)/$(PROJECT).bin
	${MSBL_GEN} $(BUILD_DIR)/$(PROJECT).bin $(TARGET) 8192;
	mv $(PROJECT).msbl $(BUILD_DIR)
	mv $(PROJECT)_bootmem.bin $(BUILD_DIR)

.PHONY: bootloader
bootloader: $(BUILD_DIR)/$(PROJECT).bin msblGenProducts
	cd $(BOOTLOADER_DIR) && $(MAKE) CONFIG_FILE=config/${BL_CONFIG_FILE}
#	Unify image
#	cd $(BOOTLOADER_DIR) && $(MAKE) firmware MAIN_APP_PATH=$(abspath $(BUILD_DIR)/${PROJECT}.bin)

unified_bin: $(BUILD_DIR)/$(PROJECT).bin
	${UNIFY_SCRIPT} -b $(BOOTLOADER_PATH) -a $(BUILD_DIR)/$(PROJECT).bin $(COMBINER_LDARGS) -l $(LINKERFILE) -o $(BUILD_DIR)/$(PROJECT)-with-bl.bin

.PHONY: release_bootloader
release_bootloader: bootloader $(BUILD_DIR)/$(PROJECT).bin msblGenProducts
	cd $(BOOTLOADER_DIR) && $(MAKE) release_firmware MAIN_APP_PATH=$(BUILD_DIR)/$(PROJECT).bin

.NOTPARALLEL:output_info
.PHONY: output_info
output_info:
	@${PREFIX}-size --format=SysV ${BUILD_DIR}/${PROJECT}.elf;
	@${PREFIX}-size --format=Berkeley ${BUILD_DIR}/${PROJECT}.elf

.NOTPARALLEL:run_msblgen
.PHONY: run_msblgen
run_msblgen: msblGenProducts $(BUILD_DIR)/$(PROJECT).bin

all: ${BUILD_DIR}/${PROJECT}.elf ${BUILD_DIR}/${PROJECT}.bin output_info run_msblgen unified_bin

clean:
	rm -rf $(BUILD_DIR)
# The rule to clean out all the build products.
distclean: clean
	$(MAKE) -C ${BOOTLOADER_DIR} clean
	$(MAKE) -C ${PERIPH_DRIVER_DIR} clean
