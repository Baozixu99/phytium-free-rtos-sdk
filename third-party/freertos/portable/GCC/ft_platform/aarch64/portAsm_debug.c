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
 * FilePath: portAsm_debug.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:03:36
 * Description:  This file is for the port asm debug functions
 *
 * Modify History:
 *  Ver   Who          Date         Changes
 * ----- ------       --------    --------------------------------------
 * 1.0  wangxiaodong  2021/12/14  first release
 * 1.1  wangxiaodong  2022/8/9    adapt E2000D
 */



#include <stdio.h>

void test_value(void *x0, void *x1, void *x2, void *x3)
{
    printf("x2 %p \r\n", x2) ;
    while (1) ;
}