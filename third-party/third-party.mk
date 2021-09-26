
THIRD_PARTY_CUR_DIR := $(FREERTOS_SDK_ROOT)/third-party

EXCL_SRC += $(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_1.c\
			$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_2.c\
			$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_3.c\
			$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_5.c

SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang
SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable
SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos

ifdef CONFIG_TARGET_ARMV8_AARCH64
	ifdef CONFIG_TARGET_F2000_4
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft2004_aarch64
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft2004_aarch64
	endif #CONFIG_TARGET_F2000_4

	ifdef CONFIG_TARGET_D2000
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/d2000_aarch64
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/d2000_aarch64
	endif #CONFIG_TARGET_D2000

	ifdef CONFIG_TARGET_E2000
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/e2000_aarch64
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/e2000_aarch64
	endif #CONFIG_TARGET_E2000
endif #CONFIG_TARGET_ARMV8_AARCH64

ifdef CONFIG_TARGET_ARMV8_AARCH32
	ifdef CONFIG_TARGET_F2000_4
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft2004_aarch32
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft2004_aarch32
	endif

	ifdef CONFIG_TARGET_D2000
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/d2000_aarch32
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/d2000_aarch32
	endif

endif #CONFIG_TARGET_ARMV8_AARCH32


ifdef CONFIG_USE_LWIP
# src code of lwip
SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/api \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/core \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/core/ipv4 \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/netif

INC_DIR += 	$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include \
		    $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/compat \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip/apps \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip/priv \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/lwip/prot \
			$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/include/netif

# src code of ports
ifdef CONFIG_ENABLE_F_GMAC
	INC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/f_gmac \
				$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/f_gmac/arch

	SRC_DIR +=  $(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/f_gmac \
				$(THIRD_PARTY_CUR_DIR)/lwip-2.1.2/ports/f_gmac/arch
endif

endif #CONFIG_USE_LWIP

INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/include




