# 指定工程项目根目录为当前（只能指定一个目录）
export PROJECT_DIR 	?= .
# 用户添加的源文件夹和头文件夹（可以指定多个）
export USR_SRC_DIR  ?= . \
					   ./src
export USR_INC_DIR  ?= . \
					   ./inc

# 定义不同平台的默认配置文件
ifeq ($(MAKECMDGOALS),config_ft2004_aarch32)
export DEF_KCONFIG=ft2004_aarch32_deconfig
endif

ifeq ($(MAKECMDGOALS),config_ft2004_aarch64)
export DEF_KCONFIG=ft2004_aarch64_deconfig
endif

# 用户定义的编译目标文件上传路径 
USR_BOOT_DIR		?= /mnt/d/tftboot/

# 指定编译freertos项目使用的makefile
include $(FREERTOS_SDK_ROOT)/make/build_freertos.mk

.PHONY: boot config_ft2004_aarch32

# 完成编译
boot:
	make
	@cp ./$(CONFIG_TARGET_NAME).bin $(USR_BOOT_DIR)

# 加载默认配置，然后生成sdkconfig.h文件，最后清空之前的编译输出
config_ft2004_aarch32: lddefconfig genconfig clean
config_ft2004_aarch64: lddefconfig genconfig clean
