/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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