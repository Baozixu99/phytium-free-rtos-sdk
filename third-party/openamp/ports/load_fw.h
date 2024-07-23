/*
 * Copyright : (C) 2024 Phytium Information Technology, Inc.
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
 * FilePath: load_fw.h
 * Created Date: 2024-05-06 14:48:29
 * Last Modified: 2024-07-10 17:05:10
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#ifndef LOAD_FW_H_
#define LOAD_FW_H_

/***************************** Include Files *********************************/
#include "platform_info.h"
#include "rsc_table.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
extern "C"
{
#endif

struct mem_file {
	const void *base;
};

int load_exectuable_block(struct remoteproc *rproc,
			  const struct image_store_ops *store_ops, void *store,
			  const char *img_path) ;
void print_resource_table(const struct remote_resource_table *table);

#ifdef __cplusplus
}
#endif


#endif
