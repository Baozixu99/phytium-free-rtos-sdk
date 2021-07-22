export PROJECT_DIR 	?= .

# prepare output dir and start make
include $(FREERTOS_SDK_ROOT)/make/build_freertos.mk

# make and copy image to TFTP folder
boot:
	make
	@cp ./$(CONFIG_TARGET_NAME).bin /mnt/d/tftboot/