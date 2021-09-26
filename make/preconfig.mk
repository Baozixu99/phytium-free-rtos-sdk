export USR_SRC_DIR ?= $(PROJECT_DIR)
export USR_INC_DIR ?= $(PROJECT_DIR)

# Read in dependencies to all Kconfig* files, make sure to run
# oldconfig if changes are detected.
# DO NOT USE CONFIG ITEM BEFORE INCLUDE KCONFIG
# specify menuconfig setting 
export MENUCONFIG_STYLE=aquatic
export KCONFIG_CONFIG=sdkconfig
export OUTPUT_DIR  ?= $(PROJECT_DIR)/build

# 定义不同平台的默认配置文件
ifeq ($(MAKECMDGOALS),config_ft2004_aarch32)
export DEF_KCONFIG=ft2004_aarch32_defconfig
endif

ifeq ($(MAKECMDGOALS),config_ft2004_aarch64)
export DEF_KCONFIG=ft2004_aarch64_defconfig
endif

ifeq ($(MAKECMDGOALS),config_e2000_aarch32)
export DEF_KCONFIG=e2000_aarch32_defconfig
endif

ifeq ($(MAKECMDGOALS),config_e2000_aarch64)
export DEF_KCONFIG=e2000_aarch64_defconfig
endif

ifeq ($(MAKECMDGOALS),config_d2000_aarch32)
export DEF_KCONFIG=d2000_aarch32_defconfig
endif

ifeq ($(MAKECMDGOALS),config_d2000_aarch64)
export DEF_KCONFIG=d2000_aarch64_defconfig
endif

#if sdkconfig not exits, prompt user to load default sdkconfig first
FILE_EXISTS = $(shell if [ -f ./$(KCONFIG_CONFIG) ]; then echo "exist"; else echo "notexist"; fi;)

ifneq ($(MAKECMDGOALS),lddefconfig)
ifeq (FILE_EXISTS, "notexist")
$(error error, please type in 'make lddefconfig DEF_KCONFIG=<file_name>' first!!)
endif

$(shell if [ ! -d $(KCONFIG_CONFIG) ]; then touch ./$(KCONFIG_CONFIG); fi)
include $(PROJECT_DIR)/$(KCONFIG_CONFIG)
else

ifndef DEF_KCONFIG
# make lddefconfig DEF_KCONFIG=qemu_aarch64_baremetal_defconfig
$(error error, please type in 'make lddefconfig DEF_KCONFIG=<file_name>'!!)
endif

endif

