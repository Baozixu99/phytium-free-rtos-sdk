ifdef CONFIG_USE_LIBMETAL

LIBMETAL_DIR = $(abspath $(SDK_DIR)/third-party/libmetal)

ifdef CONFIG_USE_FREERTOS

	CSRCS_RELATIVE_FILES += $(wildcard metal/system/freertos/ft_platform/*.c)
	ABSOLUTE_CFILES += $(wildcard $(LIBMETAL_DIR)/metal/system/freertos/*.c)

endif

ABSOLUTE_CFILES +=$(wildcard $(LIBMETAL_DIR)/metal/*.c)


endif #CONFIG_USE_LIBMETAL