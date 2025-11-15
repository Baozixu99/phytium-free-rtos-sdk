ifdef CONFIG_USE_FI2C
DRIVERS_CSRCS += \
    fi2c.c\
    fi2c_g.c\
    fi2c_hw.c\
    fi2c_intr.c\
    fi2c_master.c\
    fi2c_sinit.c
endif

ifdef CONFIG_USE_FI2C_V2 
DRIVERS_CSRCS += \
    fi2c_msg_g.c \
    fi2c_msg_intr.c\
    fi2c_msg_common.c\
    fi2c_msg_sinit.c \
    fi2c_msg_master.c \
    fi2c_msg_slave.c \
    fi2c_msg.c 
endif


