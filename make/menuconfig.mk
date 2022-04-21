.PHONY: menuconfig oldconfig alldefconfig savedefconfig lddefconfig genconfig
menuconfig: 
	$(STANDALONE_DIR)/lib/Kconfiglib/menuconfig.py
	$(STANDALONE_DIR)/lib/Kconfiglib/genconfig.py

genconfig:
	$(STANDALONE_DIR)/lib/Kconfiglib/genconfig.py

# backup current configs
oldconfig:
	$(STANDALONE_DIR)/lib/Kconfiglib/oldconfig.py

# write configuration where all symbols and set as
#	default val
alldefconfig:
	$(STANDALONE_DIR)/lib/Kconfiglib/alldefconfig.py

# # Saves a minimal configuration file that only lists symbols that differ in value
# #	from their defaults
savedefconfig:
	$(STANDALONE_DIR)/lib/Kconfiglib/savedefconfig.py

lddefconfig:
	@cp $(FREERTOS_SDK_ROOT)/configs/$(DEF_KCONFIG) $(KCONFIG_DIR)/$(KCONFIG_CONFIG) -f
	@echo "get default configs at " $(FREERTOS_SDK_ROOT)/configs/$(DEF_KCONFIG)

ldconfig:
	cp $(LDCONFIG_ARG) $(KCONFIG_DIR)/$(KCONFIG_CONFIG) -f
	@echo "get configs at " $(LDCONFIG_ARG)
	$(STANDALONE_DIR)/lib/Kconfiglib/genconfig.py

