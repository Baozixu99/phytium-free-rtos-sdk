OS_DRIVER_CUR_DIR := $(FREERTOS_SDK_ROOT)/drivers

ifdef CONFIG_FREERTOS_USE_UART
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/serial/fpl011
	INC_DIR += $(OS_DRIVER_CUR_DIR)/serial/fpl011
endif

ifdef CONFIG_FREERTOS_USE_PWM
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/pwm
	INC_DIR += $(OS_DRIVER_CUR_DIR)/pwm
endif

ifdef CONFIG_FREERTOS_USE_QSPI
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/qspi
	INC_DIR += $(OS_DRIVER_CUR_DIR)/qspi
endif

ifdef CONFIG_FREERTOS_USE_WDT
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/wdt
	INC_DIR += $(OS_DRIVER_CUR_DIR)/wdt
endif

ifdef CONFIG_FREERTOS_USE_XMAC
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/eth/xmac
	INC_DIR += $(OS_DRIVER_CUR_DIR)/eth/xmac
endif

ifdef CONFIG_FREERTOS_USE_GMAC
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/eth/gmac
	INC_DIR += $(OS_DRIVER_CUR_DIR)/eth/gmac
endif

ifdef CONFIG_FREERTOS_USE_I2C
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/i2c
	INC_DIR += $(OS_DRIVER_CUR_DIR)/i2c
endif

ifdef CONFIG_FREERTOS_USE_MIO
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/mio
	INC_DIR += $(OS_DRIVER_CUR_DIR)/mio
endif

ifdef CONFIG_FREERTOS_USE_TIMER
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/timer
	INC_DIR += $(OS_DRIVER_CUR_DIR)/timer
endif

ifdef CONFIG_FREERTOS_USE_GPIO
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/gpio/fgpio
	INC_DIR += $(OS_DRIVER_CUR_DIR)/gpio/fgpio
endif

ifdef CONFIG_FREERTOS_USE_FSPIM
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/spi/fspim
	INC_DIR += $(OS_DRIVER_CUR_DIR)/spi/fspim
endif

ifdef CONFIG_FREERTOS_USE_FDDMA
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/dma/fddma
	INC_DIR += $(OS_DRIVER_CUR_DIR)/dma/fddma
endif

ifdef CONFIG_FREERTOS_USE_FGDMA
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/dma/fgdma
	INC_DIR += $(OS_DRIVER_CUR_DIR)/dma/fgdma
endif

ifdef CONFIG_FREERTOS_USE_FSDIO
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/mmc/fsdio
	INC_DIR += $(OS_DRIVER_CUR_DIR)/mmc/fsdio
endif

ifdef CONFIG_FREERTOS_USE_ADC
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/adc
	INC_DIR += $(OS_DRIVER_CUR_DIR)/adc
endif

ifdef CONFIG_FREERTOS_USE_CAN
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/can
	INC_DIR += $(OS_DRIVER_CUR_DIR)/can
endif

ifdef CONFIG_FREERTOS_USE_MEDIA
	SRC_DIR += $(OS_DRIVER_CUR_DIR)/media
	INC_DIR += $(OS_DRIVER_CUR_DIR)/media
endif

