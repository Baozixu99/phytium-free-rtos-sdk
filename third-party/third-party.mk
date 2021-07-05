
THIRD_PARTY_CUR_DIR := $(FREERTOS_SDK_ROOT)/third-party

ifdef CONFIG_USE_FREERTOS
	EXCL_SRC += $(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_1.c\
				$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_2.c\
				$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_3.c\
				$(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang/heap_5.c

	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/MemMang
	SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos

	ifdef CONFIG_TARGET_ARMV8_AARCH64
		ifdef CONFIG_TARGET_F2000_4
			SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft2004_aarch64
			INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/ft2004_aarch64
		endif #CONFIG_TARGET_F2000_4

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

		ifdef CONFIG_TARGET_AARCH32_QEMU
			SRC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/qemu_aarch32
			INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/portable/GCC/qemu_aarch32
		endif
	endif

	INC_DIR += $(THIRD_PARTY_CUR_DIR)/freertos/include
endif #CONFIG_USE_FREERTOS