/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fqspi_os.h
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 16:59:58
 * Description:  This file is for providing function related definitions of qspi driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 * 1.1 wangxiaodong 2022/11/09  qspi sfud perfection
 */

#ifndef FQSPI_OS_H
#define FQSPI_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "ferror_code.h"
#include "fqspi.h"
#include "fqspi_hw.h"
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FREERTOS_QSPI_SEM_ERROR     FT_CODE_ERR(ErrModBsp, ErrBspQSpi, 10)

/**
 * QSPI message structure
 */
typedef struct
{
    const void *write_buf;  /* qspi write buffer */
    void *read_buf;         /* qspi read buffer */
    size_t length;          /* qspi read or write buffer length */
    u32 addr;               /* qspi flash address to read or write */
    u8 cmd;                 /* qspi operate command  */
    u8 cs;                  /* qspi cs channel */
} FFreeRTOSQspiMessage;

typedef struct
{
    FQspiCtrl qspi_ctrl; /* qspi Object */
    xSemaphoreHandle wr_semaphore; /*!< qspi read and write semaphore for resource sharing */
} FFreeRTOSQspi;

/* init FreeRTOS qspi instance */
FFreeRTOSQspi *FFreeRTOSQspiInit(u32 instance_id);

/* deinit FreeRTOS qspi instance */
void FFreeRTOSQspiDeinit(FFreeRTOSQspi *os_qspi_p);

/* tranfer qspi mesage */
FError FFreeRTOSQspiTransfer(FFreeRTOSQspi *os_qspi_p, FFreeRTOSQspiMessage *message);

#ifdef __cplusplus
}
#endif

#endif // !