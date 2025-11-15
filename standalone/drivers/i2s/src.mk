ifdef CONFIG_USE_FI2S
DRIVERS_CSRCS += \
    fi2s.c\
    fi2s_g.c\
    fi2s_intr.c\
    fi2s_sinit.c
endif

ifdef CONFIG_USE_FI2S_V2
DRIVERS_CSRCS += \
    fi2s_msg_g.c\
    fi2s_msg.c\
    fi2s_msg_intr.c\
    fi2s_msg_sinit.c
endif

ifdef CONFIG_USE_CODEC_V2
DRIVERS_CSRCS += \
    fcodec_msg_g.c\
    fcodec_msg.c\
    fcodec_msg_sinit.c
endif
