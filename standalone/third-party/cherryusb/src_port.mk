ifdef CONFIG_CHERRYUSB_HOST

ifdef CONFIG_CHERRY_USB_PORT_XHCI
	CSRCS_RELATIVE_FILES += port/xhci/phytium/fxhci/xhci.c \
							port/xhci/phytium/fxhci/xhci_drv.c \
							port/xhci/phytium/fxhci/usb_hc_xhci.c
endif #CONFIG_CHERRY_USB_PORT_XHCI

ifdef CONFIG_CHERRY_USB_PORT_PUSB2
	CSRCS_RELATIVE_FILES += port/pusb2/fpusb2/usb_hc_pusb2.c \
							port/pusb2/fpusb2/cps.c \
							port/pusb2/fpusb2/dc.c \
							port/pusb2/fpusb2/hc.c \
							port/pusb2/fpusb2/vhub.c \
							port/pusb2/fpusb2/gdma.c
endif #CONFIG_CHERRY_USB_PORT_PUSB2

ifdef CONFIG_CHERRY_USB_PORT_PUSB3
	CSRCS_RELATIVE_FILES += port/xhci/phytium/fxhci/xhci.c \
							port/xhci/phytium/fxhci/xhci_drv.c \
							port/pusb3/fpusb3/usb_hc_pusb3.c \
							port/pusb3/fpusb3/core.c \
							port/pusb3/fpusb3/otg.c \
							port/pusb3/fpusb3/host.c
endif #CONFIG_CHERRY_USB_PORT_PUSB3

endif #CONFIG_CHERRYUSB_HOST

ifdef CONFIG_CHERRYUSB_DEVICE

ifdef CONFIG_CHERRY_USB_PORT_PUSB2
	CSRCS_RELATIVE_FILES += port/pusb2/fpusb2/usb_dc_pusb2.c \
							port/pusb2/fpusb2/cps.c \
							port/pusb2/fpusb2/dc.c \
							port/pusb2/fpusb2/hc.c \
							port/pusb2/fpusb2/vhub.c \
							port/pusb2/fpusb2/gdma.c
endif #CONFIG_CHERRY_USB_PORT_PUSB2

ifdef CONFIG_CHERRY_USB_PORT_PUSB3
	CSRCS_RELATIVE_FILES += port/pusb3/fpusb3/usb_dc_pusb3.c \
							port/pusb3/fpusb3/art.c \
							port/pusb3/fpusb3/debug.c \
							port/pusb3/fpusb3/core.c \
							port/pusb3/fpusb3/mem.c \
							port/pusb3/fpusb3/ring.c \
							port/pusb3/fpusb3/otg.c \
							port/pusb3/fpusb3/gadget.c \
							port/pusb3/fpusb3/host.c
endif #CONFIG_CHERRY_USB_PORT_PUSB3

endif #CONFIG_CHERRYUSB_DEVICE