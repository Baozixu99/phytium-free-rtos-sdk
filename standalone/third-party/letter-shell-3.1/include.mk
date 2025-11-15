

ifdef CONFIG_USE_LETTER_SHELL
THIRDP_CUR_DIR := $(SDK_DIR)/third-party

# src files
BUILD_INC_PATH_DIR +=  $(THIRDP_CUR_DIR)/letter-shell-3.1/port \
			$(THIRDP_CUR_DIR)/letter-shell-3.1/src \
			$(THIRDP_CUR_DIR)/letter-shell-3.1

ifdef CONFIG_LS_PL011_UART
	BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/letter-shell-3.1/port/pl011
endif

ifdef CONFIG_LS_SERIAL_V2
	BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/letter-shell-3.1/port/serial_v2
endif

endif #CONFIG_USE_LETTER_SHELL

