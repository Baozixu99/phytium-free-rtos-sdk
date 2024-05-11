
FREERTOS_SDK_DIR ?= $(SDK_DIR)/..

# primary sdk dir, relative to standalone sdk
PRI_SDK_RELATIVE_DIR ?= ../

export PRI_SDK_RELATIVE_DIR

# drivers
include $(FREERTOS_SDK_DIR)/drivers/include.mk

# freertos sdk
include $(FREERTOS_SDK_DIR)/third-party/include.mk
