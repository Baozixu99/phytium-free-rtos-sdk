ifdef CONFIG_USE_I2C

ifdef CONFIG_USE_FI2C
DRIVERS_CSRCS += \
    i2c/fi2c/fi2c_os.c
endif


ifdef CONFIG_USE_FI2C_V2
DRIVERS_CSRCS += \
    i2c/fi2c_v2_0/fi2c_msg_os.c
endif

endif
