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
 * FilePath: test_uart.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:02:36
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include <stdio.h>
#include "ft_types.h"
#include "fpl011.h"
#include "interrupt.h"

static FPl011 pl011_obj;

void Uart1Init(void)
{
    FPl011Config config = *FPl011LookupConfig(1);
    printf("config addr: 0x%x \r\n", config.base_address);
    FPl011CfgInitialize(&pl011_obj, &config);
    FPl011SetOptions(&pl011_obj, FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN | FPL011_OPTION_FIFOEN);
}

u8 BlockReceive()
{
    return FPl011BlockReceive(&pl011_obj);
}