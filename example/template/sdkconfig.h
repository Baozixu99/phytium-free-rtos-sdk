#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

/* Project Configuration */

/* Template Configuration */

#define CONFIG_TARGET_NAME "template"
/* end of Template Configuration */
/* end of Project Configuration */

/* Standalone Setting */

#define CONFIG_USE_FREERTOS

/* Arch Configuration */

#define CONFIG_TARGET_ARMV8_AARCH32
/* CONFIG_TARGET_ARMV8_AARCH64 is not set */
/* CONFIG_TARGET_ARMV7 is not set */
/* CONFIG_USE_CACHE is not set */
/* end of Arch Configuration */

/* Board Configuration */

#define CONFIG_TARGET_F2000_4
/* CONFIG_TARGET_E2000 is not set */
/* end of Board Configuration */

/* Components Configuration */

/* CONFIG_USE_SPI is not set */
/* CONFIG_USE_QSPI is not set */
#define CONFIG_USE_GIC
#define CONFIG_EBABLE_GICV3
/* CONFIG_USE_USART is not set */
/* CONFIG_USE_GPIO is not set */
/* CONFIG_USE_IOMUX is not set */
/* end of Components Configuration */
#define CONFIG_USE_NOSTD_LIBC
/* CONFIG_USE_LIBC is not set */
/* end of Standalone Setting */

/* Building Option */

#define CONFIG_ENVI_UBUNTU_20_04

/* Cross-Compiler Setting */

#define CONFIG_COMPILER_NO_STD_STARUP
/* CONFIG_USE_EXT_COMPILER is not set */
/* end of Cross-Compiler Setting */
/* CONFIG_LOG_VERBOS is not set */
/* CONFIG_LOG_DEBUG is not set */
/* CONFIG_LOG_INFO is not set */
/* CONFIG_LOG_WARN is not set */
#define CONFIG_LOG_ERROR
/* CONFIG_LOG_NONE is not set */

/* GNU Linker Setting */

#define CONFIG_E2000_FT2004_AARCH32_RAM_LD
/* CONFIG_E2000_FT20004_AARCH64_RAM_LD is not set */
/* CONFIG_USER_DEFINED_LD is not set */
/* end of GNU Linker Setting */
/* end of Building Option */

/* Component Configuration */

/* Freertos Drivers */

#define CONFIG_FREERTOS_USE_UART
/* end of Freertos Drivers */
/* end of Component Configuration */

/* FreeRTOS Setting */


#endif
