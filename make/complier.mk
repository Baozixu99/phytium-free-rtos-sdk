# Copyright (C) ARM Limited, 2014-2016. All rights reserved.
#
# This example is intended to be built with Linaro bare-metal GCC
#
# Environment variables for build options that the user might wish to change
#
# Variable     Example Value
# ----------   -------------
# APP          myapp.axf
# QUIET        @ for terse output, or leave blank for detailed output
# OPT_LEVEL    0, 1, 2 or 3
# DEFINES      -D MYDEFINE

#include $(FREERTOS_SDK_ROOT)/make/preconfig.mk

APP ?= $(CONFIG_TARGET_NAME)

QUIET ?=@
OPT_LEVEL ?= 0

# Other switches the user should not normally need to change:
ifdef CONFIG_TARGET_ARMV8_AARCH32 
	ARCH = armv8-a
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	ARCH = armv8-a
endif

ifdef CONFIG_TARGET_ARMV7
	ARCH = armv7-a
endif


DEBUG_FLAGS = -g 

CPPFLAGS_EXTRA :=

ifndef FREERTOS_SDK_ROOT
$(error error, please set FREERTOS_SDK_ROOT first!!)
endif

ifeq ($(QUIET),@)
PROGRESS = @echo Compiling $<...
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	ifdef CONFIG_USE_EXT_COMPILER
		CROSS_PATH  := $(CONFIG_EXT_COMPILER_PREFIX)
	else
		CROSS_PATH  := $(FREERTOS_AARCH64_CROSS_PATH)
	endif
CROSS_COMPILE := $(CROSS_PATH)/bin/aarch64-none-elf-
endif

ifdef CONFIG_TARGET_ARMV8_AARCH32
	ifdef CONFIG_USE_EXT_COMPILER
		CROSS_PATH  := $(CONFIG_EXT_COMPILER_PREFIX)
	else
		CROSS_PATH  := $(FREERTOS_AARCH32_CROSS_PATH)
	endif
CROSS_COMPILE := $(CROSS_PATH)/bin/arm-none-eabi-
endif

CC = $(CROSS_COMPILE)gcc
OC = $(CROSS_COMPILE)objcopy
OD = $(CROSS_COMPILE)objdump
LD = $(CROSS_COMPILE)ld

# Commit hash from git
COMMIT=$(shell git rev-parse --short HEAD)
PACK_DATA=$(shell date +'%m%d_%H%M')
INC_DIR ?=	# include 目录
SRC_DIR ?=  # 源文件目录
EXCL_SRC ?= # 在源文件目录中不编译
#OUTPUT_DIR ?= build # 输出目录
$(shell if [ ! -e $(OUTPUT_DIR) ];then mkdir -p $(OUTPUT_DIR); fi)

include $(STANDALONE_DIR)/standalone.mk
include $(STANDALONE_DIR)/lib/lib.mk
include $(FREERTOS_SDK_ROOT)/make/ld.mk
include $(FREERTOS_SDK_ROOT)/third-party/third-party.mk


INC_DIR		:= $(INC_DIR) $(USR_INC_DIR)
INCLUDES	:= $(patsubst %, -I %, $(INC_DIR))
			
define EOL =

endef

ifeq ($(OS),Windows_NT)
RM_FILES = $(foreach file,$(1),if exist $(file) del /q $(file)$(EOL))
RM_D
IRS = $(foreach dir,$(1),if exist $(dir) rmdir /s /q $(dir)$(EOL))
else
RM_FILES = $(foreach file,$(1),rm -f $(file)$(EOL))
RM_DIRS = $(foreach dir,$(1),rm -rf $(dir)$(EOL))
endif

DEPEND_FLAGS = -MD -MF $@.d

CPPFLAGS = $(DEFINES) $(INCLUDES) $(DEPEND_FLAGS) $(CPPFLAGS_EXTRA)
CFLAGS =  $(DEBUG_FLAGS) -DGUEST  -ffreestanding  -Wextra -g -O$(OPT_LEVEL) 
ASFLAGS = $(CFLAGS)

LIBC ?= 
LIBPATH ?= 
ifdef CONFIG_TARGET_ARMV8_AARCH32
	LIBPATH 	+= $(CROSS_PATH)/arm-none-eabi/lib/thumb/v7/nofp
# support float and div, turn on by default for aarch64
	ASFLAGS     := $(ASFLAGS) -mfpu=vfpv3-fp16  -ftree-vectorize -mfloat-abi=softfp -ffast-math -fsingle-precision-constant -march=$(ARCH)
	CFLAGS      := $(CFLAGS) -mfpu=vfpv3-fp16 -mfloat-abi=softfp -march=$(ARCH)
endif

LDFLAGS = -T$(LDSNAME) -Wl,--build-id=none 
ifdef CONFIG_USE_LIBC 
ifdef CONFIG_TARGET_ARMV8_AARCH32
LDFLAGS += -lgcc  -L $(LIBPATH)
endif
else
LDFLAGS += -nostdlib -nostartfiles
endif

TARGET_ARCH = -march=$(ARCH)

#mkdir 创建输出文件目录
SRC_DIR   := $(SRC_DIR) $(USR_SRC_DIR)
X_OUTPUT_DIRS :=	$(patsubst %, $(OUTPUT_DIR)/%, $(SRC_DIR))

APP_C_SRC	:=	$(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))
APP_S_SRC	:=	$(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.S))

OBJ_FILES	:=	$(patsubst %, $(OUTPUT_DIR)/%, $(APP_S_SRC:.S=.o)) \
				$(patsubst %, $(OUTPUT_DIR)/%, $(APP_C_SRC:.c=.o))
				
EXCL_OBJS   ?= $(patsubst %, $(OUTPUT_DIR)/%, $(EXCL_SRC:.c=.o))
OBJ_FILES   := $(filter-out $(EXCL_OBJS), $(OBJ_FILES))
OBJ_FILES   += $(LIBC)

DEP_FILES := $(OBJ_FILES:%=%.d)

ifdef CONFIG_DUMMY_COMPILE
	DO_ECHO = @echo
else
	DO_ECHO = 
endif

.phony: all clean rebuild
all: $(APP)

$(APP): $(OBJ_FILES) $(LDSNAME)
	@echo Linking $@.elf
	@echo Dumping $@.map

	$(DO_ECHO) $(QUIET) $(CC) $(TARGET_ARCH) $(LDFLAGS) --output $@.elf -Wl,-Map=$@.map $(OBJ_FILES) -lm
	@echo Objcopying $@.bin
	$(DO_ECHO) $(QUIET) $(OC) -v -O binary $@.elf $@.bin
	$(DO_ECHO) $(QUIET) $(OD) -D $@.elf > $@.dis
	@echo Done.

clean:
	$(call RM_DIRS,$(OUTPUT_DIR))
	$(call RM_FILES,*.elf)
	$(call RM_FILES,*.bin)
	$(call RM_FILES,*.dis)
	$(call RM_FILES,*.map)
	$(call RM_FILES,*.tar.gz)

$(X_OUTPUT_DIRS):
	@mkdir -p $@

$(OUTPUT_DIR)/%.o : %.S | $(X_OUTPUT_DIRS)
	$(PROGRESS)
	$(QUIET) $(CC) -c $(TARGET_ARCH) $(CPPFLAGS) $(ASFLAGS) -o $@ $<

$(OUTPUT_DIR)/%.o : %.c | $(X_OUTPUT_DIRS)
	$(PROGRESS)
	$(QUIET) $(CC) -c $(TARGET_ARCH) $(CPPFLAGS) $(CFLAGS) -o $@ $<


# Make sure everything is rebuilt if this makefile is changed
$(OBJ_FILES) $(APP): makefile

-include $(DEP_FILES)
