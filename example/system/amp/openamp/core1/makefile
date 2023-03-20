export PROJECT_DIR ?= $(shell pwd)

export USR_SRC_DIR = $(PROJECT_DIR) \
				$(PROJECT_DIR)/../src
export USR_INC_DIR = $(PROJECT_DIR) \
				$(PROJECT_DIR)/../src

# 用户定义的编译目标文件上传路径 
ifeq ($(OS),Windows_NT)
	USR_BOOT_DIR		?= $(subst \,/, $(PHYTIUM_DEV_PATH))/tftp
else
	USR_BOOT_DIR		?= /mnt/d/tftboot
endif

# 设置启动镜像名
USER_BOOT_IMAGE    ?= openamp_core1

# prepare output dir and start make
include $(FREERTOS_SDK_ROOT)/make/build_freertos.mk

.PHONY: rebuild boot config_amp_d2000_aarch64 backup_kconfig
load_amp_d2000_aarch64_config:
	@cp $(PROJECT_DIR)/../configs/d2000_amp_core0_aarch64_defconfig \
	./$(KCONFIG_CONFIG) -f

load_amp_d2000_aarch32_config:
	@cp $(PROJECT_DIR)/../configs/d2000_amp_core0_aarch32_defconfig \
	./$(KCONFIG_CONFIG) -f

rebuild:
	make clean
	make

# 完成编译
boot:
	make -j
	@cp ./$(CONFIG_TARGET_NAME).elf $(USR_BOOT_DIR)/$(USER_BOOT_IMAGE).elf
	@ls $(USR_BOOT_DIR)/$(USER_BOOT_IMAGE).elf -l

config_amp_d2000_aarch64: load_amp_d2000_aarch64_config genconfig clean
config_amp_d2000_aarch32: load_amp_d2000_aarch32_config genconfig clean




