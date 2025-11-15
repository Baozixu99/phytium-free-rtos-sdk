
ifdef CONFIG_USE_CMSIS_DSP

    BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/CMSIS/CORE/Include \
				$(SDK_DIR)/third-party/CMSIS/DSP/Include \
			    $(SDK_DIR)/third-party/CMSIS/DSP/Include/dsp \
			    $(SDK_DIR)/third-party/CMSIS/DSP/PrivateInclude \
				$(SDK_DIR)/third-party/CMSIS/DSP/ComputeLibrary/Include

endif