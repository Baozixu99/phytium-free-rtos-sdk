
ifdef CONFIG_USE_GMSSL

BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/GmSSL-3.1.1 \
                       $(SDK_DIR)/third-party/GmSSL-3.1.1/include \
											 $(SDK_DIR)/third-party/GmSSL-3.1.1/ports


ifdef CONFIG_TARGET_ARMV8_AARCH32
	BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/GmSSL-3.1.1/metal/processor/arm 
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/GmSSL-3.1.1/metal/processor/aarch64 
endif


endif #CONFIG_USE_GMSSL

