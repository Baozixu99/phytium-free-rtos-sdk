# 指定工程项目根目录为当前（只能指定一个目录）
export PROJECT_DIR 	?= $(shell pwd)
# 用户添加的源文件夹和头文件夹（可以指定多个）
export USR_SRC_DIR  ?= $(PROJECT_DIR) \
					   $(PROJECT_DIR)/src
export USR_INC_DIR  ?= $(PROJECT_DIR) \
					   $(PROJECT_DIR)/inc

# 用户定义的编译目标文件上传路径 
ifeq ($(OS),Windows_NT)
USR_BOOT_DIR		?= /d/tftboot
else
USR_BOOT_DIR		?= /mnt/d/tftboot
endif


# 设置启动镜像名
BOOT_IMG_NAME      ?= freertos

# 指定编译freertos项目使用的makefile
include $(FREERTOS_SDK_ROOT)/make/build_freertos.mk

# 完成编译
boot:
	make
#	@echo tar $(CONFIG_TARGET_NAME)
#	@echo img $(BOOT_IMG_NAME)
	cp ./$(CONFIG_TARGET_NAME).elf $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).elf


