ifdef CONFIG_ENABLE_FGMAC
DRIVERS_CSRCS += \
   eth/gmac/fgmac_os.c
endif

ifdef CONFIG_ENABLE_FXMAC
DRIVERS_CSRCS += \
    eth/xmac/fxmac_os.c
endif

ifdef CONFIG_ENABLE_FXMAC_V2
DRIVERS_CSRCS += \
    eth/xmac_v2_0/fxmac_msg_os.c
endif

ifdef CONFIG_ENABLE_E1000E
DRIVERS_CSRCS += \
    eth/e1000e/e1000e_os.c
endif