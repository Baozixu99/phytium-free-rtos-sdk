ifdef CONFIG_USE_CHERRY_USB
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/core \
						  $(SDK_DIR)/third-party/cherryusb/common \
						  $(SDK_DIR)/third-party/cherryusb/osal \
						  $(SDK_DIR)/third-party/cherryusb

ifdef CONFIG_CHERRYUSB_HOST
	ifdef CONFIG_CHERRY_USB_HOST_HUB
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/class/hub
	endif #CONFIG_CHERRY_USB_HOST_HUB

	ifdef CONFIG_CHERRY_USB_HOST_MSC
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/class/msc
	endif #CONFIG_CHERRY_USB_HOST_MSC

	ifdef CONFIG_CHERRY_USB_HOST_HID
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/class/hid
	endif #CONFIG_CHERRY_USB_HOST_HID
endif #CONFIG_CHERRYUSB_HOST

ifdef CONFIG_CHERRYUSB_DEVICE
	ifdef CONFIG_CHERRY_USB_DEVICE_MSC
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/class/msc
	endif

	ifdef CONFIG_CHERRY_USB_DEVICE_CDC
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/class/cdc
	endif

	ifdef CONFIG_CHERRY_USB_DEVICE_CDC
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/class/hid
	endif

	ifdef CONFIG_CHERRY_USB_DEVICE_VEDIO
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/class/video
	endif
endif #CONFIG_CHERRYUSB_DEVICE

ifdef CONFIG_CHERRY_USB_PORT_PUSB2
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/port/pusb2 \
						  $(SDK_DIR)/third-party/cherryusb/port/pusb2/fpusb2/inc

	ifdef CONFIG_USE_BAREMETAL
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/port/pusb2/standalone
	endif

	ifdef CONFIG_USE_FREERTOS
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/port/pusb2/freertos
	endif #CONFIG_USE_FREERTOS

endif #CONFIG_CHERRY_USB_PORT_PUSB2

ifdef CONFIG_CHERRY_USB_PORT_PUSB3
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/port/pusb3/fpusb3
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/fxhci

	ifdef CONFIG_USE_BAREMETAL
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/port/pusb3/standalone
	endif

	ifdef CONFIG_USE_FREERTOS
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/port/pusb3/freertos
	endif
endif #CONFIG_CHERRY_USB_PORT_PUSB2

ifdef CONFIG_CHERRY_USB_PORT_XHCI
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium

	ifdef CONFIG_USE_FREERTOS
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/freertos
	endif #CONFIG_USE_FREERTOS
endif #CONFIG_CHERRY_USB_PORT_XHCI

endif #CONFIG_USE_CHERRY_USB