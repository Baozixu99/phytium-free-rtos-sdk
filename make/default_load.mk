
# 根据例程默认，编译各平台的镜像
.PHONY: load_e2000_aarch32 load_e2000_aarch64 backup_kconfig build_ft2004_aarch32 build_ft2004_aarch64 build_d2000_aarch32 build_d2000_aarch64 config_ft2004_aarch32 config_ft2004_aarch64 config_d2000_aarch32 config_d2000_aarch64

load_ft2004_aarch32:
	@echo "========Load for FT2000/4 AARCH32 Start============="
	make ldconfig LDCONFIG_ARG=$(EG_CONFIGS_DIR)/ft2004_aarch32_eg_configs
	@echo "==============FT2000/4 AARCH32======================="

load_ft2004_aarch64:
	@echo "=======Load for FT2000/4 AARCH64 Start==============="
	make ldconfig LDCONFIG_ARG=$(EG_CONFIGS_DIR)/ft2004_aarch64_eg_configs
	@echo "==============FT2000/4 AARCH64======================="

load_d2000_aarch32:
	@echo "=======Load for D2000 AARCH32===================="
	make ldconfig LDCONFIG_ARG=$(EG_CONFIGS_DIR)/d2000_aarch32_eg_configs
	@echo "==============D2000 AARCH32======================="

load_d2000_aarch64:
	@echo "========Load for D2000 AARCH64==================="
	make ldconfig LDCONFIG_ARG=$(EG_CONFIGS_DIR)/d2000_aarch64_eg_configs
	@echo "==============D2000 AARCH64========================="

load_e2000_aarch32:
	@echo "=======Load for E2000 AARCH32===================="
	make ldconfig LDCONFIG_ARG=$(EG_CONFIGS_DIR)/e2000_aarch32_eg_configs
	@echo "==============E2000 AARCH32======================="

load_e2000_aarch64:
	@echo "========Load for E2000 AARCH64==================="
	make ldconfig LDCONFIG_ARG=$(EG_CONFIGS_DIR)/e2000_aarch64_eg_configs
	@echo "==============E2000 AARCH64========================="


backup_kconfig:
ifdef CONFIG_TARGET_ARMV8_AARCH32 
ifdef CONFIG_TARGET_D2000
	cp sdkconfig $(EG_CONFIGS_DIR)/d2000_aarch32_eg_configs
endif
ifdef CONFIG_TARGET_F2000_4
	cp sdkconfig $(EG_CONFIGS_DIR)/ft2004_aarch32_eg_configs
endif
ifdef CONFIG_TARGET_E2000
	cp sdkconfig $(EG_CONFIGS_DIR)/e2000_aarch32_eg_configs
endif
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64 
ifdef CONFIG_TARGET_D2000
	cp sdkconfig $(EG_CONFIGS_DIR)/d2000_aarch64_eg_configs
endif
ifdef CONFIG_TARGET_F2000_4
	cp sdkconfig $(EG_CONFIGS_DIR)/ft2004_aarch64_eg_configs
endif
ifdef CONFIG_TARGET_E2000
	cp sdkconfig $(EG_CONFIGS_DIR)/e2000_aarch64_eg_configs
endif
endif


config_ft2004_aarch32: lddefconfig genconfig clean
config_ft2004_aarch64: lddefconfig genconfig clean
config_d2000_aarch32: lddefconfig genconfig clean
config_d2000_aarch64: lddefconfig genconfig clean