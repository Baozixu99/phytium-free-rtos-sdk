ifdef CONFIG_USE_FREERTOS
	CSRCS_RELATIVE_FILES += osal/usb_osal_freertos.c
endif #CONFIG_USE_FREERTOS

ifdef CONFIG_CHERRYUSB_HOST
	CSRCS_RELATIVE_FILES += core/usbh_core.c

	ifdef CONFIG_CHERRY_USB_HOST_HUB
		CSRCS_RELATIVE_FILES += class/hub/usbh_hub.c
	endif #CONFIG_CHERRY_USB_HOST_HUB

	ifdef CONFIG_CHERRY_USB_HOST_MSC
		CSRCS_RELATIVE_FILES += class/msc/usbh_msc.c
	endif #CONFIG_CHERRY_USB_HOST_MSC

	ifdef CONFIG_CHERRY_USB_HOST_HID
		CSRCS_RELATIVE_FILES += class/hid/usbh_hid.c
	endif #CONFIG_CHERRY_USB_HOST_HID

	ifdef CONFIG_CHERRY_USB_PORT_XHCI
		ifdef CONFIG_USE_FREERTOS
			CSRCS_RELATIVE_FILES += port/xhci/phytium/freertos/usb_glue_phytium.c

			ifdef CONFIG_CHERRY_USB_PORT_XHCI_PLATFROM
				CSRCS_RELATIVE_FILES += port/xhci/phytium/freertos/usb_glue_phytium_plat.c
			endif #CONFIG_CHERRY_USB_PORT_XHCI_PLATFROM

			ifdef CONFIG_CHERRY_USB_PORT_XHCI_PCIE
				CSRCS_RELATIVE_FILES += port/xhci/phytium/freertos/usb_glue_phytium_pcie.c
			endif #CONFIG_CHERRY_USB_PORT_XHCI_PCIE
		endif #CONFIG_USE_FREERTOS
	endif #CONFIG_CHERRY_USB_PORT_XHCI

	ifdef CONFIG_CHERRY_USB_PORT_PUSB2
		ifdef CONFIG_USE_FREERTOS
			CSRCS_RELATIVE_FILES += port/pusb2/freertos/usb_hc_glue_phytium.c
		endif #CONFIG_USE_FREERTOS
	endif #CONFIG_CHERRY_USB_PORT_PUSB2

	ifdef CONFIG_CHERRY_USB_PORT_PUSB3
		ifdef CONFIG_USE_FREERTOS
			CSRCS_RELATIVE_FILES += port/pusb3/freertos/usb_glue_phytium.c \
									port/pusb3/freertos/usb_glue_hc_phytium.c
		endif
	endif #CONFIG_CHERRY_USB_PORT_PUSB3

endif #CONFIG_CHERRYUSB_HOST

ifdef CONFIG_CHERRYUSB_DEVICE
	CSRCS_RELATIVE_FILES += core/usbd_core.c

	ifdef CONFIG_CHERRY_USB_DEVICE_MSC
		CSRCS_RELATIVE_FILES += class/msc/usbd_msc.c
	endif #CONFIG_CHERRY_USB_DEVICE_MSC

	ifdef CONFIG_CHERRY_USB_DEVICE_CDC
		CSRCS_RELATIVE_FILES += class/cdc/usbd_cdc_ecm.c \
								class/cdc/usbd_cdc_acm.c
	endif #CONFIG_CHERRY_USB_DEVICE_CDC

	ifdef CONFIG_CHERRY_USB_DEVICE_HID
		CSRCS_RELATIVE_FILES += class/hid/usbd_hid.c
	endif #CONFIG_CHERRY_USB_DEVICE_HID

	ifdef CONFIG_CHERRY_USB_DEVICE_VEDIO
		CSRCS_RELATIVE_FILES += class/video/usbd_video.c
	endif #CONFIG_CHERRY_USB_DEVICE_VEDIO

	ifdef CONFIG_CHERRY_USB_PORT_PUSB2
		ifdef CONFIG_USE_BAREMETAL
			CSRCS_RELATIVE_FILES += port/pusb2/standalone/usb_dc_glue_phytium.c
		endif #CONFIG_USE_BAREMETAL
		ifdef CONFIG_USE_FREERTOS
			CSRCS_RELATIVE_FILES += port/pusb2/freertos/usb_dc_glue_phytium.c
		endif #CONFIG_USE_FREERTOS

	endif #CONFIG_CHERRY_USB_PORT_PUSB2

	ifdef CONFIG_CHERRY_USB_PORT_PUSB3
		ifdef CONFIG_USE_BAREMETAL
			CSRCS_RELATIVE_FILES += port/pusb3/standalone/usb_glue_phytium.c
		endif #CONFIG_USE_BAREMETAL
		ifdef CONFIG_USE_FREERTOS
			CSRCS_RELATIVE_FILES += port/pusb3/freertos/usb_glue_phytium.c \
									port/pusb3/freertos/usb_glue_dc_phytium.c
		endif #CONFIG_USE_FREERTOS
	endif #CONFIG_CHERRY_USB_PORT_PUSB3
endif #CONFIG_CHERRYUSB_DEVICE

ifdef CONFIG_CHERRY_USB_PORT_SOURCE_CODE
include $(SDK_DIR)/third-party/cherryusb/src_port.mk
endif