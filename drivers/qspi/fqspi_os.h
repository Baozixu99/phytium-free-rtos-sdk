/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: fqspi_os.h
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 16:59:58
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef DRIVERS_FQSPI_OS_H
#define DRIVERS_FQSPI_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "ferror_code.h"
#include "fqspi.h"
#include "fqspi_hw.h"
#include "ftypes.h"

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
}FFreeRTOSQspiMessage;

typedef struct
{
    FQspiCtrl qspi_ctrl; /* qspi Object */
    xSemaphoreHandle wr_semaphore; /*!< qspi read and write semaphore for resource sharing */
} FFreeRTOSQspi;

/* init freeRTOS qspi instance */
FFreeRTOSQspi *FFreeRTOSQspiInit(u32 instance_id);

/* deinit freeRTOS qspi instance */
void FFreeRTOSQspiDeinit(FFreeRTOSQspi *os_qspi_p);

/* tranfer qspi mesage */
FError FFreeRTOSQspiTransfer(FFreeRTOSQspi *os_qspi_p, FFreeRTOSQspiMessage *message);

#endif // !
