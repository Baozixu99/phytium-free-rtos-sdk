
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
 * FilePath: fsdio_port.h
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:22
 * Description:  This files is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef THIRD_PARTY_FSDIO_OS_H
#define THIRD_PARTY_FSDIO_OS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "sdmmc_host.h"

sdmmc_err_t fsdio_host_init(sdmmc_host_instance_t *const instance, const sdmmc_host_config_t* config);

sdmmc_err_t fsdio_host_deinit(sdmmc_host_instance_t *const instance);

sdmmc_err_t fsdio_host_lock(sdmmc_host_t *const host);

void fsdio_host_unlock(sdmmc_host_t *const host);

#ifdef __cplusplus
}
#endif

#endif