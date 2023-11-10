ifdef CONFIG_USE_LIBMETAL


ifdef CONFIG_USE_FREERTOS

	CSRCS_RELATIVE_FILES += $(wildcard metal/system/freertos/ft_platform/*.c)

endif

LIBMETAL_DIR = $(SDK_DIR)/third-party/libmetal

ABSOLUTE_CFILES +=$(wildcard $(LIBMETAL_DIR)/metal/*.c) 

ifdef CONFIG_TARGET_ARMV8_AARCH32
	ABSOLUTE_CFILES += $(wildcard $(LIBMETAL_DIR)/metal/system/freertos/*.c)
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	ABSOLUTE_CFILES += $(wildcard $(LIBMETAL_DIR)/metal/system/freertos/*.c)
endif

endif #CONFIG_USE_LIBMETAL