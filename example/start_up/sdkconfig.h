#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

/* Project Configuration */

/* FT2000-4 AARCH32 FreeRTOS Configuration */

#define CONFIG_TARGET_NAME "d2000_freertos"
/* CONFIG_DEMO_HELLO_WORLD is not set */
/* CONFIG_DEMO_GET_CPU_STATS is not set */
#define CONFIG_DEMO_MSG_QUEUE
/* CONFIG_DEMO_SEMAPHORE is not set */
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
/* CONFIG_USE_SYS_TICK is not set */
/* end of Arch Configuration */

/* Board Configuration */

/* CONFIG_TARGET_F2000_4 is not set */
/* CONFIG_TARGET_E2000 is not set */
#define CONFIG_TARGET_D2000
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
/* end of Components Configuration */
#define CONFIG_USE_G_LIBC
/* CONFIG_USE_NEW_LIBC is not set */
/* end of Standalone Setting */

/* Building Option */

/* Cross-Compiler Setting */

#define CONFIG_COMPILER_NO_STD_STARUP
#define CONFIG_GCC_OPTIMIZE_LEVEL 0
/* CONFIG_USE_EXT_COMPILER is not set */
/* end of Cross-Compiler Setting */
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
#define CONFIG_STACK_SIZE 0x400
/* end of Linker Options */
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
