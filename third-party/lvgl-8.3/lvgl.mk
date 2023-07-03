THIRD_PARTY_CUR_DIR := $(FREERTOS_SDK_ROOT)/third-party

include $(STANDALONE_DIR)/third-party/lvgl-8.3/lvgl.mk

include

ifdef CONFIG_USE_FREERTOS

		INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lvgl-8.3/port\

					
		SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lvgl-8.3/port\
			

endif