# 指定工程项目根目录为当前（只能指定一个目录）
export PROJECT_DIR 	?= .
# 用户添加的源文件夹和头文件夹（可以指定多个）
export USR_SRC_DIR  ?= . \
					   ./src
export USR_INC_DIR  ?= . \
					   ./inc

# 用户定义的编译目标文件上传路径 
USR_BOOT_DIR		?= /mnt/d/tftboot/

# 指定编译freertos项目使用的makefile
include $(FREERTOS_SDK_ROOT)/make/build_freertos.mk

# 完成编译
boot:
	make
	@cp ./$(CONFIG_TARGET_NAME).bin $(USR_BOOT_DIR)