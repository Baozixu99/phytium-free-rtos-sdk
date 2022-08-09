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
 * FilePath: interrupt_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */
#include "shell.h"
#include <string.h>
#include <stdio.h>
#include "feature_interrupt.h"

typedef enum
{
    BINARY_SEM_TASK_INDEX = 0,
    COUNT_SEM_TASK_INDEX = 1,
    QUEUE_TASK_INDEX = 2,
    INTR_FEATURE_LENGTH
} FreeRtosIntrFeatureSelect;

static void CreateIntrCmdUsage(void)
{
    printf("usage:\r\n");
    printf(" intr bin_cre \r\n");
    printf("    -- create intr binary sem tasks now\r\n");
    printf(" intr bin_del \r\n");
    printf("    -- del intr binary sem tasks now\r\n");
    printf(" intr count_cre \r\n");
    printf("    -- create counting sem tasks now\r\n");
    printf(" intr count_del \r\n");
    printf("    -- del counting sem tasks now\r\n");
    printf(" intr queue_cre \r\n");
    printf("    -- create queue tasks now\r\n");
    printf(" intr queue_del \r\n");
    printf("    -- del queue tasks now\r\n");
}

int CreateIntrCmd(int argc, char *argv[])
{
    static int create_flg[INTR_FEATURE_LENGTH] = {0}; /* 1 is tasks has been created*/

    if (argc < 2)
    {
        CreateIntrCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "bin_cre"))
    {
        if(create_flg[BINARY_SEM_TASK_INDEX]  == 0)
        {
            CreateBinarySemTasks();
            create_flg[BINARY_SEM_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use bin_del cmd first \r\n");
        }
    }
    else if (!strcmp(argv[1], "bin_del"))
    {
        if(create_flg[BINARY_SEM_TASK_INDEX]  == 1)
        {
            DeleteBinarySemTasks();
            create_flg[BINARY_SEM_TASK_INDEX]  = 0;
        }        
        else
        {
            printf("Please use bin_cre cmd first \r\n");
        }
    }
    else if (!strcmp(argv[1], "count_cre"))
    {
        if(create_flg[COUNT_SEM_TASK_INDEX]  == 0)
        {
            CreateCountSemTasks();
            create_flg[COUNT_SEM_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use count_del cmd first \r\n");
        }
    }
    else if (!strcmp(argv[1], "count_del"))
    {
        if(create_flg[COUNT_SEM_TASK_INDEX]  == 1)
        {
            DeleteCountSemTasks();
            create_flg[COUNT_SEM_TASK_INDEX]  = 0;
        }        
        else
        {
            printf("Please use count_cre cmd first \r\n");
        }
    }
    else if (!strcmp(argv[1], "queue_cre"))
    {
        if(create_flg[QUEUE_TASK_INDEX]  == 0)
        {
            CreateQueueTasks();
            create_flg[QUEUE_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use queue_del cmd first \r\n");
        }
    }
    else if (!strcmp(argv[1], "queue_del"))
    {
        if(create_flg[QUEUE_TASK_INDEX]  == 1)
        {
            DeleteQueueTasks();
            create_flg[QUEUE_TASK_INDEX]  = 0;
        }        
        else
        {
            printf("Please use queue_cre cmd first \r\n");
        }
    }
    else
    {
        printf("Error: Invalid arguments \r\n");
        CreateIntrCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), intr, CreateIntrCmd, intr task test);


