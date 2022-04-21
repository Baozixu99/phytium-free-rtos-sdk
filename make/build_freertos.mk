# 指定工程项目根目录为当前（只能指定一个目录）
export PROJECT_DIR 	?= $(shell pwd)
# 用户添加的源文件夹和头文件夹（可以指定多个）
export USR_SRC_DIR  ?= $(PROJECT_DIR) \
					   $(PROJECT_DIR)/src
export USR_INC_DIR  ?= $(PROJECT_DIR) \
					   $(PROJECT_DIR)/inc

export EXCL_SRC ?= # 在源文件目录中不编译
export OUTPUT_DIR  ?= $(PROJECT_DIR)/build#编译信息存放目录
export KCONFIG_DIR ?= $(PROJECT_DIR)# sdkconfig 目录
export EG_CONFIGS_DIR ?= $(PROJECT_DIR)/configs#不同平台备份配置信息

export STANDALONE_DIR?=$(FREERTOS_STANDALONE)
include $(FREERTOS_SDK_ROOT)/make/preconfig.mk
ifdef USER_DEFINE_MK#用户自定义makefile 脚本
include $(USER_DEFINE_MK)
endif
include $(FREERTOS_SDK_ROOT)/make/standalone_dependence.mk# standalone sdk 依赖
include $(FREERTOS_SDK_ROOT)/freertos.mk#freertos 相关依赖
include $(FREERTOS_SDK_ROOT)/make/complier.mk#编译功能
include $(FREERTOS_SDK_ROOT)/make/buildinfo.mk#目录调试相关打印
include $(FREERTOS_SDK_ROOT)/make/packsource.mk#
include $(FREERTOS_SDK_ROOT)/make/menuconfig.mk#kconfig 相关功能
include $(FREERTOS_SDK_ROOT)/make/default_load.mk#kconfig 相关的默认加载功能
