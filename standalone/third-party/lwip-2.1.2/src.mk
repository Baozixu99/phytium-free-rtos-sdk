
ifdef CONFIG_USE_LWIP

# src code of lwip
LWIP_FILES += $(wildcard api/*.c) \
				$(wildcard core/*.c) \
				$(wildcard core/ipv4/*.c) \
				$(wildcard core/ipv6/*.c) \
				$(wildcard apps/if/*.c)   \
				$(wildcard apps/ping/*.c)
LWIP_FILES += netif/bridgeif.c \
				netif/bridgeif_fdb.c \
				netif/ethernet.c \
				netif/lowpan6.c \
				netif/lowpan6_ble.c \
				netif/lowpan6_common.c \
				netif/zepif.c



	
	ifdef CONFIG_USE_LWIP_APP_TFTP # src code of tftp app
		LWIP_FILES += $(wildcard apps/tftp/*.c)
	endif #CONFIG_USE_LWIP_APP_TFTP

	ifdef CONFIG_USE_LWIP_APP_LWIPERF # src code of lwiperf app
		LWIP_FILES += $(wildcard apps/lwiperf/*.c)
	endif #CONFIG_USE_LWIP_APP_LWIPERF

	ifdef CONFIG_USE_LWIP_APP_SNTP # src code of sntp app
	LWIP_FILES += $(wildcard apps/sntp/*.c)
	endif #CONFIG_USE_LWIP_APP_SNTP

ifdef CONFIG_USE_BAREMETAL

	LWIP_FILES += $(filter-out ports/sockets_ext.c, $(wildcard ports/*.c))

	LWIP_FILES += $(wildcard ports/arch/*.c)
		
	ifdef CONFIG_LWIP_FGMAC # src code of ports
		LWIP_FILES += $(wildcard ports/fgmac/*.c)
	endif #CONFIG_LWIP_FGMAC

	ifdef CONFIG_LWIP_FXMAC
		LWIP_FILES += $(wildcard ports/fxmac/*.c)
	endif

	ifdef CONFIG_LWIP_FXMAC_V2
		LWIP_FILES += $(wildcard ports/fxmac_v2_0/*.c)
	endif

	ifdef CONFIG_LWIP_E1000E
		LWIP_FILES += $(wildcard ports/e1000e/*.c)
	endif
else
	LWIP_FILES += $(wildcard ports/*.c)
endif

endif #CONFIG_USE_LWIP

