
SOC_CSRCS += \
    fmmu_table.c\
    fcpu_affinity_mask.c

ifeq ($(CONFIG_ENABLE_FGPIO),y)
SOC_CSRCS += fgpio_table.c
endif