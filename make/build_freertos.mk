INC_DIR ?=	# include 目录
SRC_DIR ?=  # 源文件目录
EXCL_SRC ?= # 在源文件目录中不编译

export STANDALONE_DIR=$(FREERTOS_STANDALONE)

include $(FREERTOS_SDK_ROOT)/make/preconfig.mk
include $(FREERTOS_SDK_ROOT)/make/complier.mk
include $(FREERTOS_SDK_ROOT)/make/buildinfo.mk
include $(FREERTOS_SDK_ROOT)/make/packsource.mk
include $(FREERTOS_SDK_ROOT)/make/menuconfig.mk
include $(FREERTOS_SDK_ROOT)/make/build_all.mk