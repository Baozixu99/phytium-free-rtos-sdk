#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

/* Project Configuration */

#define CONFIG_TARGET_NAME "e2000q_freertos_a64"
#define CONFIG_FATFS_BASIC_TEST
/* CONFIG_FATFS_SPEED_TEST is not set */
/* CONFIG_FATFS_CYCLE_TEST is not set */
/* end of Project Configuration */

/* Standalone Setting */

#define CONFIG_USE_FREERTOS

/* Arch Configuration */

/* CONFIG_TARGET_ARMV8_AARCH32 is not set */
#define CONFIG_TARGET_ARMV8_AARCH64
#define CONFIG_USE_CACHE
#define CONFIG_USE_MMU
/* CONFIG_MMU_DEBUG_PRINTS is not set */
/* CONFIG_BOOT_WITH_FLUSH_CACHE is not set */
/* end of Arch Configuration */

/* Board Configuration */

/* CONFIG_TARGET_F2000_4 is not set */
/* CONFIG_TARGET_D2000 is not set */
#define CONFIG_TARGET_E2000Q
/* CONFIG_TARGET_E2000D is not set */
/* CONFIG_TARGET_E2000S is not set */
#define CONFIG_TARGET_E2000
#define CONFIG_DEFAULT_DEBUG_PRINT_UART1
/* CONFIG_DEFAULT_DEBUG_PRINT_UART0 is not set */
/* CONFIG_DEFAULT_DEBUG_PRINT_UART2 is not set */
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
/* CONFIG_USE_ETH is not set */
/* CONFIG_USE_CAN is not set */
/* CONFIG_USE_I2C is not set */
/* CONFIG_USE_TIMER is not set */
/* CONFIG_USE_MIO is not set */
#define CONFIG_USE_SDMMC
/* CONFIG_ENABLE_FSDMMC is not set */
#define CONFIG_ENABLE_FSDIO
/* CONFIG_USE_PCIE is not set */
/* CONFIG_USE_WDT is not set */
/* CONFIG_USE_DMA is not set */
/* CONFIG_USE_NAND is not set */
/* CONFIG_USE_RTC is not set */
/* CONFIG_USE_SATA is not set */
/* CONFIG_USE_USB is not set */
/* CONFIG_USE_ADC is not set */
/* CONFIG_USE_PWM is not set */
/* CONFIG_USE_IPC is not set */
/* CONFIG_USE_MEDIA is not set */
/* CONFIG_USE_SCMI_MHU is not set */
/* end of Components Configuration */
#define CONFIG_USE_NEW_LIBC
/* end of Standalone Setting */

/* Building Option */

/* Sdk common configuration */

/* CONFIG_LOG_VERBOS is not set */
/* CONFIG_LOG_DEBUG is not set */
/* CONFIG_LOG_INFO is not set */
/* CONFIG_LOG_WARN is not set */
#define CONFIG_LOG_ERROR
/* CONFIG_LOG_NONE is not set */
#define CONFIG_LOG_EXTRA_INFO
/* CONFIG_LOG_DISPALY_CORE_NUM is not set */
/* CONFIG_BOOTUP_DEBUG_PRINTS is not set */
#define CONFIG_USE_DEFAULT_INTERRUPT_CONFIG
#define CONFIG_INTERRUPT_ROLE_MASTER
/* CONFIG_INTERRUPT_ROLE_SLAVE is not set */
/* end of Sdk common configuration */

/* Image information configuration */

/* CONFIG_IMAGE_INFO is not set */
/* end of Image information configuration */

/* Linker Options */

/* CONFIG_AARCH32_RAM_LD is not set */
#define CONFIG_AARCH64_RAM_LD
/* CONFIG_USER_DEFINED_LD is not set */
#define CONFIG_LINK_SCRIPT_ROM
#define CONFIG_ROM_START_UP_ADDR 0x80100000
#define CONFIG_ROM_SIZE_MB 2
#define CONFIG_LINK_SCRIPT_RAM
#define CONFIG_RAM_START_UP_ADDR 0x81000000
#define CONFIG_RAM_SIZE_MB 64
#define CONFIG_HEAP_SIZE 1
#define CONFIG_STACK_SIZE 0x100000
#define CONFIG_FPU_STACK_SIZE 0x1000
/* end of Linker Options */

/* Compiler Options */

/* Cross-Compiler Setting */

#define CONFIG_GCC_OPTIMIZE_LEVEL 0
/* CONFIG_USE_EXT_COMPILER is not set */
/* CONFIG_USE_KLIN_SYS is not set */
/* end of Cross-Compiler Setting */
#define CONFIG_OUTPUT_BINARY
/* end of Compiler Options */
/* end of Building Option */

/* Component Configuration */

/* Freertos Uart Drivers */

#define CONFIG_FREERTOS_USE_UART
/* end of Freertos Uart Drivers */

/* Freertos Pwm Drivers */

/* CONFIG_FREERTOS_USE_PWM is not set */
/* end of Freertos Pwm Drivers */

/* Freertos Qspi Drivers */

/* CONFIG_FREERTOS_USE_QSPI is not set */
/* end of Freertos Qspi Drivers */

/* Freertos Wdt Drivers */

/* CONFIG_FREERTOS_USE_WDT is not set */
/* end of Freertos Wdt Drivers */

/* Freertos Eth Drivers */

/* CONFIG_FREERTOS_USE_XMAC is not set */
/* CONFIG_FREERTOS_USE_GMAC is not set */
/* end of Freertos Eth Drivers */

/* Freertos Gpio Drivers */

/* CONFIG_FREERTOS_USE_GPIO is not set */
/* end of Freertos Gpio Drivers */

/* Freertos Spim Drivers */

/* CONFIG_FREERTOS_USE_FSPIM is not set */
/* end of Freertos Spim Drivers */

/* Freertos DMA Drivers */

/* CONFIG_FREERTOS_USE_FDDMA is not set */
/* CONFIG_FREERTOS_USE_FGDMA is not set */
/* end of Freertos DMA Drivers */

/* Freertos Adc Drivers */

/* CONFIG_FREERTOS_USE_ADC is not set */
/* end of Freertos Adc Drivers */

/* Freertos Can Drivers */

/* CONFIG_FREERTOS_USE_CAN is not set */
/* end of Freertos Can Drivers */

/* Freertos I2c Drivers */

/* CONFIG_FREERTOS_USE_I2C is not set */
/* end of Freertos I2c Drivers */

/* Freertos Mio Drivers */

/* CONFIG_FREERTOS_USE_MIO is not set */
/* end of Freertos Mio Drivers */

/* Freertos Timer Drivers */

/* CONFIG_FREERTOS_USE_TIMER is not set */
/* end of Freertos Timer Drivers */

/* Freertos Media Drivers */

/* CONFIG_FREERTOS_USE_MEDIA is not set */
/* end of Freertos Media Drivers */
/* end of Component Configuration */

/* Third-Party Configuration */

/* CONFIG_USE_LWIP is not set */
#define CONFIG_USE_BACKTRACE
#define CONFIG_USE_FATFS_0_1_4

/* Fatfs configuration (0.1.4) */

#define CONFIG_FATFS_RAM_DISK

/* RAM Disk Configuration */

#define CONFIG_FATFS_RAM_DISK_BASE 0xa0000000
#define CONFIG_FATFS_RAM_DISK_SIZE_MB 500
#define CONFIG_FATFS_RAM_DISK_SECTOR_SIZE_BYTE 512
/* end of RAM Disk Configuration */
#define CONFIG_FATFS_SDMMC
#define CONFIG_FATFS_SDMMC_FSDIO_TF
#define CONFIG_FATFS_SDMMC_FSDIO_EMMC
/* CONFIG_FATFS_SDMMC_FSDMMC_TF is not set */
#define CONFIG_FATFS_SATA_DISK

/* SATA Disk Configuration */

/* CONFIG_FATFS_FSATA is not set */
/* CONFIG_FATFS_FSATA_PCIE is not set */
/* end of SATA Disk Configuration */
#define CONFIG_FATFS_USB
#define CONFIG_FATFS_VOLUME_COUNT 10
/* CONFIG_FATFS_LFN_NONE is not set */
#define CONFIG_FATFS_LFN_HEAP
/* CONFIG_FATFS_LFN_STACK is not set */
#define CONFIG_FATFS_MAX_LFN 255
#define CONFIG_FATFS_FS_LOCK 0
#define CONFIG_FATFS_TIMEOUT_MS 10000
#define CONFIG_FATFS_PER_FILE_CACHE
#define CONFIG_FATFS_ALLOC_PREFER_MEMP
#define CONFIG_FATFS_MEMP_SIZE 2
/* end of Fatfs configuration (0.1.4) */
/* CONFIG_USE_SFUD is not set */
/* CONFIG_USE_SPIFFS is not set */
/* CONFIG_USE_AMP is not set */
#define CONFIG_USE_LETTER_SHELL

/* Letter Shell Configuration */

#define CONFIG_LS_PL011_UART
#define CONFIG_DEFAULT_LETTER_SHELL_USE_UART1
/* CONFIG_DEFAULT_LETTER_SHELL_USE_UART0 is not set */
/* CONFIG_DEFAULT_LETTER_SHELL_USE_UART2 is not set */
/* end of Letter Shell Configuration */
#define CONFIG_USE_TLSF
#define CONFIG_USE_SDMMC_CMD

/* Sdmmc configuration */

/* CONFIG_SDMMC_USE_FSDMMC is not set */
#define CONFIG_SDMMC_USE_FSDIO
/* end of Sdmmc configuration */
#define CONFIG_USE_CHERRY_USB

/* CherryUSB Configuration */

#define CONFIG_CHERRY_USB_PORT_XHCI
/* CONFIG_CHERRY_USB_PORT_PHYTIUM_OTG is not set */
#define CONFIG_CHERRYUSB_HOST
/* CONFIG_CHERRYUSB_DEVICE is not set */
#define CONFIG_CHERRY_USB_HOST_HUB
#define CONFIG_CHERRY_USB_HOST_MSC
/* CONFIG_CHERRY_USB_HOST_HID is not set */
/* CONFIG_CHERRY_USB_HOST_VEDIO is not set */
/* CONFIG_CHERRY_USB_HOST_CDC is not set */
/* CONFIG_CHERRY_USB_HOST_RNDIS_WIRELESS is not set */
/* end of CherryUSB Configuration */
/* CONFIG_USE_LVGL is not set */
/* end of Third-Party Configuration */

/* Kernel Configuration */

#define CONFIG_FREERTOS_OPTIMIZED_SCHEDULER
#define CONFIG_FREERTOS_HZ 1000
#define CONFIG_FREERTOS_MAX_PRIORITIES 32
#define CONFIG_FREERTOS_KERNEL_INTERRUPT_PRIORITIES 13
#define CONFIG_FREERTOS_MAX_API_CALL_INTERRUPT_PRIORITIES 11
#define CONFIG_FREERTOS_THREAD_LOCAL_STORAGE_POINTERS 1
#define CONFIG_FREERTOS_MINIMAL_TASK_STACKSIZE 1024
#define CONFIG_FREERTOS_MAX_TASK_NAME_LEN 32
#define CONFIG_FREERTOS_TIMER_TASK_PRIORITY 1
#define CONFIG_FREERTOS_TIMER_TASK_STACK_DEPTH 2048
#define CONFIG_FREERTOS_TIMER_QUEUE_LENGTH 10
#define CONFIG_FREERTOS_QUEUE_REGISTRY_SIZE 0
#define CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
#define CONFIG_FREERTOS_USE_TRACE_FACILITY
#define CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
/* CONFIG_FREERTOS_USE_TICKLESS_IDLE is not set */
#define CONFIG_FREERTOS_TOTAL_HEAP_SIZE 10240
#define CONFIG_FREERTOS_TASK_FPU_SUPPORT 1
/* end of Kernel Configuration */

#endif
