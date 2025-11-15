
ifdef CONFIG_USE_LWIP

# src code of lwip

BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/lwip-2.1.2 \
			$(SDK_DIR)/third-party/lwip-2.1.2/include \
		    $(SDK_DIR)/third-party/lwip-2.1.2/include/compat \
			$(SDK_DIR)/third-party/lwip-2.1.2/include/lwip \
			$(SDK_DIR)/third-party/lwip-2.1.2/include/lwip/apps \
			$(SDK_DIR)/third-party/lwip-2.1.2/include/lwip/priv \
			$(SDK_DIR)/third-party/lwip-2.1.2/include/lwip/prot \
			$(SDK_DIR)/third-party/lwip-2.1.2/include/netif \
			$(SDK_DIR)/third-party/lwip-2.1.2/apps/if \
			$(SDK_DIR)/third-party/lwip-2.1.2/apps/ping
	
	ifdef CONFIG_USE_LWIP_APP_TFTP # src code of tftp app
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/lwip-2.1.2/apps/tftp 
	endif #CONFIG_USE_LWIP_APP_TFTP

	ifdef CONFIG_USE_LWIP_APP_LWIPERF # src code of lwiperf app
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/lwip-2.1.2/apps/lwiperf
	endif #CONFIG_USE_LWIP_APP_LWIPERF

	ifdef CONFIG_USE_LWIP_APP_SNTP # src code of sntp app
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/lwip-2.1.2/apps/sntp
	endif #CONFIG_USE_LWIP_APP_SNTP
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/lwip-2.1.2/ports
ifdef CONFIG_USE_BAREMETAL
	
	BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/lwip-2.1.2/ports/arch 
				
	ifdef CONFIG_LWIP_FGMAC # src code of ports
		BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/lwip-2.1.2/ports/fgmac 
	endif #CONFIG_LWIP_FGMAC

	ifdef CONFIG_LWIP_FXMAC
		BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/lwip-2.1.2/ports/fxmac 
	endif

	ifdef CONFIG_LWIP_FXMAC_V2
		BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/lwip-2.1.2/ports/fxmac_v2_0
	endif

	ifdef CONFIG_LWIP_E1000E
		BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/lwip-2.1.2/ports/e1000e 
	endif	
else


endif

endif #CONFIG_USE_LWIP

