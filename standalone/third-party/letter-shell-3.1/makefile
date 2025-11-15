include $(PROJECT_DIR)/sdkconfig


 ifeq (,$(COVERAGE_SRC_PATH))

include src.mk
CSRCS_RELATIVE_FILES := $(foreach file, $(SHELL_CSRCS), $(wildcard */**/$(file) **/$(file)))

else

include $(COVERAGE_SRC_PATH)

endif




include $(SDK_DIR)/tools/build/compiler.mk
