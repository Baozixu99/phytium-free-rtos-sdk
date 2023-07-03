export STANDALONE_DIR?=$(FREERTOS_STANDALONE)

include $(STANDALONE_DIR)/standalone.mk#裸跑相关依赖
include $(STANDALONE_DIR)/lib/lib.mk#libc 相关依赖

EXCL_SRC += $(STANDALONE_DIR)/common/fmemory_pool.c

ifdef CONFIG_TARGET_ARMV8_AARCH32
EXCL_SRC += $(STANDALONE_DIR)/arch/armv8/aarch32/gcc/fvectors.S
endif
