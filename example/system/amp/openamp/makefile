# Compiling for specific instance

.PHONY: all clean boot menuconfig_driver_core menuconfig_device_core 

clean_driver_core:
	$(MAKE) -s -C ./driver_core clean

clean_device_core:
	$(MAKE) -s -C ./device_core clean

clean: clean_driver_core clean_device_core

config_d2000_aarch64:
	$(MAKE) -C ./driver_core load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_test_openamp_driver_core
	$(MAKE) -C ./device_core load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_test_openamp_device_core

config_d2000_aarch32:
	$(MAKE) -C ./driver_core load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_test_openamp_driver_core
	$(MAKE) -C ./device_core load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_test_openamp_device_core

config_ft2004_aarch64:
	$(MAKE) -C ./driver_core load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_DSK_openamp_driver_core
	$(MAKE) -C ./device_core load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_DSK_openamp_device_core

config_ft2004_aarch32:
	$(MAKE) -C ./driver_core load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_DSK_openamp_driver_core
	$(MAKE) -C ./device_core load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_DSK_openamp_device_core

config_e2000d_aarch64:
	$(MAKE) -C ./driver_core load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_openamp_driver_core
	$(MAKE) -C ./device_core load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_openamp_device_core

config_e2000d_aarch32:
	$(MAKE) -C ./driver_core load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_openamp_driver_core
	$(MAKE) -C ./device_core load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_openamp_device_core

config_e2000q_aarch64:
	$(MAKE) -C ./driver_core load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_openamp_driver_core
	$(MAKE) -C ./device_core load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_openamp_device_core

config_e2000q_aarch32:
	$(MAKE) -C ./driver_core load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_openamp_driver_core
	$(MAKE) -C ./device_core load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_openamp_device_core

config_phytiumpi_aarch64:
	$(MAKE) -C ./driver_core load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_openamp_driver_core
	$(MAKE) -C ./device_core load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_openamp_device_core

config_phytiumpi_aarch32:
	$(MAKE) -C ./driver_core load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_openamp_driver_core
	$(MAKE) -C ./device_core load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_openamp_device_core

menuconfig_driver_core:
	$(MAKE) -C ./driver_core menuconfig

menuconfig_device_core:
	$(MAKE) -C ./device_core menuconfig

image:
	$(MAKE) -s -C ./driver_core image
	$(MAKE) -s -C ./device_core image

backupconfig:
	$(MAKE) -C ./driver_core backup_kconfig
	$(MAKE) -C ./device_core backup_kconfig

