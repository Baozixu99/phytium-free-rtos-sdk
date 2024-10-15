

TLSF_RT_C_DIR = $(abspath $(SDK_DIR)/third-party/tlsf-3.1.0/)

ABSOLUTE_CFILES += $(wildcard $(TLSF_RT_C_DIR)/src/*.c)

ifdef CONFIG_USE_FREERTOS
	CSRCS_RELATIVE_FILES += $(wildcard port/*.c)
endif


