#
# @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
#  
# SPDX-License-Identifier: Apache-2.0.
# 
# @Date: 2021-05-31 16:54:49
# @LastEditTime: 2021-05-31 16:55:15
# @Description:  This files is for baremetal build config
# 
# @Modify History: 
#  Ver   Who        Date         Changes
# ----- ------     --------    --------------------------------------
#

.PHONY: menuconfig oldconfig alldefconfig savedefconfig lddefconfig genconfig
menuconfig: 
	$(FREERTOS_STANDALONE)/lib/Kconfiglib/menuconfig.py
	$(FREERTOS_STANDALONE)/lib/Kconfiglib/genconfig.py

genconfig:
	$(FREERTOS_STANDALONE)/lib/Kconfiglib/genconfig.py

# backup current configs
oldconfig:
	$(FREERTOS_STANDALONE)/lib/Kconfiglib/oldconfig.py

# write configuration where all symbols and set as
#	default val
alldefconfig:
	$(FREERTOS_STANDALONE)/lib/Kconfiglib/alldefconfig.py

# # Saves a minimal configuration file that only lists symbols that differ in value
# #	from their defaults
savedefconfig:
	$(FREERTOS_STANDALONE)/lib/Kconfiglib/savedefconfig.py

lddefconfig:
	@cp $(FREERTOS_SDK_ROOT)/configs/$(DEF_KCONFIG) ./$(KCONFIG_CONFIG) -f
	@echo "get default configs at " $(FREERTOS_SDK_ROOT)/configs/$(DEF_KCONFIG)