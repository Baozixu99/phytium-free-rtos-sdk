ifdef CONFIG_ENABLE_Pl011_UART
DRIVERS_CSRCS += \
    fpl011.c\
    fpl011_g.c\
    fpl011_hw.c\
    fpl011_intr.c\
    fpl011_options.c\
    fpl011_sinit.c
endif

ifdef CONFIG_USE_SERIAL_V2
DRIVERS_CSRCS += \
    fuart_msg.c\
    fuart_msg_g.c\
    fuart_msg_hw.c\
    fuart_msg_intr.c\
    fuart_msg_sinit.c\
    fuart_msg_options.c
endif
