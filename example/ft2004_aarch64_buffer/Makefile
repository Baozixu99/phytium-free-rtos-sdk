
# specify src code dir 
export PROJECT_DIR 	   ?= .

# prepare output dir and start make
include $(FREERTOS_SDK_ROOT)/make/build.mk

boot:
	make
	@cp ./$(CONFIG_TARGET_NAME).bin /mnt/d/tftboot/



