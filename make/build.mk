INC_DIR ?=	# include 目录
SRC_DIR ?=  # 源文件目录
EXCL_SRC ?= # 在源文件目录中不编译

include $(SDK_ROOT)/make/preconfig.mk
include $(FREERTOS_SDK_ROOT)/third-party/third-party.mk
include $(SDK_ROOT)/make/complier.mk

include $(SDK_ROOT)/make/buildinfo.mk
include $(SDK_ROOT)/make/packsource.mk
include $(SDK_ROOT)/make/menuconfig.mk