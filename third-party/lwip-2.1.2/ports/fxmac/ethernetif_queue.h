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
 * FilePath: ethernetif_queue.h
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:53
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef __ETHERNETIF_QUEUE_H_
#define __ETHERNETIF_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PQ_QUEUE_SIZE 4096

typedef struct 
{
	void *data[PQ_QUEUE_SIZE];
	int head, tail, len;
} pq_queue_t;

pq_queue_t*	xmac_pq_create_queue(void);
int xmac_pq_enqueue(pq_queue_t *q, void *p);
void* xmac_pq_dequeue(pq_queue_t *q);
int	xmac_pq_qlength(pq_queue_t *q);

#ifdef __cplusplus
}
#endif

#endif
