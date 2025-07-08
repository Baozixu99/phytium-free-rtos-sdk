
PROJECT_DIR = $(CURDIR)
FREERTOS_SDK_DIR = $(CURDIR)/../../../..
-include $(PROJECT_DIR)/sdkconfig

# # 设置启动镜像名
BOOT_IMG_NAME      ?= freertos

USER_CSRC := main.c \
			 src/cmd_usb_host.c \
			 src/usb_host_probe_devices_example.c \
			 src/usb_host_mass_storage_read_write_example.c \
			 src/usb_host_mass_storage_bench_example.c \
			 src/usb_host_mouse_input_example.c \
			 src/usb_host_keyboard_input_example.c
USER_INCLUDE := $(PROJECT_DIR) \
				$(PROJECT_DIR)/inc
USER_ASRC := 
USER_CXXSRC := 

include $(FREERTOS_SDK_DIR)/tools/makeall.mk

ifeq ($(OS),Windows_NT)
USR_BOOT_DIR		?= D:\\tftpboot
else
USR_BOOT_DIR		?= /mnt/d/tftpboot
endif

image: all
	@cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/freertos.elf
ifdef CONFIG_OUTPUT_BINARY
	@cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/freertos.bin
endif
	@ls $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).* -l

mk_xhcilib:
ifdef CONFIG_TARGET_ARMV8_AARCH32
ifdef CONFIG_MFLOAT_ABI_HARD
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/libxhci_a32_hardfp.a ./build/cherryusb/fxhci/*.o
endif
ifdef CONFIG_MFLOAT_ABI_SOFTFP
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/libxhci_a32_softfp_$(CONFIG_ARM_MFPU).a ./build/cherryusb/fxhci/*.o
endif
else
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/libxhci_a64.a ./build/cherryusb/fxhci/*.o
endif

mk_pusb2lib:
ifdef CONFIG_TARGET_ARMV8_AARCH32
ifdef CONFIG_MFLOAT_ABI_HARD
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_hc_a32_hardfp.a ./build/cherryusb/fpusb2/*.o
endif
ifdef CONFIG_MFLOAT_ABI_SOFTFP
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_hc_a32_softfp_$(CONFIG_ARM_MFPU).a ./build/cherryusb/fpusb2/*.o
endif
else
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_hc_a64.a ./build/cherryusb/fpusb2/*.o
endif

mk_pusb3lib:
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/pusb3/libpusb3_hc_a64.a ./build/cherryusb/fpusb3/*.o ./build/cherryusb/fxhci/*.o

all_libs:
	@rm $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/libxhci_a64.a -f
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_xhci_no_log
	make clean image mk_xhcilib
	@rm $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/libxhci_a32_hardfp.a -f
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_xhci_no_log_hardfp
	make clean image mk_xhcilib
	@rm $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/libxhci_a32_softfp*.a -f
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_xhci_no_log_softfp
	make clean image mk_xhcilib
	@rm $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_hc_a64.c -f
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_pusb2_no_log
	make clean image mk_pusb2lib
	@rm $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_hc_a32_hardfp.c -f
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_pusb2_hardfp_no_log
	make clean image mk_pusb2lib
	@rm $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_hc_a32_softfp*.c -f
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_pusb2_softfp_no_log
	make clean image mk_pusb2lib
	@rm $(SDK_DIR)/third-party/cherryusb/port/pusb3/libpusb3_hc_a64.c -f
	make load_kconfig LOAD_CONFIG_NAME=pd2408_aarch64_demo_pusb3_non_log
	make clean image mk_pusb3lib

all_images:
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_xhci_lib
	make clean image
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_xhci_lib_hardfp
	make clean image
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_xhci_lib_softfp
	make clean image
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_pcie_xhci_lib
	make clean image
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_pcie_xhci_lib_hardfp
	make clean image
	make load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_pcie_xhci_lib_softfp
	make clean image
	make load_kconfig LOAD_CONFIG_NAME=pd2408_aarch64_demo_pcie_xhci_lib
	make clean image
	make load_kconfig LOAD_CONFIG_NAME=pd2408_aarch64_demo_pusb3_lib
	make clean image