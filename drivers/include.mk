
# FREERTOS_SDK_DIR ?= $(SDK_DIR)/..

OS_DRV_CUR_DIR := $(FREERTOS_SDK_DIR)/drivers


##########################drivers####################################
# timer
ifdef CONFIG_ENABLE_TIMER_TACHO
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/timer
endif

# mio
ifdef CONFIG_ENABLE_MIO
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/mio/fmio
endif

# qspi
ifdef CONFIG_USE_FQSPI
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/qspi/fqspi
endif #CONFIG_USE_QSPI

# usart
ifdef CONFIG_FREERTOS_USE_UART
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/serial/fpl011
endif #CONFIG_ENABLE_Pl011_UART

ifdef CONFIG_FREERTOS_USE_SERIAL_V2
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/serial/fserial_v2_0
endif #CONFIG_USE_SERIAL_V2

# spi
ifdef CONFIG_USE_FSPIM
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/spi/fspim
endif

ifdef CONFIG_USE_FSPIM_V2
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/spi/fspim_v2_0
endif


# eth
ifdef CONFIG_ENABLE_FGMAC
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/eth/gmac
endif #CONFIG_ENABLE_FGMAC


ifdef CONFIG_ENABLE_FXMAC
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/eth/xmac
endif

ifdef CONFIG_ENABLE_FXMAC_V2
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/eth/xmac_v2_0
endif

# can
ifdef CONFIG_USE_FCAN
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/can
endif

# i2c
ifdef CONFIG_USE_FI2C
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/i2c/fi2c
endif

ifdef CONFIG_USE_FI2C_V2
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/i2c/fi2c_v2_0
endif

#wdt
ifdef CONFIG_USE_FWDT
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/wdt
endif

# dma
ifdef CONFIG_ENABLE_FGDMA
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/dma/fgdma
endif

ifdef CONFIG_ENABLE_FDDMA
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/dma/fddma
endif

#nand
ifdef CONFIG_ENABLE_FNAND
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/nand/fnand
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/nand/fnand/manufacturer
endif


#rtc
ifdef CONFIG_ENABLE_FRTC
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/rtc/frtc
endif

#usb
ifdef CONFIG_USE_USB

	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/usb

ifdef CONFIG_ENABLE_USB_FXHCI
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/usb/fxhci
endif
endif

#adc
ifdef CONFIG_USE_FADC
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/adc
endif

#pwm
ifdef CONFIG_USE_FPWM
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/pwm
endif

#semaphore
ifdef CONFIG_ENABLE_FSEMAPHORE
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/ipc/fsemaphore
endif

#scmi_mhu
ifdef CONFIG_ENABLE_SCMI_MHU
	BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/scmi/fscmi_mhu
endif

ifdef CONFIG_USE_MEDIA
		BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/media
endif

ifdef CONFIG_USE_I2S
		BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/i2s
endif

BUILD_INC_PATH_DIR += $(OS_DRV_CUR_DIR)/port