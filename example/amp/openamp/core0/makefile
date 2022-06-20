export PROJECT_DIR ?= $(shell pwd)

export USR_SRC_DIR = $(PROJECT_DIR)
export USR_INC_DIR = $(PROJECT_DIR) 

# 用户定义的编译目标文件上传路径 
ifeq ($(OS),Windows_NT)
	USR_BOOT_DIR		?= $(subst \,/, $(PHYTIUM_DEV_PATH))/tftp
else
	USR_BOOT_DIR		?= /mnt/d/tftboot
endif

# 设置启动镜像名
USER_BOOT_IMAGE    ?= openamp_core0


# 设置启动镜像名
BOOT_IMG_NAME      ?= openamp_core0

# 指定编译freertos项目使用的makefile
include $(FREERTOS_SDK_ROOT)/make/build_freertos.mk

# 完成编译
boot:
	make -j
	cp ./$(CONFIG_TARGET_NAME).elf $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).elf

scp:
	scp ./openamp_core0.elf root@192.168.2.198:/lib/firmware

