/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * 
 * FilePath: fi2s_os.h
 * Created Date: 2024-02-29 10:49:34
 * Last Modified: 2024-05-14 09:16:40
 * Description:  This file is for providing function related definitions of i2s driver
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *   1.0     Wangzq   2024/03/07        init
 */


#ifndef FI2S_OS_H
#define FI2S_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "fi2s.h"
#include "ftypes.h"
#include "fparameters.h"
#include "event_groups.h"

#ifdef __cplusplus
extern "C"
{
#endif
/************************** Constant Definitions *****************************/
#define FFREERTOS_I2S_SUCCESS                   FT_SUCCESS
/*Error code depend on OS standard*/
#define FREERTOS_I2S_TASK_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2s, 0x1)
#define FREERTOS_I2S_MESG_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2s, 0x2)
#define FREERTOS_I2S_MEMY_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2s, 0x3)

typedef struct
{
    FI2s i2s_ctrl;
} FFreeRTOSI2s;

/*init the i2s and return the i2s instance*/
FFreeRTOSI2s *FFreeRTOSI2sInit(u32 id);

/*setup the i2s */
FError FFreeRTOSSetupI2S(FFreeRTOSI2s *os_i2s_p);

/*deinit the i2s */
FError FFreeRTOSI2SDeinit(FFreeRTOSI2s *os_i2s_p);

#ifdef __cplusplus
}
#endif

#endif
