PROJECT_DIR = $(CURDIR)
FREERTOS_SDK_DIR = $(CURDIR)/../..


# # 设置启动镜像名
BOOT_IMG_NAME      ?= freertos

USER_CSRC := main.c
			
USER_ASRC :=  #添加汇编代码
USER_CXXSRC := #添加C++代码

#添加 .h 文件所在目录
USER_INCLUDE := $(PROJECT_DIR)	


include $(FREERTOS_SDK_DIR)/tools/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftpboot

image:
	@$(MAKE) -s clean
	@$(MAKE) -s all -j
	@cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).elf
ifdef CONFIG_OUTPUT_BINARY
	@cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).bin
endif
	@ls $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).* -l
	@$(MAKE) -s check_standalone_version

e2000_jailhouse:
	scp ./$(IMAGE_OUT_NAME).bin root@192.168.4.20:/root