
BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/tlsf-3.1.0/src

ifdef CONFIG_USE_BAREMETAL

		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/tlsf-3.1.0/port/

endif
