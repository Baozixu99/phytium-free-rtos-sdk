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
 * FilePath: i2c_example.h
 * Date: 2022-07-18 14:41:23
 * LastEditTime: 2022-07-18 14:41:23
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 0.0.1 liushengming 2022.7.18 init
 */
#ifndef QSPI_EXAMPLE_H
#define QSPI_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "fi2c_os.h"
/***************************** Include Files *********************************/

/************************** Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* i2c write and read test */
BaseType_t FFreeRTOSI2cCreate(void);
/* dump buffer of slave */
void FFreeRTOSI2cSlaveDump(FFreeRTOSI2c *os_i2c_p);

#ifdef __cplusplus
}
#endif

#endif