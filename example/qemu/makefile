CROSS_COMPILE ?= $(AARCH32_LINUX_CROSS_PATH)/bin/arm-linux-gnueabihf-
TARGET		  ?= qemu_freertos

CC 				:= $(CROSS_COMPILE)gcc
LD 				:= $(CROSS_COMPILE)ld
OBJCOPY 		:= $(CROSS_COMPILE)objcopy
OBJDUMP			:= $(CROSS_COMPILE)objdump
SDCARD          := ./sdcard
IMAGE			:= fs.img
QEMU            := qemu-system-arm
BOOTLOADER      := $(FREERTOS_SDK_ROOT)/third-party/bootloader
LDSNAME			:= ./qemu_aarch32_freertos.ld
OUTPUTDIR		:= ./build
THIRD_PARTY_CUR_DIR := $(FREERTOS_SDK_ROOT)/third-party

INCUDIRS		:= 	. \
					./include \
					./include/sdk \
					./src/fatfs \
					$(THIRD_PARTY_CUR_DIR)/freertos/include \
					$(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/qemu_aarch32

SRCDIRS			:= 	. \
					./src \
					./src/app \
					./src/app/cmd \
					./src/driver \
					./src/fatfs \
					$(THIRD_PARTY_CUR_DIR)/freertos \
					$(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/qemu_aarch32 \
					$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/qemu_aarch32/
					

INCLUDE 		:= 	$(patsubst %, -I %, $(INCUDIRS))

INCLUDE 		:= 	$(patsubst %, -I %, $(INCUDIRS))

SFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))
CFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

SFILENDIR		:= $(notdir $(SFILES))
CFILENDIR		:= $(notdir $(CFILES))

SOBJS			:= $(patsubst %, $(OUTPUTDIR)/%, $(SFILENDIR:.S=.o))
COBJS			:= $(patsubst %, $(OUTPUTDIR)/%, $(CFILENDIR:.c=.o))

OBJS			:= $(SOBJS)$(COBJS)

VPATH			:= $(SRCDIRS)

$(shell if [ ! -e $(OUTPUTDIR) ];then mkdir -p $(OUTPUTDIR); fi)

.PHONY:clean

$(TARGET).bin : $(OBJS)
	$(LD) -T$(LDSNAME) -o $(TARGET).elf $^
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -S -m arm $(TARGET).elf > $(TARGET).dis
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).map

$(SOBJS) : $(OUTPUTDIR)/%.o : %.S
	$(CC) -Wall -nostdlib -c -g -O0 -mno-unaligned-access -Wall -mfloat-abi=hard -mfpu=vfpv4 -MMD -MP -fno-common -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mapcs -std=gnu99 -mcpu=cortex-a7  -DDEBUG -DCPU_MCIMX6Y2DVM05 -DFSL_RTOS_FREE_RTOS -DEVK_MCIMX6ULL $(INCLUDE) -o $@ $<

$(COBJS) : $(OUTPUTDIR)/%.o : %.c
	#$(CC) -Wall -nostdlib -c -O0 -g -mcpu=cortex-a7 $(INCLUDE) -o $@ $<
	$(CC) -Wall -nostdlib -c -g -O0 -mno-unaligned-access -Wall -mfloat-abi=hard -mfpu=vfpv4 -MMD -MP -fno-common -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mapcs -std=gnu99 -mcpu=cortex-a7 -DDEBUG -DCPU_MCIMX6Y2DVM05 -DFSL_RTOS_FREE_RTOS -DEVK_MCIMX6ULL $(INCLUDE) -o $@ $<

image:
	rm $(IMAGE) -f
	rm $(SDCARD) -rf
	mkdir $(SDCARD)
	mkfs.msdos -F 32 -C $(IMAGE) 131072
	sudo mount -t msdos -o loop $(IMAGE) $(SDCARD)/

bootloader: 
	make -C $(BOOTLOADER)/

boot: $(OBJS)
	sudo cp $(TARGET).bin $(SDCARD)/os.bin
	$(QEMU) -M mcimx6ul-evk -m 512M -kernel $(BOOTLOADER)/bootloader.elf \
	-serial stdio -sd $(IMAGE)

clean:
	rm $(OUTPUTDIR) -rf
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).dis $(OBJS) *.map

print:
	@echo INCLUDE = $(INCLUDE)
	@echo SFILES = $(SFILES)
	@echo CFILES = $(CFILES)
	@echo SFILENDIR = $(SFILENDIR)
	@echo CFILENDIR = $(CFILENDIR)
	@echo SOBJS = $(SOBJS)
	@echo COBJS = $(COBJS)
	@echo OBJS = $(OBJS)