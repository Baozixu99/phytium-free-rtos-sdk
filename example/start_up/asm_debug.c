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
 * FilePath: asm_debug.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:02:09
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include <stdio.h>
//#define USE_ASM_DEBUG

void AsmStub0()
{
#ifdef USE_ASM_DEBUG
    printf("stub0\r\n");
#endif
}

void AsmStub1()
{
#ifdef USE_ASM_DEBUG
    printf("stub1\r\n");
#endif
}

void AsmStub2()
{
#ifdef USE_ASM_DEBUG
    printf("stub2\r\n");
#endif
}

void AsmStub3()
{
#ifdef USE_ASM_DEBUG
    printf("stub3\r\n");
#endif
}