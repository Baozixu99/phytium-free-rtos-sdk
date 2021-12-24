/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-05 13:48:25
 * @LastEditTime: 2021-12-15 15:58:22
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#include <stdio.h>
#include "ft_types.h"
#include "fpl011.h"
#include "gicv3.h"
#include "interrupt.h"

static FPl011 pl011_obj;

void Uart1Init(void)
{
    FPl011Config *config = (FPl011Config *)FPl011LookupConfig(1);
    printf("config addr: 0x%x \r\n", config->base_address);
    FPl011CfgInitialize(&pl011_obj, config);
    FPl011SetOptions(&pl011_obj, FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN | FPL011_OPTION_FIFOEN);
}

u8 BlockReceive()
{
    return FPl011BlockReceive(&pl011_obj);
}