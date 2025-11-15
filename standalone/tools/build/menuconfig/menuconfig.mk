.PHONY: menuconfig oldconfig alldefconfig savedefconfig lddefconfig genconfig setconfig ldconfig

menuconfig:
	$(PYTHON) $(SDK_KCONFIG_DIR)/menuconfig.py
	$(PYTHON) $(SDK_KCONFIG_DIR)/genconfig.py

update_menuconfig:
	$(PYTHON) $(SDK_KCONFIG_DIR)/menuconfig_autosave.py
	$(PYTHON) $(SDK_KCONFIG_DIR)/genconfig.py


setconfig:
	$(PYTHON) $(SDK_KCONFIG_DIR)/setconfig.py $(SETCONFIG_ARG)
	$(PYTHON) $(SDK_KCONFIG_DIR)/genconfig.py

genconfig:
	$(PYTHON) $(SDK_KCONFIG_DIR)/genconfig.py

# backup current configs
oldconfig:
	$(PYTHON) $(SDK_KCONFIG_DIR)/oldconfig.py

# write configuration where all symbols and set as
#	default val
alldefconfig:
	$(PYTHON) $(SDK_KCONFIG_DIR)/alldefconfig.py

# # Saves a minimal configuration file that only lists symbols that differ in value
# #	from their defaults
savedefconfig:
	$(PYTHON) $(SDK_KCONFIG_DIR)/savedefconfig.py

lddefconfig:
	cp $(STANDALONE_DIR)/configs/$(DEF_KCONFIG) ./$(KCONFIG_CONFIG) -f
	@echo "get default configs at " $(STANDALONE_DIR)/configs/$(DEF_KCONFIG)

ldconfig:
	cp $(LDCONFIG_ARG) ./$(KCONFIG_CONFIG) -f
	@echo "get configs at " $(LDCONFIG_ARG)
	$(PYTHON) $(SDK_KCONFIG_DIR)/genconfig.py

refresh_config:
	python3 $(SDK_OTHERS_TOOLS_DIR)/refresh_config.py

misrac_check:
	python3 $(SDK_OTHERS_TOOLS_DIR)/misrac_cppcheck/misrac_cppcheck.py --misra-rules $(MISRA_RULES) --files-path $(FILES_PATH)