#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

/* Project Configuration */

#define CONFIG_TARGET_NAME "d2000_freertos"
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
/* CONFIG_USE_SYS_TICK is not set */
/* CONFIG_MMU_DEBUG_PRINTS is not set */
/* end of Arch Configuration */

/* Board Configuration */

/* CONFIG_TARGET_F2000_4 is not set */
#define CONFIG_TARGET_D2000
/* CONFIG_TARGET_E2000Q is not set */
/* CONFIG_TARGET_E2000D is not set */
/* CONFIG_TARGET_E2000S is not set */
/* end of Board Configuration */

/* Components Configuration */

/* CONFIG_USE_SPI is not set */
/* CONFIG_USE_QSPI is not set */
#define CONFIG_USE_GIC
#define CONFIG_ENABLE_GICV3
#define CONFIG_USE_SERIAL

/* Usart Configuration */

#define CONFIG_ENABLE_Pl011_UART
/* end of Usart Configuration */
/* CONFIG_USE_GPIO is not set */
/* CONFIG_USE_IOMUX is not set */
#define CONFIG_USE_ETH

/* Eth Configuration */

/* CONFIG_ENABLE_FXMAC is not set */
#define CONFIG_ENABLE_FGMAC
#define CONFIG_FGMAC_PHY_COMMON
/* CONFIG_FGMAC_PHY_AR803X is not set */
/* end of Eth Configuration */
/* CONFIG_USE_CAN is not set */
/* CONFIG_USE_I2C is not set */
/* CONFIG_USE_TIMER is not set */
/* CONFIG_USE_SDMMC is not set */
/* CONFIG_USE_PCIE is not set */
/* CONFIG_USE_WDT is not set */
/* CONFIG_USE_DMA is not set */
/* CONFIG_USE_NAND is not set */
/* CONFIG_USE_RTC is not set */
/* CONFIG_USE_SATA is not set */
/* CONFIG_USE_USB is not set */
/* end of Components Configuration */
#define CONFIG_USE_NEW_LIBC
/* end of Standalone Setting */

/* Building Option */

/* CONFIG_LOG_VERBOS is not set */
/* CONFIG_LOG_DEBUG is not set */
#define CONFIG_LOG_INFO
/* CONFIG_LOG_WARN is not set */
/* CONFIG_LOG_ERROR is not set */
/* CONFIG_LOG_NONE is not set */

/* Linker Options */

/* CONFIG_AARCH32_RAM_LD is not set */
#define CONFIG_AARCH64_RAM_LD
/* CONFIG_USER_DEFINED_LD is not set */
#define CONFIG_LINK_SCRIPT_ROM
#define CONFIG_ROM_START_UP_ADDR 0x80100000
#define CONFIG_ROM_SIZE_MB 1
#define CONFIG_LINK_SCRIPT_RAM
#define CONFIG_RAM_START_UP_ADDR 0x81000000
#define CONFIG_RAM_SIZE_MB 64
#define CONFIG_HEAP_SIZE 1
#define CONFIG_STACK_SIZE 0x100000
/* end of Linker Options */

/* Compiler Options */

#define CONFIG_DON_T_BINARY_OUTPUT
/* end of Compiler Options */
/* end of Building Option */

/* Component Configuration */

/* Freertos Drivers */

#define CONFIG_FREERTOS_USE_UART
/* end of Freertos Drivers */
/* end of Component Configuration */

/* FreeRTOS Setting */

/* CONFIG_USE_LWIP is not set */
#define CONFIG_USE_BACKTRACE
/* CONFIG_USE_AMP is not set */
/* CONFIG_USE_LETTER_SHELL is not set */
/* end of FreeRTOS Setting */

#endif
