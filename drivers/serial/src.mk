ifdef CONFIG_USE_SERIAL
ifdef CONFIG_FREERTOS_USE_UART
DRIVERS_CSRCS += \
    serial/fpl011/fpl011_os.c
endif
ifdef CONFIG_FREERTOS_USE_SERIAL_V2
DRIVERS_CSRCS += \
    serial/fserial_v2_0/fuart_msg_os.c
endif
endif