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
 * FilePath: ethernetif_queue.c
 * Date: 2022-04-02 16:43:32
 * LastEditTime: 2022-04-19 21:27:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver Who      Date        Changes
 * ----- ------     --------    --------------------------------------
 */

#include <stdlib.h>
#include "ethernetif_queue.h"
#include "f_printk.h"

#define NUM_QUEUES	2

pq_queue_t pq_queue[NUM_QUEUES];

pq_queue_t *xmac_pq_create_queue()
{
	static int i;
	pq_queue_t *q = NULL;

	if (i >= NUM_QUEUES) 
	{
		f_printk("ERR: Max Queues allocated\n\r");
		return q;
	}

	q = &pq_queue[i++];

	if (!q)
		return q;

	q->head = q->tail = q->len = 0;

	return q;
}

int xmac_pq_enqueue(pq_queue_t *q, void *p)
{
	if (q->len == PQ_QUEUE_SIZE)
		return -1;

	q->data[q->head] = p;
	q->head = (q->head + 1)%PQ_QUEUE_SIZE;
	q->len++;

	return 0;
}

void* xmac_pq_dequeue(pq_queue_t *q)
{
	int ptail;

	if (q->len == 0)
		return NULL;

	ptail = q->tail;
	q->tail = (q->tail + 1)%PQ_QUEUE_SIZE;
	q->len--;

	return q->data[ptail];
}

int xmac_pq_qlength(pq_queue_t *q)
{
	return q->len;
}
