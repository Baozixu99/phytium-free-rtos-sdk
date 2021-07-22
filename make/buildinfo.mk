#
# @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
#  
# SPDX-License-Identifier: Apache-2.0.
# 
# @Date: 2021-06-02 14:17:37
# @LastEditTime: 2021-06-02 14:24:21
# @Description:  This files is to print build info for debugging 
# 
# @Modify History: 
#  Ver   Who        Date         Changes
# ----- ------     --------    --------------------------------------
#

.PHONY: info info-objs info-incs info-srcs info-libs info-variables
info: info-objs info-incs info-srcs
	@echo "project path: " $(PROJECT_DIR)
	@echo
	@echo "freertos sdk path: " $(FREERTOS_SDK_ROOT)
	@echo
	@echo "output path: " $(OUTPUT_DIR) 
	@echo 
	@echo "x output path: "$(X_OUTPUT_DIRS)
	@echo
	@echo "link script: "$(LDSNAME)
	@echo
	@echo "lib path: "$(LIBPATH)
	@echo
	@echo "cross compiler: "$(CROSS_COMPILE)

info-objs:
	@echo "all objs files"
	@echo $(notdir $(OBJ_FILES)) 
	@echo

info-incs:
	@echo "all include paths"
	@echo $(patsubst $(FREERTOS_SDK_ROOT)/%, %, $(INCLUDES))
	@echo

info-srcs:
	@echo "all src files"
	@echo $(notdir $(APP_S_SRC) $(APP_C_SRC))
	@echo

info-variables:
	@echo "sdkconfig: "$(PROJECT_DIR)/$(KCONFIG_CONFIG)

info-flags:
	@echo "target arch: "$(TARGET_ARCH)
	@echo "link flags: "$(LDFLAGS)
	@echo "cpp flags: "$(CPPFLAGS)
	@echo "asemblely flags: "$(ASFLAGS)
	@echo "c flags: "$(CFLAGS) 
