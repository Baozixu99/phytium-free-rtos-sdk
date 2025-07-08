ifdef CONFIG_USE_SPI

ifdef CONFIG_USE_FSPIM
DRIVERS_CSRCS += \
    spi/fspim/fspim_os.c
endif


ifdef CONFIG_USE_FSPIM_V2
DRIVERS_CSRCS += \
    spi/fspim_v2_0/fspim_msg_os.c
endif

endif
