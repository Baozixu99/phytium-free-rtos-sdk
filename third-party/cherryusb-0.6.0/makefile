
CHERRY_USB_DIR :=  $(THIRD_PARTY_CUR_DIR)/cherryusb-0.6.0

INC_DIR +=  $(CHERRY_USB_DIR)/core \
			$(CHERRY_USB_DIR)/common \
			$(CHERRY_USB_DIR)/osal
			
SRC_DIR +=  $(CHERRY_USB_DIR)/core \
			$(CHERRY_USB_DIR)/common \
			$(CHERRY_USB_DIR)/osal

ifdef CONFIG_CHERRY_USB_PORT_XHCI
	INC_DIR += $(CHERRY_USB_DIR)/port/xhci
	SRC_DIR += $(CHERRY_USB_DIR)/port/xhci
endif #CONFIG_CHERRY_USB_PORT_XHCI

ifdef CONFIG_CHERRYUSB_HOST
	EXCL_SRC += $(CHERRY_USB_DIR)/core/usbd_core.c

	ifdef CONFIG_CHERRY_USB_HOST_HUB
		EXCL_SRC += $(CHERRY_USB_DIR)/class/hub/usbd_hub.c

		INC_DIR += $(CHERRY_USB_DIR)/class/hub
		SRC_DIR += $(CHERRY_USB_DIR)/class/hub
	endif #CONFIG_CHERRY_USB_HOST_HUB

	ifdef CONFIG_CHERRY_USB_HOST_MSC
		EXCL_SRC += $(CHERRY_USB_DIR)/class/msc/usbd_msc.c
		
		INC_DIR += $(CHERRY_USB_DIR)/class/msc
		SRC_DIR += $(CHERRY_USB_DIR)/class/msc
	endif #CONFIG_CHERRY_USB_HOST_MSC

	ifdef CONFIG_CHERRY_USB_HOST_HID
		EXCL_SRC += $(CHERRY_USB_DIR)/class/hid/usbd_hid.c

		INC_DIR += $(CHERRY_USB_DIR)/class/hid
		SRC_DIR += $(CHERRY_USB_DIR)/class/hid
	endif #CONFIG_CHERRY_USB_HOST_HID

	ifdef CONFIG_CHERRY_USB_HOST_VEDIO
		EXCL_SRC += $(CHERRY_USB_DIR)/class/vedio/usbd_vedio.c

		INC_DIR += $(CHERRY_USB_DIR)/class/vedio
		SRC_DIR += $(CHERRY_USB_DIR)/class/vedio
	endif #CONFIG_CHERRY_USB_HOST_VEDIO

	ifdef CONFIG_CHERRY_USB_HOST_RNDIS_WIRELESS
		EXCL_SRC += $(CHERRY_USB_DIR)/class/wireless/usbd_rndis.c

		INC_DIR += $(CHERRY_USB_DIR)/class/wireless
		SRC_DIR += $(CHERRY_USB_DIR)/class/wireless
	endif #CONFIG_CHERRY_USB_HOST_RNDIS_WIRELESS

	ifdef CONFIG_CHERRY_USB_HOST_CDC
		EXCL_SRC += $(CHERRY_USB_DIR)/class/cdc/usbd_cdc.c

		INC_DIR += $(CHERRY_USB_DIR)/class/cdc
		SRC_DIR += $(CHERRY_USB_DIR)/class/cdc
	endif #CONFIG_CHERRY_USB_HOST_CDC

endif #CONFIG_CHERRYUSB_HOST

ifdef CONFIG_CHERRYUSB_DEVICE
	EXCL_SRC += $(CHERRY_USB_DIR)/core/usbh_core.c
endif #CONFIG_CHERRYUSB_DEVICE
