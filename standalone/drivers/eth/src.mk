ifdef CONFIG_ENABLE_FGMAC
DRIVERS_CSRCS += \
    fgmac.c\
    fgmac_dma.c\
    fgmac_g.c\
    fgmac_hw.c\
    fgmac_intr.c\
    fgmac_sinit.c

DRIVERS_CSRCS += phy/fgmac_phy.c 

ifdef CONFIG_FGMAC_PHY_AR803X
    DRIVERS_CSRCS += phy/ar803x/fgmac_ar803x.c
endif

endif

ifdef CONFIG_ENABLE_FXMAC
DRIVERS_CSRCS += \
    fxmac.c\
    fxmac_bdring.c\
    fxmac_debug.c\
    fxmac_g.c\
    fxmac_intr.c\
    fxmac_options.c\
    fxmac_phy.c\
    fxmac_sinit.c

ifdef CONFIG_FXMAC_PHY_YT
    DRIVERS_CSRCS += phy/yt/phy_yt.c
endif

endif

ifdef CONFIG_ENABLE_FXMAC_V2
DRIVERS_CSRCS += \
    fxmac_msg_bdring.c\
    fxmac_msg_common.c\
    fxmac_msg_debug.c\
    fxmac_msg_g.c\
    fxmac_msg_intr.c\
    fxmac_msg_phy.c\
    fxmac_msg_sinit.c\
    fxmac_msg.c
endif

ifdef CONFIG_ENABLE_E1000E
DRIVERS_CSRCS += \
    e1000e.c\
    e1000e_g.c\
    e1000e_sinit.c\
    e1000e_dma.c\
    e1000e_hw.c\
    e1000e_debug.c\
    e1000e_phy.c\
    e1000e_intr.c
endif
