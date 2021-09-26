#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

/* Project Configuration */

/* FT2000-4 AARCH32 FreeRTOS Configuration */

#define CONFIG_TARGET_NAME "ft2004_freertos"
/* end of FT2000-4 AARCH32 FreeRTOS Configuration */
/* end of Project Configuration */

/* Standalone Setting */

#define CONFIG_USE_FREERTOS

/* Arch Configuration */

/* CONFIG_TARGET_ARMV8_AARCH32 is not set */
#define CONFIG_TARGET_ARMV8_AARCH64
/* CONFIG_TARGET_ARMV7 is not set */
#define CONFIG_USE_CACHE
#define CONFIG_USE_L3CACHE
#define CONFIG_USE_MMU
#define CONFIG_USE_SYS_TICK
/* end of Arch Configuration */

/* Board Configuration */

#define CONFIG_TARGET_F2000_4
/* CONFIG_TARGET_E2000 is not set */
/* CONFIG_TARGET_D2000 is not set */
/* end of Board Configuration */

/* Components Configuration */

/* CONFIG_USE_SPI is not set */
/* CONFIG_USE_QSPI is not set */
#define CONFIG_USE_GIC
#define CONFIG_EBABLE_GICV3
#define CONFIG_USE_USART

/* Usart Configuration */

#define CONFIG_ENABLE_Pl011_UART
/* end of Usart Configuration */
/* CONFIG_USE_GPIO is not set */
/* CONFIG_USE_IOMUX is not set */
#define CONFIG_USE_ETH

/* Eth Configuration */

/* CONFIG_ENABLE_F_XMAC is not set */
#define CONFIG_ENABLE_F_GMAC

/* F_GMAC Configuration */

/* CONFIG_F_GMAC_PHY_COMMON is not set */
#define CONFIG_F_GMAC_PHY_AR803X
/* end of F_GMAC Configuration */
/* end of Eth Configuration */
/* CONFIG_USE_CAN is not set */
/* CONFIG_USE_I2C is not set */
/* CONFIG_USE_TIMER is not set */
/* CONFIG_USE_SDMMC is not set */
/* CONFIG_USE_PCIE is not set */
/* CONFIG_USE_WDT is not set */
/* CONFIG_USE_DMA is not set */
/* end of Components Configuration */
#define CONFIG_USE_LIBC
/* end of Standalone Setting */

/* Building Option */

#define CONFIG_ENVI_UBUNTU_20_04

/* Cross-Compiler Setting */

#define CONFIG_COMPILER_NO_STD_STARUP
/* CONFIG_USE_EXT_COMPILER is not set */
/* end of Cross-Compiler Setting */
/* CONFIG_LOG_VERBOS is not set */
/* CONFIG_LOG_DEBUG is not set */
#define CONFIG_LOG_INFO
/* CONFIG_LOG_WARN is not set */
/* CONFIG_LOG_ERROR is not set */
/* CONFIG_LOG_NONE is not set */

/* GNU Linker Setting */

/* CONFIG_AARCH32_RAM_LD is not set */
#define CONFIG_AARCH64_RAM_LD
/* CONFIG_USER_DEFINED_LD is not set */
/* end of GNU Linker Setting */
/* end of Building Option */

/* Component Configuration */

/* Freertos Drivers */

#define CONFIG_FREERTOS_USE_UART
/* end of Freertos Drivers */
/* end of Component Configuration */

/* FreeRTOS Setting */

/* CONFIG_USE_LWIP is not set */
/* end of FreeRTOS Setting */

#endif
