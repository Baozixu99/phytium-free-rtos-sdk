
EXCL_SRC += $(FREERTOS_SDK_ROOT)/standalone/common/fmemory_pool.c


ifdef CONFIG_TARGET_ARMV8_AARCH64
EXCL_SRC += $(FREERTOS_SDK_ROOT)/standalone/arch/armv8/aarch64/gcc/boot.S
EXCL_SRC += $(FREERTOS_SDK_ROOT)/standalone/arch/armv8/aarch64/gcc/vectors.S
EXCL_SRC += $(FREERTOS_SDK_ROOT)/standalone/arch/armv8/aarch64/gcc/vectors_g.c
else
EXCL_SRC += $(FREERTOS_SDK_ROOT)/standalone/arch/armv8/aarch32/gcc/vector.S
endif
