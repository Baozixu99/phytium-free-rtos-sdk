THIRD_PARTY_CUR_DIR := $(FREERTOS_SDK_ROOT)/third-party

EXCL_SRC += $(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_1.c\
			$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_2.c\
			$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_3.c\
			$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_5.c

SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang
SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable
SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos

INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft_platform

ifdef CONFIG_TARGET_ARMV8_AARCH64
	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch64
	INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch64
endif #CONFIG_TARGET_ARMV8_AARCH64

ifdef CONFIG_TARGET_ARMV8_AARCH32
	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch32
	INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch32
endif #CONFIG_TARGET_ARMV8_AARCH32


ifdef CONFIG_USE_LWIP

include $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/lwip_freertos.mk 

# src code of lwip
# SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/api \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/core \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/core/ipv4 \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/core/ipv6 \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/netif \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/arch \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/apps/if

# INC_DIR += 	$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include \
# 		    $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/compat \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip/apps \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip/priv \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip/port \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/netif \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/arch

# EXCL_SRC +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/netif/slipif.c \
# 			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/arch/sys_arch_raw.c

# # src code of ports
# ifdef CONFIG_LWIP_FGMAC
# INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/rtos/fgmac 

# SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/rtos/fgmac 
# endif

# ifdef CONFIG_LWIP_FXMAC
# INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/rtos/fxmac 
# SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/rtos/fxmac 

# endif

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

ifdef CONFIG_USE_FATFS_0_1_4

include $(THIRD_PARTY_CUR_DIR)/fatfs-0.1.4/fatfs.mk

endif #CONFIG_USE_FATFS_0_1_4

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

ifdef CONFIG_USE_TLSF

INC_DIR += $(THIRD_PARTY_CUR_DIR)/tlsf-3.1.0/src \
			$(THIRD_PARTY_CUR_DIR)/tlsf-3.1.0/port
SRC_DIR += $(THIRD_PARTY_CUR_DIR)/tlsf-3.1.0/src \
			$(THIRD_PARTY_CUR_DIR)/tlsf-3.1.0/port

endif #CONFIG_USE_TLSF

ifdef CONFIG_USE_SDMMC_CMD

$(shell export PATH=$(THIRD_PARTY_CUR_DIR)/sdmmc-1.0:$PATH)

include $(THIRD_PARTY_CUR_DIR)/sdmmc-1.0/sdmmc.mk

endif #CONFIG_USE_SDMMC_CMD

ifdef CONFIG_USE_CHERRY_USB

include $(THIRD_PARTY_CUR_DIR)/cherryusb-0.6.0/makefile

endif #CONFIG_USE_CHERRY_USB
