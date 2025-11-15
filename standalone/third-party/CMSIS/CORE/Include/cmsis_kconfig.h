#ifndef __CMSIS_KCONFIG_H
#define __CMSIS_KCONFIG_H

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_CMSIS_DSP_MATH_NEON
    #define ARM_MATH_NEON
#endif

#endif