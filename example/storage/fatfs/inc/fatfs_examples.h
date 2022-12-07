/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * FilePath: atfs_examples.h
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */

#ifndef FATFS_EXAMPLES_H
#define FATFS_EXAMPLES_H


enum
{
    FFREERTOS_FATFS_RAM_DISK =   0U,
    FFREERTOS_FATFS_TF_CARD =    1U,
    FFREERTOS_FATFS_EMMC_CARD =  2U,
    FFREERTOS_FATFS_USB_DISK =   3U,

    FFREERTOS_DISK_TYPE_NUM,
};

/* fatfs run */
BaseType_t FFreeRTOSFatfsTest(void);

#endif // !