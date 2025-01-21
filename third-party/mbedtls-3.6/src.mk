
ifdef CONFIG_USE_MBEDTLS

MBEDTLS_RT_C_DIR = $(abspath $(FREERTOS_SDK_DIR)/third-party/mbedtls-3.6)

# src code of lwip
ABSOLUTE_CFILES  += $(wildcard $(MBEDTLS_RT_C_DIR)/library/*.c) \
				$(wildcard $(MBEDTLS_RT_C_DIR)/ports/src/*.c)

endif #CONFIG_USE_MBEDTLS

