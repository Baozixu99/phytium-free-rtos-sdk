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
 * FilePath: sd_read_write.h
 * Date: 2022-07-18 16:43:35
 * LastEditTime: 2022-07-18 16:43:35
 * Description:  This files is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#ifndef  EXAMPLE_SD_READ_WRITE_H
#define  EXAMPLE_SD_READ_WRITE_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
BaseType_t FFreeRTOSSdWriteRead(u32 slot_id, boolean is_emmc, u32 start_blk, u32 blk_num);

#ifdef __cplusplus
}
#endif

#endif