#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

/* Project Configuration */

/*  Freertos Configuration */

#define CONFIG_TARGET_NAME "openamp_core0"
#define CONFIG_SHM_BASE_ADDR 0xc0000000
/* end of  Freertos Configuration */

/* AMP Config */

#define CONFIG_IPI_IRQ_NUM 9
#define CONFIG_IPI_IRQ_NUM_PRIORITY 16
#define CONFIG_SPIN_MEM 0x80000000
#define CONFIG_TARGET_CPU_ID 2
#define CONFIG_IPI_CHN_BITMASK 255
#define CONFIG_DEBUG_CODE
/* end of AMP Config */
/* end of Project Configuration */

/* Standalone Setting */

#define CONFIG_USE_FREERTOS

/* Arch Configuration */

#define CONFIG_TARGET_ARMV8_AARCH32
/* CONFIG_TARGET_ARMV8_AARCH64 is not set */
/* CONFIG_TARGET_ARMV7 is not set */
#define CONFIG_USE_CACHE
/* CONFIG_USE_L3CACHE is not set */
#define CONFIG_USE_MMU
/* CONFIG_USE_SYS_TICK is not set */
#define CONFIG_USE_AARCH64_L1_TO_AARCH32
/* end of Arch Configuration */

/* Board Configuration */

/* CONFIG_TARGET_F2000_4 is not set */
#define CONFIG_TARGET_F2000_4
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
/* CONFIG_USE_ETH is not set */
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

/* Linker Options */

#define CONFIG_AARCH32_RAM_LD
/* CONFIG_AARCH64_RAM_LD is not set */
/* CONFIG_USER_DEFINED_LD is not set */
#define CONFIG_LINK_SCRIPT_ROM
#define CONFIG_ROM_START_UP_ADDR 0xb0100000
#define CONFIG_ROM_SIZE_MB 2
#define CONFIG_LINK_SCRIPT_RAM
#define CONFIG_RAM_START_UP_ADDR 0xb0300000
#define CONFIG_RAM_SIZE_MB 64
#define CONFIG_HEAP_SIZE 1
#define CONFIG_SVC_STACK_SIZE 0x1000
#define CONFIG_SYS_STACK_SIZE 0x1000
#define CONFIG_IRQ_STACK_SIZE 0x1000
#define CONFIG_ABORT_STACK_SIZE 0x1000
#define CONFIG_FIQ_STACK_SIZE 0x1000
#define CONFIG_UNDEF_STACK_SIZE 0x1000
/* end of Linker Options */

/* Compiler Options */

#define CONFIG_DON_T_BINARY_OUTPUT
/* end of Compiler Options */
/* CONFIG_LOG_VERBOS is not set */
/* CONFIG_LOG_DEBUG is not set */
/* CONFIG_LOG_INFO is not set */
/* CONFIG_LOG_WARN is not set */
#define CONFIG_LOG_ERROR
/* CONFIG_LOG_NONE is not set */
/* end of Building Option */

/* Component Configuration */

/* Freertos Drivers */

#define CONFIG_FREERTOS_USE_UART
/* end of Freertos Drivers */
/* end of Component Configuration */

/* FreeRTOS Setting */

/* CONFIG_USE_LWIP is not set */
#define CONFIG_USE_BACKTRACE
#define CONFIG_USE_AMP
#define CONFIG_USE_LIBMETAL

/* OpenAmp */

#define CONFIG_USE_OPENAMP
#define CONFIG_USE_OPENAMP_IPI
#define CONFIG_OPENAMP_RESOURCES_ADDR 0xc0000000
#define CONFIG_VRING_TX_ADDR 0xc0020000
#define CONFIG_VRING_RX_ADDR 0xc0024000
#define CONFIG_VRING_SIZE 0x100
#define CONFIG_POLL_BASE_ADDR 0xc0224000
/* CONFIG_SKIP_SHBUF_IO_WRITE is not set */
/* CONFIG_USE_MASTER_VRING_DEFINE is not set */

/* Baremetal config */

#define CONFIG_MEM_NO_CACHE
/* CONFIG_MEM_WRITE_THROUGH is not set */
/* CONFIG_MEM_NORMAL is not set */
/* end of Baremetal config */
/* end of OpenAmp */
#define CONFIG_USE_LETTER_SHELL

/* Letter Shell Configuration */

#define CONFIG_LS_PL011_UART
/* end of Letter Shell Configuration */
/* end of FreeRTOS Setting */

#endif
