/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: cmd_https.c
 * Created Date: 2024-9-29 11:06:40
 * Last Modified: 2024-09-29 15:52:43
 * Description:  This file is for https example cmd catalogue.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   huangjin     2024/9/29          first release
 */

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

#include "https_example.h"

#define EXAMPLE_IDLE 0
#define HTTPS_EXAMPLE_RUNNING 1

static u32 init_flag_mask = EXAMPLE_IDLE;

static void  HttpsExampleCheckState(void)
{
    switch(init_flag_mask)
    {
        case HTTPS_EXAMPLE_RUNNING:
            printf("Https example is running, we need to deinitialize it first! \r\n");
            HttpsTestDeinit();
            init_flag_mask = EXAMPLE_IDLE;
            break;
        default:
            break;
    }
}

/* usage info function for https example */
static void HttpsExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("lwip https\r\n");
    printf("-- run https example to initialize mac controller\r\n");
}

/* entry function for https example */
static int HttpsExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        HttpsExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "https"))
    {
        HttpsExampleCheckState();
        ret = FFreeRTOSHttpsTaskCreate();
        init_flag_mask = HTTPS_EXAMPLE_RUNNING;       
    }

    return ret;
}

/* register command for https example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), lwip, HttpsExampleEntry, https example);
#endif