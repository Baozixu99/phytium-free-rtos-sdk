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

APP ?= $(CONFIG_TARGET_NAME)

QUIET ?=@
# OPT_LEVEL ?= 0
OPT_LEVEL ?=  $(CONFIG_GCC_OPTIMIZE_LEVEL)

INC_DIR ?=	# include 目录
SRC_DIR ?=  # 源文件目录
EXCL_SRC ?= # 在源文件目录中不编译
OUTPUT_DIR ?= #输出目标文件

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
PROGRESS = @echo $<...
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	ifdef CONFIG_USE_EXT_COMPILER
		CROSS_PATH  := $(CONFIG_EXT_COMPILER_PREFIX)
	else
		CROSS_PATH  := $(AARCH64_CROSS_PATH)
	endif
CROSS_COMPILE := $(CROSS_PATH)/bin/aarch64-none-elf-
endif

ifdef CONFIG_TARGET_ARMV8_AARCH32
	ifdef CONFIG_USE_EXT_COMPILER
		CROSS_PATH  := $(CONFIG_EXT_COMPILER_PREFIX)
	else
		CROSS_PATH  := $(AARCH32_CROSS_PATH)
	endif
CROSS_COMPILE := $(CROSS_PATH)/bin/arm-none-eabi-
endif

CC = $(CROSS_COMPILE)gcc
OC = $(CROSS_COMPILE)objcopy
OD = $(CROSS_COMPILE)objdump
LD = $(CROSS_COMPILE)ld

$(shell if [ ! -e $(OUTPUT_DIR) ];then mkdir -p $(OUTPUT_DIR); fi)

INC_DIR		:= $(INC_DIR) $(USR_INC_DIR)
INCLUDES	:= $(patsubst %, -I %, $(INC_DIR))
# 使用ld.o作为后缀，区分临时用的linkscript
PROJ_LD		:= $(OUTPUT_DIR)/linkscript.ld.o
TEMP_LD		:= $(OUTPUT_DIR)/linkscript.ld.c
			
define EOL =

endef

RM_FILES = $(foreach file,$(1),rm -f $(file)$(EOL))
RM_DIRS = $(foreach dir,$(1),rm -rf $(dir)$(EOL))

DEPEND_FLAGS = -MD -MF $@.d

CPPFLAGS = $(DEFINES) $(INCLUDES) $(DEPEND_FLAGS) $(CPPFLAGS_EXTRA)
CFLAGS =  $(DEBUG_FLAGS) -DGUEST  -ffreestanding  -Wextra -Wno-implicit-fallthrough -g -O$(OPT_LEVEL) 
ASFLAGS = $(CFLAGS)

#mkdir 创建输出文件目录
SRC_DIR   := $(SRC_DIR) $(USR_SRC_DIR)
X_OUTPUT_DIRS :=	$(patsubst %, $(OUTPUT_DIR)/%, $(SRC_DIR))

APP_C_SRC	:=	$(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))
APP_S_SRC	:=	$(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.S))

OBJ_FILES	:=	$(patsubst %, $(OUTPUT_DIR)/%, $(APP_S_SRC:.S=.o)) \
				$(patsubst %, $(OUTPUT_DIR)/%, $(APP_C_SRC:.c=.o))
				
EXCL_OBJS   ?= $(patsubst %, $(OUTPUT_DIR)/%, $(EXCL_SRC:.c=.o) $(EXCL_SRC:.S=.o))
OBJ_FILES   := $(filter-out $(EXCL_OBJS), $(OBJ_FILES))

DEP_FILES := $(OBJ_FILES:%=%.d)

LIBC ?= 
LIBPATH ?= 
CC_VERSION = 10.3.1
ifdef CONFIG_TARGET_ARMV8_AARCH32
#  support float and div, turn on by default for aarch64 -mfpu=crypto-neon-fp-armv8。
	ASFLAGS     := $(ASFLAGS) -mfpu=crypto-neon-fp-armv8  -ftree-vectorize -mfloat-abi=softfp -ffast-math -fsingle-precision-constant -march=$(ARCH)
	CFLAGS      := $(CFLAGS) -mfpu=crypto-neon-fp-armv8 -mfloat-abi=softfp -march=$(ARCH)
endif


TEST_DATA ?= 
# 使用外链的NewLibc
ifdef CONFIG_USE_NEW_LIBC
	ifdef CONFIG_TARGET_ARMV8_AARCH32
		LIBPATH := $(CROSS_PATH)/newlib/arm-none-eabi/newlib
		INC_DIR	:= $(INC_DIR)  $(CROSS_PATH)/newlib/newlib/libc/include
		OBJ_FILES += $(CROSS_PATH)/newlib/arm-none-eabi/newlib/libc.a  \
					 $(CROSS_PATH)/newlib/arm-none-eabi/newlib/libm.a  \
					 $(CROSS_PATH)/lib/gcc/arm-none-eabi/$(CC_VERSION)/libgcc.a	
	endif

	ifdef CONFIG_TARGET_ARMV8_AARCH64
		LIBPATH := $(CROSS_PATH)/newlib/aarch64-none-elf/newlib
		INC_DIR	:= $(INC_DIR)  $(CROSS_PATH)/newlib/newlib/libc/include
		OBJ_FILES += $(CROSS_PATH)/newlib/aarch64-none-elf/newlib/libc.a  \
					 $(CROSS_PATH)/newlib/aarch64-none-elf/newlib/libm.a  \
					 $(CROSS_PATH)/lib/gcc/aarch64-none-elf/$(CC_VERSION)/libgcc.a
	endif

	LDFLAGS += -nostdlib -nostartfiles
	LDFLAGS += -lgcc  -L $(LIBPATH)	
endif

OBJ_FILES += $(LIB_OBJ_FILES)

# 不使用Libc库
ifdef CONFIG_USE_NOSTD_LIBC
	LDFLAGS += -nostdlib -nostartfiles
endif


.phony: all linkscript clean rebuild
all: $(APP)

linkscript:
# 如果用户指定了linkscript，跳过使用用户指定的linkscript
ifndef CONFIG_USER_DEFINED_LD
	@cp -f $(LDSNAME) $(TEMP_LD)
	$(QUIET) $(CC) -P -E $(TEMP_LD) -o $(PROJ_LD) -I $(KCONFIG_DIR) $(INCLUDES) 
	@rm -f $(TEMP_LD)
else
	PROJ_LD := $(EXT_LDSNAME) 
endif

$(APP): $(OBJ_FILES) linkscript
	@echo "  LINK	$@.elf"
	@echo "  DUMP	$@.map"
	$(QUIET) $(CC) $(TARGET_ARCH) $(LDFLAGS) -T $(PROJ_LD) --output $@.elf -Wl,-Map=$@.map $(OBJ_FILES) -lm
ifdef CONFIG_OUTPUT_BINARY
	@echo "  COPY	$@.bin"
	$(QUIET) $(OC) -v -O binary $@.elf $@.bin
endif
	$(QUIET) $(OD) -D $@.elf > $@.asm
	$(QUIET) $(OD) -S $@.elf > $@.dis

ifdef CONFIG_TARGET_F2000_4
	@echo "  FT2000_4_BOARD COMPLIE"
endif

ifdef CONFIG_TARGET_D2000
	@echo "  D2000_BOARD COMPLIE"
endif

ifdef CONFIG_TARGET_E2000S
	@echo "  E2000S_BOARD COMPLIE"
endif

ifdef CONFIG_TARGET_E2000D
	@echo "  E2000D_BOARD COMPLIE"
endif

ifdef CONFIG_TARGET_E2000Q
	@echo "  E2000Q_BOARD COMPLIE"
endif

ifdef CONFIG_TARGET_ARMV8_AARCH32
	@echo "  AARCH32 CODE"
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	@echo "  AARCH64 CODE"
endif

	@echo "  SUCCESS !!!"



$(X_OUTPUT_DIRS):
	@mkdir -p $@

$(OUTPUT_DIR)/%.o : %.S | $(X_OUTPUT_DIRS)
	@echo "  AS    $@"
	$(QUIET) $(CC) -c $(TARGET_ARCH) $(CPPFLAGS) $(ASFLAGS) -o $@ $<

$(OUTPUT_DIR)/%.o : %.c | $(X_OUTPUT_DIRS)
	@echo "  CC    $@"
	$(QUIET) $(CC) -c $(TARGET_ARCH) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	$(call RM_DIRS,$(OUTPUT_DIR))
	$(call RM_FILES,*.elf)
	$(call RM_FILES,*.bin)
	$(call RM_FILES,*.dis)
	$(call RM_FILES,*.map)
	$(call RM_FILES,*.tar.gz)
	$(call RM_FILES,*.asm)
	$(call RM_FILES,*.out)

# Make sure everything is rebuilt if this makefile is changed
$(OBJ_FILES) $(APP): makefile

-include $(DEP_FILES)
