ifdef CONFIG_USE_FSPIM
DRIVERS_CSRCS += \
    fspim.c\
    fspim_g.c\
    fspim_hw.c\
    fspim_intr.c\
    fspim_selftest.c\
    fspim_sinit.c
endif


ifdef CONFIG_USE_FSPIM_V2
DRIVERS_CSRCS += \
    fspim_msg_g.c\
    fspim_msg_hw.c\
    fspim_msg_intr.c\
    fspim_msg_sinit.c\
    fspim_msg.c
endif

