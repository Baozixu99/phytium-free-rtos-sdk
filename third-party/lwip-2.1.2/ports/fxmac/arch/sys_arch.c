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
 * FilePath: sys_arch.c
 * Date: 2022-07-18 13:27:02
 * LastEditTime: 2022-07-18 13:27:02
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */
/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"

#if !NO_SYS

#include "FreeRTOS.h"
#include "task.h"

#if defined(LWIP_PROVIDE_ERRNO)
int errno;
#endif

extern int isr_calling_flg ;

/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
  UBaseType_t archMessageLength = size;

  if (size > archMAX_MESG_QUEUE_LENGTH)
    archMessageLength = archMAX_MESG_QUEUE_LENGTH;

  *mbox = xQueueCreate(archMessageLength, sizeof(void *));

  if (*mbox == NULL)
    return ERR_MEM;

  return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
  UBaseType_t uxReturn;

  uxReturn = uxQueueMessagesWaiting(*mbox);
  if (uxReturn)
  {
    /* Line for breakpoint.  Should never break here! */
    portNOP();
#if SYS_STATS
    lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
    LWIP_ASSERT("sys_mbox_free err uxQueueMessagesWaiting.\r\n ", uxReturn == pdTRUE);
  }

  vQueueDelete(*mbox);
#if SYS_STATS
  --lwip_stats.sys.mbox.used;
#endif /* SYS_STATS */
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void sys_mbox_post(sys_mbox_t *mbox, void *data)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  if (*mbox == NULL)
    return;

  if(isr_calling_flg != 0)
  {
		xQueueSendToBackFromISR( *mbox, &data, &xHigherPriorityTaskWoken );
		if (xHigherPriorityTaskWoken == pdTRUE) {
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
  }
  else
  {
    xQueueSendToBack(*mbox, &data, portMAX_DELAY);
  }
}

/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
  err_t result;
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  if (*mbox == NULL)
    return ERR_MEM;

  if(isr_calling_flg != 0)
  {
    result = xQueueSendFromISR( *mbox, &msg, &xHigherPriorityTaskWoken );
    if(xHigherPriorityTaskWoken == pdTRUE)
    {
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
  else
  {
    result = xQueueSend( *mbox, &msg, ( portTickType ) 0 );
  }

  if( result == pdPASS )
	{
		result = ERR_OK;
	}
	else
	{
		printf("Queue is full\r\n");
		/* The queue was already full. */
		result = ERR_MEM;
  #if SYS_STATS
      lwip_stats.sys.mbox.err++;
  #endif /* SYS_STATS */
	}

  return result;
}

/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
  err_t result;
  BaseType_t xHigherPriorityTaskWoken;

  if (*mbox == NULL)
    return ERR_MEM;

  if (xQueueSendFromISR(*mbox, &msg, &xHigherPriorityTaskWoken) == pdTRUE)
  {
    result = ERR_OK;
  }
  else
  {
    // could not post, queue must be full
    result = ERR_MEM;
  }

  // Actual macro used here is port specific.
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

  return result;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP.
*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
  void *dummyptr;

  BaseType_t err;

  portTickType xStartTime, xEndTime, xElapsed;
  unsigned long ulReturn;
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  xStartTime = xTaskGetTickCount();

  if (*mbox == NULL)
  {
    return SYS_ARCH_TIMEOUT;
  }
    
  if (msg == NULL)
  {
    msg = &dummyptr;
  }

  if (timeout != 0)
  {

    if(isr_calling_flg != 0)
    {
      if( pdTRUE == xQueueReceiveFromISR( *mbox, &( *msg ), &xHigherPriorityTaskWoken ) )
			{
				xEndTime = xTaskGetTickCount();
				xElapsed = ( xEndTime - xStartTime ) * portTICK_RATE_MS;
				ulReturn = xElapsed;
				if (xHigherPriorityTaskWoken == pdTRUE)
				{
					portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
				}
			}
			else
			{
				*msg = NULL;
				ulReturn = SYS_ARCH_TIMEOUT;
			}
    }
    else
    {
		if( pdTRUE == xQueueReceive( *mbox, &( *msg ), timeout / portTICK_RATE_MS ) )
		{
			xEndTime = xTaskGetTickCount();
			xElapsed = ( xEndTime - xStartTime ) * portTICK_RATE_MS;

			ulReturn = xElapsed;
		}
		else
		{
			/* Timed out. */
			*msg = NULL;
			ulReturn = SYS_ARCH_TIMEOUT;
		}
    }

  }
  else // block forever for a message.
  {

	if(isr_calling_flg != 0)
    {
		xQueueReceiveFromISR(*mbox, &(*msg), &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken == pdTRUE)
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
    }
	else
	{
		xQueueReceive(*mbox, &(*msg), portMAX_DELAY);
	}

	xEndTime = xTaskGetTickCount();
	xElapsed = ( xEndTime - xStartTime ) * portTICK_RATE_MS;

	if( xElapsed == 0UL )
	{
		xElapsed = 1UL;
	}

	ulReturn = xElapsed;

    
    // err = xQueueReceive(*mbox, &(*msg), portMAX_DELAY);

    // if (pdTRUE != err)
    // {
    //   LWIP_ASSERT("sys_arch_mbox_fetch xQueueReceive returned with error!", err == pdTRUE);
    // }

    // EndTime = xTaskGetTickCount();
    // Elapsed = (EndTime - StartTime) * portTICK_RATE_MS;

    // return (Elapsed); // return time blocked TODO test
  }
  return ulReturn;
}

/*-----------------------------------------------------------------------------------*/
/*
  Similar to sys_arch_mbox_fetch, but if message is not ready immediately, we'll
  return with SYS_MBOX_EMPTY.  On success, 0 is returned.
*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	void *dummyptr;
	long lResult;
	unsigned long ulReturn;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if (*mbox == NULL)
	{
		return SYS_ARCH_TIMEOUT;
	}
	
	if (msg == NULL)
	{
		msg = &dummyptr;
	}

	if(isr_calling_flg != 0)
	{
		lResult = xQueueReceiveFromISR( *mbox, &( *msg ), &xHigherPriorityTaskWoken );
		if (xHigherPriorityTaskWoken == pdTRUE) 
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
	else
	{
		lResult = xQueueReceive( *mbox, &( *msg ), 0UL );
	}

	if( lResult == pdPASS )
	{
		ulReturn = ERR_OK;
	}
	else
	{
		ulReturn = SYS_MBOX_EMPTY;
	}

	return ulReturn;

//   if (pdTRUE == xQueueReceive(*mbox, &(*msg), 0))
//   {
//     return ERR_OK;
//   }
//   else
//   {
//     return SYS_MBOX_EMPTY;
//   }
}
/*----------------------------------------------------------------------------------*/
int sys_mbox_valid(sys_mbox_t *mbox)
{
  if (*mbox == SYS_MBOX_NULL)
    return 0;
  else
    return 1;
}
/*-----------------------------------------------------------------------------------*/
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
  *mbox = SYS_MBOX_NULL;
}

/*-----------------------------------------------------------------------------------*/
//  Creates a new semaphore. The "count" argument specifies
//  the initial state of the semaphore.
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
  vSemaphoreCreateBinary(*sem);

  if (*sem == NULL)
  {
#if SYS_STATS
    ++lwip_stats.sys.sem.err;
#endif /* SYS_STATS */
    return ERR_MEM;
  }

  if (count == 0) // Means it can't be taken
  {
    xSemaphoreTake(*sem, 1);
  }

#if SYS_STATS
  ++lwip_stats.sys.sem.used;
  if (lwip_stats.sys.sem.max < lwip_stats.sys.sem.used)
  {
    lwip_stats.sys.sem.max = lwip_stats.sys.sem.used;
  }
#endif /* SYS_STATS */

  return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  portTickType xStartTime, xEndTime, xElapsed;
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  BaseType_t err;
  unsigned long ulReturn;

  xStartTime = xTaskGetTickCount();

  if (*sem == NULL)
    return SYS_ARCH_TIMEOUT;

  if (timeout != 0)
  {

	if(isr_calling_flg != 0)
	{
		if( xSemaphoreTakeFromISR( *sem, &xHigherPriorityTaskWoken ) == pdTRUE )
		{
			xEndTime = xTaskGetTickCount();
			xElapsed = (xEndTime - xStartTime) * portTICK_RATE_MS;
			ulReturn = xElapsed;
			if (xHigherPriorityTaskWoken == pdTRUE)
			{
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
		else
		{
			ulReturn = SYS_ARCH_TIMEOUT;
		}
	}
	else
	{
		if (xSemaphoreTake(*sem, timeout / portTICK_RATE_MS) == pdTRUE)
		{
			xEndTime = xTaskGetTickCount();
			xElapsed = (xEndTime - xStartTime) * portTICK_RATE_MS;

			return (xElapsed); // return time blocked TODO test
		}
		else
		{
			return SYS_ARCH_TIMEOUT;
		}
	}

    // if (xSemaphoreTake(*sem, timeout / portTICK_RATE_MS) == pdTRUE)
    // {
    //   EndTime = xTaskGetTickCount();
    //   Elapsed = (EndTime - StartTime) * portTICK_RATE_MS;

    //   return (Elapsed); // return time blocked TODO test
    // }
    // else
    // {
    //   return SYS_ARCH_TIMEOUT;
    // }
  }
  else // must block without a timeout
  {
	
	if(isr_calling_flg != 0)
	{
		xSemaphoreTakeFromISR( *sem, &xHigherPriorityTaskWoken );
		if (xHigherPriorityTaskWoken == pdTRUE)
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
	else
	{
		xSemaphoreTake(*sem, portMAX_DELAY);
	}

	xEndTime = xTaskGetTickCount();
	xElapsed = ( xEndTime - xStartTime ) * portTICK_RATE_MS;

	if( xElapsed == 0UL )
	{
		xElapsed = 1UL;
	}

	ulReturn = xElapsed;

    // err = xSemaphoreTake(*sem, portMAX_DELAY);

    // if (pdTRUE != err)
    // {
    //   LWIP_ASSERT("sys_arch_mbox_fetch xQueueReceive returned with error!", err == pdTRUE);
    // }

    // EndTime = xTaskGetTickCount();
    // Elapsed = (EndTime - StartTime) * portTICK_RATE_MS;

    // return (Elapsed); // return time blocked
  }
  return ulReturn ;
}

/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
void sys_sem_signal(sys_sem_t *sem)
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  if (*sem == NULL)
    return;

	if(isr_calling_flg != 0)
	{
		xSemaphoreGiveFromISR( *sem, &xHigherPriorityTaskWoken );
		if (xHigherPriorityTaskWoken == pdTRUE)
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
	else
	{
		xSemaphoreGive(*sem);
	}	

}


// void sys_sem_signal_fromisr(sys_sem_t *sem)
// {
//   printf("sys_sem_signal_fromisr is here \r\n") ;
//   	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
//   if (*sem == NULL)
//     return;

//   xSemaphoreGiveFromISR(*sem,&xHigherPriorityTaskWoken);
// }

/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
void sys_sem_free(sys_sem_t *sem)
{
  vSemaphoreDelete(*sem);
}
/*-----------------------------------------------------------------------------------*/
int sys_sem_valid(sys_sem_t *sem)
{
  if (*sem == SYS_SEM_NULL)
    return 0;
  else
    return 1;
}

/*-----------------------------------------------------------------------------------*/
/**
 * @ingroup sys_sem
 * Set a semaphore invalid so that sys_sem_valid returns 0
 */
void sys_sem_set_invalid(sys_sem_t *sem)
{
  *sem = SYS_SEM_NULL;
}
/*-----------------------------------------------------------------------------------*/

/* sys_init() must be called before anything else. */
void sys_init(void)
{
  /* nothing on FreeRTOS porting */
}
/*-----------------------------------------------------------------------------------*/
/* Mutexes*/
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
#if LWIP_COMPAT_MUTEX == 0

/**
 * @ingroup sys_mutex
 * Create a new mutex.
 * Note that mutexes are expected to not be taken recursively by the lwIP code,
 * so both implementation types (recursive or non-recursive) should work.
 * @param mutex pointer to the mutex to create
 * @return ERR_OK if successful, another err_t otherwise
 */
err_t sys_mutex_new(sys_mutex_t *mutex)
{

  *mutex = xSemaphoreCreateMutex();

  // vSemaphoreCreateBinary(*mutex);

  if (*mutex == NULL)
  {
#if SYS_STATS
    ++lwip_stats.sys.mutex.err;
#endif /* SYS_STATS */
    return ERR_MEM;
  }

#if SYS_STATS
  ++lwip_stats.sys.mutex.used;
  if (lwip_stats.sys.mutex.max < lwip_stats.sys.mutex.used)
  {
    lwip_stats.sys.mutex.max = lwip_stats.sys.mutex.used;
  }
#endif /* SYS_STATS */
  return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/* Deallocate a mutex*/
void sys_mutex_free(sys_mutex_t *mutex)
{
#if SYS_STATS
  --lwip_stats.sys.mutex.used;
#endif /* SYS_STATS */

  vSemaphoreDelete(*mutex);
}
/*-----------------------------------------------------------------------------------*/
/* Lock a mutex*/
void sys_mutex_lock(sys_mutex_t *mutex)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if (*mutex == NULL)
		return;
	if(isr_calling_flg != 0)
	{
		xSemaphoreTakeFromISR( *mutex, &xHigherPriorityTaskWoken );
		if (xHigherPriorityTaskWoken == pdTRUE)
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
	else
	{
		sys_arch_sem_wait(mutex, 0);
	}
}

/*-----------------------------------------------------------------------------------*/
/* Unlock a mutex*/
void sys_mutex_unlock(sys_mutex_t *mutex)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if (*mutex == NULL)
		return;

  	if(isr_calling_flg != 0)
	{
		xSemaphoreGiveFromISR( *mutex, &xHigherPriorityTaskWoken );
		if (xHigherPriorityTaskWoken == pdTRUE)
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		xSemaphoreGive(*mutex);
	}

}
#endif /*LWIP_COMPAT_MUTEX*/
/*-----------------------------------------------------------------------------------*/
// TODO
/*-----------------------------------------------------------------------------------*/
/*
  Starts a new thread with priority "prio" that will begin its execution in the
  function "thread()". The "arg" argument will be passed as an argument to the
  thread() function. The id of the new thread is returned. Both the id and
  the priority are system dependent.
*/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
  sys_thread_t createdTaskHandle = NULL;

  int result;

  result = xTaskCreate(thread, name, stacksize, arg, prio, &createdTaskHandle);

  if (result == pdPASS)
  {
    return createdTaskHandle;
  }
  else
  {
    return NULL;
  }
}

/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.

  Note: This function is based on FreeRTOS API, because no equivalent CMSIS-RTOS
        API is available
*/
sys_prot_t sys_arch_protect(void)
{
  if(isr_calling_flg != 0)
	{

  }
  else
  {
    vPortEnterCritical();
  }
  
  return 1;
}

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.

  Note: This function is based on FreeRTOS API, because no equivalent CMSIS-RTOS
        API is available
*/
void sys_arch_unprotect(sys_prot_t pval)
{
  (void)pval;
  if(isr_calling_flg != 0)
	{
    
  }
  else
  {
    vPortExitCritical();
  }
}

void sys_arch_assert(const char *file, int line)
{

  printf("sys_arch_assert: %d in %s, pcTaskGetTaskName:%s.r\n", line, file, pcTaskGetTaskName(NULL));
  while (1)
    ;
}

#endif /* !NO_SYS */
