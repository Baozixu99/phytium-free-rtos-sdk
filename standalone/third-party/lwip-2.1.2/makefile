include $(PROJECT_DIR)/sdkconfig

ifeq (,$(COVERAGE_SRC_PATH))

include src.mk
CSRCS_RELATIVE_FILES := $(LWIP_FILES)

else

include $(COVERAGE_SRC_PATH)

endif

#LIBS_NAME ?= liblwip

include $(SDK_DIR)/tools/build/compiler.mk