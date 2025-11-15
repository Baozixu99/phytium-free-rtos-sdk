-include $(PROJECT_DIR)/sdkconfig

ifdef CONFIG_CHERRY_USB_PORT_XHCI
	ifdef CONFIG_TARGET_ARMV8_AARCH64
		SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/libxhci_a64.a
	else
		ifdef CONFIG_MFLOAT_ABI_HARD
			SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/libxhci_a32_hardfp.a
		endif

		ifdef CONFIG_MFLOAT_ABI_SOFTFP
			SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/xhci/phytium/libxhci_a32_softfp_crypto-neon-fp-armv8.a
		endif
	endif
endif #CONFIG_CHERRY_USB_PORT_XHCI

ifdef CONFIG_CHERRY_USB_PORT_PUSB2

	ifdef CONFIG_CHERRYUSB_HOST
		ifdef CONFIG_TARGET_ARMV8_AARCH64
			SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_hc_a64.a
		else
			ifdef CONFIG_MFLOAT_ABI_HARD
				SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_hc_a32_hardfp.a
			endif

			ifdef CONFIG_MFLOAT_ABI_SOFTFP
				SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_hc_a32_softfp_crypto-neon-fp-armv8.a
			endif
		endif
	endif #CONFIG_CHERRYUSB_HOST

	ifdef CONFIG_CHERRYUSB_DEVICE
		ifdef CONFIG_TARGET_ARMV8_AARCH64
			SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_dc_a64.a
		else
			ifdef CONFIG_MFLOAT_ABI_HARD
				SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_dc_a32_hardfp.a
			endif

			ifdef CONFIG_MFLOAT_ABI_SOFTFP
				SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/pusb2/libpusb2_dc_a32_softfp_crypto-neon-fp-armv8.a
			endif
		endif
	endif #CONFIG_CHERRYUSB_DEVICE

endif #CONFIG_CHERRY_USB_PORT_PUSB2

ifdef CONFIG_CHERRY_USB_PORT_PUSB3
	ifdef CONFIG_CHERRYUSB_HOST
		SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/pusb3/libpusb3_hc_a64.a
	endif #CONFIG_CHERRYUSB_HOST

	ifdef CONFIG_CHERRYUSB_DEVICE
		SOURCE_DEFINED_LIBS += $(SDK_DIR)/third-party/cherryusb/port/pusb3/libpusb3_dc_a64.a
	endif #CONFIG_CHERRYUSB_DEVICE
endif #CONFIG_CHERRY_USB_PORT_PUSB2