
ifdef CONFIG_USE_FREERTOS
ABSOLUTE_CFILES += $(wildcard osa/*.c)
endif

FSL_SDMMC_BM_DIR := $(abspath $(SDK_DIR)/third-party/fsl_sdmmc)

ABSOLUTE_CFILES += $(abspath $(wildcard $(FSL_SDMMC_BM_DIR)/common/*.c)) \
				   $(abspath $(wildcard $(FSL_SDMMC_BM_DIR)/host/*.c)) \
				   $(abspath $(wildcard $(FSL_SDMMC_BM_DIR)/partition/*.c))

ifdef CONFIG_FSL_SDMMC_USE_FSDIF
ABSOLUTE_CFILES += $(abspath $(wildcard $(FSL_SDMMC_BM_DIR)/host/fsdif/*.c))
endif

ifdef CONFIG_FSL_SDMMC_USE_FSDIF_V2
ABSOLUTE_CFILES += $(abspath $(wildcard $(FSL_SDMMC_BM_DIR)/host/fsdif_v2_0/*.c))
endif

ifdef CONFIG_FSL_SDMMC_USE_FSDMMC
ABSOLUTE_CFILES += $(abspath $(wildcard $(FSL_SDMMC_BM_DIR)/host/fsdmmc/*.c))
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SD
ABSOLUTE_CFILES += $(abspath $(wildcard $(FSL_SDMMC_BM_DIR)/sd/*.c))
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_MMC
ABSOLUTE_CFILES += $(abspath $(wildcard $(FSL_SDMMC_BM_DIR)/mmc/*.c))
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SDIO
ABSOLUTE_CFILES += $(abspath $(wildcard $(FSL_SDMMC_BM_DIR)/sdio/*.c))
endif

ifdef CONFIG_FSL_SDMMC_ENABLE_SD_SPI
ABSOLUTE_CFILES += $(abspath $(wildcard $(FSL_SDMMC_BM_DIR)/sdspi/*.c))
endif