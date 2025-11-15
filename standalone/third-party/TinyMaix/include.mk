# TinyMaix
ifdef CONFIG_USE_TINYMAIX

BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/TinyMaix/include \
			$(SDK_DIR)/third-party/TinyMaix/ports \
			$(SDK_DIR)/third-party/TinyMaix/src \
			$(SDK_DIR)/third-party/TinyMaix/tools/tmdl 

endif #CONFIG_USE_TINYMAIX