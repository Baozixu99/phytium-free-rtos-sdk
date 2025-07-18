PROJECT_DIR = $(CURDIR)
FREERTOS_SDK_DIR = $(CURDIR)/../../../..
-include $(PROJECT_DIR)/sdkconfig

# # 设置启动镜像名
BOOT_IMG_NAME      ?= freertos

USER_CSRC := main.c \
			 src/cmd_usb_device.c \
			 src/usb_device_mass_storage_example.c \
			 src/usb_device_virtual_serial_example.c \
			 src/usb_device_virtual_keyboard_example.c
			
USER_ASRC := 
USER_CXXSRC := 

USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc

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

mk_pusb2lib:
ifdef CONFIG_TARGET_ARMV8_AARCH32
ifdef CONFIG_MFLOAT_ABI_HARD
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_dc_a32_hardfp.a ./build/cherryusb/fpusb2/*.o
endif
ifdef CONFIG_MFLOAT_ABI_SOFTFP
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_dc_a32_softfp_$(CONFIG_ARM_MFPU).a ./build/cherryusb/fpusb2/*.o
endif
else
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_dc_a64.a ./build/cherryusb/fpusb2/*.o
endif

mk_pusb3lib:
	$(CROSS_COMPILE)ar rcs $(SDK_DIR)/third-party/cherryusb/port/pusb3/libpusb3_dc_a64.a ./build/cherryusb/fpusb3/*.o

all_images:
	make load_kconfig LOAD_CONFIG_NAME=pe2204_aarch64_demo_pusb2_lib
	make clean image
	make load_kconfig LOAD_CONFIG_NAME=pe2204_aarch32_demo_pusb2_hardfp_lib
	make clean image
	make load_kconfig LOAD_CONFIG_NAME=pe2204_aarch32_demo_pusb2_softfp_lib
	make clean image
	make load_kconfig LOAD_CONFIG_NAME=pd2408_aarch64_demo_pusb3_lib
	make clean image