ifdef CONFIG_ENABLE_FSDMMC
DRIVERS_CSRCS += \
    fsdmmc.c\
    fsdmmc_dma.c\
    fsdmmc_g.c\
    fsdmmc_hw.c\
    fsdmmc_intr.c\
    fsdmmc_sinit.c
endif

ifdef CONFIG_ENABLE_FSDIF
DRIVERS_CSRCS += \
    fsdif/fsdif.c\
    fsdif/fsdif_cmd.c\
    fsdif/fsdif_dma.c\
    fsdif/fsdif_g.c\
    fsdif/fsdif_intr.c\
    fsdif/fsdif_pio.c\
    fsdif/fsdif_selftest.c\
    fsdif/fsdif_sinit.c
endif

ifdef CONFIG_ENABLE_FSDIF_V2
DRIVERS_CSRCS += \
    fsdif_v2_0/fsdif_msg.c\
    fsdif_v2_0/fsdif_msg_g.c\
    fsdif_v2_0/fsdif_msg_intr.c\
    fsdif_v2_0/fsdif_msg_common.c\
    fsdif_v2_0/fsdif_msg_sinit.c
endif