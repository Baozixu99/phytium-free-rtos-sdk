# 定义环境变量用于承接sdk的路径
FREERTOS_SDK_DIR ?= 
SDK_DIR ?= $(FREERTOS_SDK_DIR)/standalone
PROJECT_DIR ?= $(CURDIR)
# 用户自定义的变量与路径
IMAGE_NAME ?= freertos
# 
IMAGE_OUTPUT ?=.

Q ?= @
export Q

EXTERNAL_PATH=$(FREERTOS_SDK_DIR)/tools

export EXTERNAL_PATH
export SDK_DIR

# sdk tools path
# config path
SDK_KCONFIG_DIR ?= $(SDK_DIR)/tools/build/Kconfiglib

# Pyhthon 工具目录
SDK_PYTHON_TOOLS_DIR ?= $(SDK_DIR)/tools/build/py_tools

# 编译中间文件输出的路径
BUILD_OUT_PATH ?= $(PROJECT_DIR)/build

# export BUILD_OUT_PATH
ifneq ($(wildcard $(SDK_DIR)/tools/build_baremetal.mk),)
$(error SDK_DIR/tools/build_baremetal.mk dose not exist)
endif

all: $(IMAGE_NAME).elf

# freertos
include $(FREERTOS_SDK_DIR)/tools/freertos_comonents.mk

# user makefile
include $(SDK_DIR)/board/user/user_make.mk

# linker
include $(SDK_DIR)/tools/build/build.mk

# make menuconfig tools
include $(SDK_DIR)/tools/build/menuconfig/preconfig.mk
include $(SDK_DIR)/tools/build/menuconfig/menuconfig.mk

amp_make:
	$(PYTHON) $(SDK_PYTHON_TOOLS_DIR)/amp_parse_config.py
	cp ./packed_image.elf $(USR_BOOT_DIR)/freertos.elf

amp_make_t:
	$(PYTHON) $(SDK_PYTHON_TOOLS_DIR)/amp_parse_config.py test
	cp ./packed_image.elf $(USR_BOOT_DIR)/freertos.elf

check_standalone_version:
	$(PYTHON) $(FREERTOS_SDK_DIR)/install.py
