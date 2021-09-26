LD_DIR := $(FREERTOS_SDK_ROOT)/make/ld


ifdef CONFIG_AARCH64_RAM_LD
LDSNAME ?= $(LD_DIR)/aarch64_ram.ld
endif

ifdef CONFIG_AARCH32_RAM_LD
LDSNAME ?= $(LD_DIR)/aarch32_ram.ld
endif

ifdef CONFIG_USER_DEFINED_LD
LDSNAME ?= $(EXT_LDSNAME)
endif