
PROJECT_DIR = $(CURDIR)
FREERTOS_SDK_DIR = $(CURDIR)/../../../..

include $(PROJECT_DIR)/sdkconfig
# # 设置启动镜像名
BOOT_IMG_NAME      ?= freertos

USER_CSRC := main.c
USER_CSRC += src/cmd_i2c.c

ifdef CONFIG_PHYTIUMPI_FIREFLY_BOARD
USER_CSRC += src/i2c_ms_example.c
endif

ifdef CONFIG_PD2308_DEMO_BOARD
USER_CSRC += src/i2c_ms_example.c
endif

ifdef CONFIG_E2000Q_DEMO_BOARD
USER_CSRC += src/i2c_rtc_example.c
endif

ifdef CONFIG_E2000D_DEMO_BOARD
USER_CSRC += src/i2c_rtc_example.c
endif
			
USER_ASRC := 
USER_CXXSRC := 

USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc \


include $(FREERTOS_SDK_DIR)/tools/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftpboot


image:
	$(MAKE) clean
	$(MAKE) all -j
	@cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/freertos.elf
ifdef CONFIG_OUTPUT_BINARY
	@cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/freertos.bin
endif
	@ls $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).* -l
	@$(MAKE) -s check_standalone_version