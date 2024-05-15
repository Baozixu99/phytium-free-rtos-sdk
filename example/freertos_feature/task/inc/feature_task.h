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
 * FilePath: feature_task.h
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for task function define
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */


#ifndef FEATURE_TASK_H
#define FEATURE_TASK_H

#include "fdebug.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FTASK_DEBUG_TAG "FFreeRTOSTask"
#define FTASK_ERROR(format, ...) FT_DEBUG_PRINT_E(FTASK_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTASK_WARN(format, ...)  FT_DEBUG_PRINT_W(FTASK_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTASK_INFO(format, ...)  FT_DEBUG_PRINT_I(FTASK_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTASK_DEBUG(format, ...) FT_DEBUG_PRINT_D(FTASK_DEBUG_TAG, format, ##__VA_ARGS__)

/* creating task */
void CreateTasks(void);

/* using parameter */
void CreateTasksForParamterTest(void);

/* test priority */
void CreateTasksForPriorityTest(void);

/* test block state */
void CreateTasksForBlockTest(void);

/* tesk until delay */

void CreateTasksForDelayUntilTest(void);

/* blocking or none */
void CreateTasksForBlockingOrNone(void);

/* idle task */
void CreateTasksForIdleTask(void);

/* change priority task */
void CreateTasksForChangePriorityTest(void);

/*auto test*/
void TasksExampleEntry(void);

#ifdef __cplusplus
}
#endif

#endif // !