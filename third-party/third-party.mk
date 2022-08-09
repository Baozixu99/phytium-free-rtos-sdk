THIRD_PARTY_CUR_DIR := $(FREERTOS_SDK_ROOT)/third-party

EXCL_SRC += $(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_1.c\
			$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_2.c\
			$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_3.c\
			$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_5.c

SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang
SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable
SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos

ifdef CONFIG_TARGET_ARMV8_AARCH64
	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch64
	INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch64
endif #CONFIG_TARGET_ARMV8_AARCH64

ifdef CONFIG_TARGET_ARMV8_AARCH32
	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch32
	INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch32
endif #CONFIG_TARGET_ARMV8_AARCH32


ifdef CONFIG_USE_LWIP
# src code of lwip
SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/api \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/core \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/core/ipv4 \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/core/ipv6 \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/netif

INC_DIR += 	$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include \
		    $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/compat \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip/apps \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip/priv \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip/prot \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/netif

# src code of ports
ifdef CONFIG_LWIP_FGMAC
INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/fgmac \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/fgmac/arch \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports

SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/fgmac \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/fgmac/arch \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports
endif

ifdef CONFIG_LWIP_FXMAC
INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/fxmac \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/fxmac/arch \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/apps/ping

SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/fxmac \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/fxmac/arch \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/apps/ping

endif

endif #CONFIG_USE_LWIP

INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/include


ifdef CONFIG_USE_LIBMETAL

$(shell export PATH=$(THIRD_PARTY_CUR_DIR)/libmetal:$PATH)

	INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/libmetal \
				$(THIRD_PARTY_CUR_DIR)/libmetal/metal/compiler/gcc \
				$(THIRD_PARTY_CUR_DIR)/libmetal/metal/system/freertos/ft_platform

	SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/libmetal/metal \
				$(THIRD_PARTY_CUR_DIR)/libmetal/metal/system/freertos/ft_platform


ifdef CONFIG_TARGET_ARMV8_AARCH32
	INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/libmetal/metal/processor/arm 
	SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/libmetal/metal/system/freertos
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/libmetal/metal/processor/aarch64 
	SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/libmetal/metal/system/freertos
endif

endif #CONFIG_USE_LIBMETAL


ifdef CONFIG_USE_OPENAMP

INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/openamp/lib \
			$(THIRD_PARTY_CUR_DIR)/openamp/lib/include \
			$(THIRD_PARTY_CUR_DIR)/openamp/lib/rpmsg \
			$(THIRD_PARTY_CUR_DIR)/openamp/ports


SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/openamp/lib \
			$(THIRD_PARTY_CUR_DIR)/openamp/lib/remoteproc \
			$(THIRD_PARTY_CUR_DIR)/openamp/lib/rpmsg \
			$(THIRD_PARTY_CUR_DIR)/openamp/lib/service/rpmsg/rpc \
			$(THIRD_PARTY_CUR_DIR)/openamp/lib/virtio \
			$(THIRD_PARTY_CUR_DIR)/openamp/ports
			 

endif #CONFIG_USE_OPENAMP

ifdef CONFIG_USE_BACKTRACE
	INC_DIR += $(THIRD_PARTY_CUR_DIR)/backtrace
	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/backtrace
endif #CONFIG_USE_BACKTRACE


ifdef CONFIG_USE_FATFS

	INC_DIR += $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.3
	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.3

	ifdef CONFIG_SELECT_FATFS_RAM_DISK
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.3/port/ramdisk
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.3/port/ramdisk
	endif

	ifdef CONFIG_SELECT_FATFS_FSDMMC
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.3/port/fsdmmc
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.3/port/fsdmmc
	endif

	ifdef CONFIG_SELECT_FATFS_FSATA
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.3/port/fsata_pcie
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.3/port/fsata_pcie
	endif
	

	ifdef CONFIG_SELECT_FATFS_USB
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.3/port/fusb_pcie
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.3/port/fusb_pcie
	endif
endif #CONFIG_USE_FATFS


ifdef CONFIG_USE_SFUD

# $(shell export PATH=$(THIRD_PARTY_CUR_DIR)/sfud:$PATH)

	INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/sfud-1.1.0 \
				$(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/inc \
				$(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports
	SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/sfud-1.1.0 \
				$(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/src \
				$(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports

	ifdef CONFIG_SFUD_CTRL_FSPIM
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports/fspim
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports/fspim	
	endif

	ifdef CONFIG_SFUD_CTRL_FQSPI
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports/fqspi
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports/fqspi	
	endif

endif #CONFIG_USE_SFUD

# spiffs
ifdef CONFIG_USE_SPIFFS

INC_DIR += $(THIRD_PARTY_CUR_DIR)/spiffs-0.3.7/inc \
			$(THIRD_PARTY_CUR_DIR)/spiffs-0.3.7/ports
SRC_DIR += $(THIRD_PARTY_CUR_DIR)/spiffs-0.3.7/src \
			$(THIRD_PARTY_CUR_DIR)/spiffs-0.3.7/ports

ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
	INC_DIR += $(THIRD_PARTY_CUR_DIR)/spiffs-0.3.7/ports/fspim
	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/spiffs-0.3.7/ports/fspim
endif #CONFIG_SPIFFS_ON_FSPIM_SFUD

ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
	INC_DIR += $(THIRD_PARTY_CUR_DIR)/spiffs-0.3.7/ports/fqspi
	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/spiffs-0.3.7/ports/fqspi
endif #CONFIG_SPIFFS_ON_FSPIM_SFUD

endif #CONFIG_USE_SPIFFS


ifdef CONFIG_USE_LETTER_SHELL
# src files
INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/letter-shell-3.1/port \
			$(THIRD_PARTY_CUR_DIR)/letter-shell-3.1/src

# inc files
SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/letter-shell-3.1/port \
			$(THIRD_PARTY_CUR_DIR)/letter-shell-3.1/port/cmd \
			$(THIRD_PARTY_CUR_DIR)/letter-shell-3.1/src

ifdef CONFIG_LS_PL011_UART
	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/letter-shell-3.1/port/pl011
endif

endif #CONFIG_USE_LETTER_SHELL




