# Compiling for specific instance

.PHONY: all clean boot core0 core1 menuconfig_core0 menuconfig_core1 autobuild


core0:
	$(MAKE) -C ./core0 -j

core1:
	$(MAKE) -C ./core1 -j

clean_core0:
	$(MAKE) -C ./core0 clean

clean_core1:
	$(MAKE) -C ./core1 clean

boot_core0:
	$(MAKE) -C ./core0 boot	

boot_core1:
	$(MAKE) -C ./core1 boot

config_amp_d2000_aarch64_core0:
	$(MAKE) -C ./core0 load_d2000_aarch64

config_amp_d2000_aarch64_core1:
	$(MAKE) -C ./core1 load_d2000_aarch64

config_amp_d2000_aarch32_core0:
	$(MAKE) -C ./core0 load_d2000_aarch32

config_amp_d2000_aarch32_core1:
	$(MAKE) -C ./core1 load_d2000_aarch32

config_amp_ft2004_aarch64_core0:
	$(MAKE) -C ./core0 load_ft2004_aarch64

config_amp_ft2004_aarch64_core1:
	$(MAKE) -C ./core1 load_ft2004_aarch64

config_amp_ft2004_aarch32_core0:
	$(MAKE) -C ./core0 load_ft2004_aarch32

config_amp_ft2004_aarch32_core1:
	$(MAKE) -C ./core1 load_ft2004_aarch32

config_amp_e2000d_aarch64_core0:
	$(MAKE) -C ./core0 load_e2000d_aarch64

config_amp_e2000d_aarch64_core1:
	$(MAKE) -C ./core1 load_e2000d_aarch64

config_amp_e2000d_aarch32_core0:
	$(MAKE) -C ./core0 load_e2000d_aarch32

config_amp_e2000d_aarch32_core1:
	$(MAKE) -C ./core1 load_e2000d_aarch32

config_amp_e2000q_aarch64_core0:
	$(MAKE) -C ./core0 load_e2000q_aarch64

config_amp_e2000q_aarch64_core1:
	$(MAKE) -C ./core1 load_e2000q_aarch64

config_amp_e2000q_aarch32_core0:
	$(MAKE) -C ./core0 load_e2000q_aarch32

config_amp_e2000q_aarch32_core1:
	$(MAKE) -C ./core1 load_e2000q_aarch32

menuconfig_core0:
	$(MAKE) -C ./core0 menuconfig

menuconfig_core1:
	$(MAKE) -C ./core1 menuconfig

all: core0 core1
clean: clean_core0 clean_core1
boot: boot_core0 boot_core1
config_amp_d2000_aarch64: config_amp_d2000_aarch64_core0 config_amp_d2000_aarch64_core1
config_amp_d2000_aarch32: config_amp_d2000_aarch32_core0 config_amp_d2000_aarch32_core1
config_amp_ft2004_aarch64: config_amp_ft2004_aarch64_core0 config_amp_ft2004_aarch64_core1
config_amp_ft2004_aarch32: config_amp_ft2004_aarch32_core0 config_amp_ft2004_aarch32_core1
config_amp_e2000d_aarch64: config_amp_e2000d_aarch64_core0 config_amp_e2000d_aarch64_core1
config_amp_e2000d_aarch32: config_amp_e2000d_aarch32_core0 config_amp_e2000d_aarch32_core1
config_amp_e2000q_aarch64: config_amp_e2000q_aarch64_core0 config_amp_e2000q_aarch64_core1
config_amp_e2000q_aarch32: config_amp_e2000q_aarch32_core0 config_amp_e2000q_aarch32_core1


backconfig:
	$(MAKE) -C ./core0 backup_kconfig
	$(MAKE) -C ./core1 backup_kconfig

