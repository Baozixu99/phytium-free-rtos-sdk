SDMMC_OS_DIR := $(FREERTOS_SDK_ROOT)/third-party/sdmmc-1.0
SDMMC_BM_DIR := $(STANDALONE_DIR)/third-party/sdmmc

include $(SDMMC_BM_DIR)/sdmmc.mk

ifdef CONFIG_USE_FREERTOS

INC_DIR += $(SDMMC_OS_DIR)/osal\
		   $(SDMMC_OS_DIR)/port	
SRC_DIR += $(SDMMC_OS_DIR)/osal\
		   $(SDMMC_OS_DIR)/port

	ifdef CONFIG_SDMMC_USE_FSDIO

	INC_DIR += $(SDMMC_OS_DIR)/port/fsdio
	SRC_DIR += $(SDMMC_OS_DIR)/port/fsdio

	endif #CONFIG_SDMMC_USE_FSDIO

endif