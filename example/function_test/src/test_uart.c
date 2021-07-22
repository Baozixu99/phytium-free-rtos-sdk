/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-05 13:48:25
 * @LastEditTime: 2021-07-22 09:48:23
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#include <stdio.h>
#include "ft_types.h"
#include "pl011_uart.h"
#include "gicv3.h"
#include "interrupt.h"

static Pl011 pl011_obj;

void Uart1Init(void)
{
    Pl011Config config = *Pl011LookupConfig(1);
    printf("config addr: 0x%x \r\n", config.base_address);
    Pl011CfgInitialize(&pl011_obj, &config);
    Pl011SetOptions(&pl011_obj, FUART_OPTION_UARTEN | FUART_OPTION_RXEN | FUART_OPTION_TXEN | FUART_OPTION_FIFOEN);
}

u8 BlockReceive()
{
    return Pl011BlockReceive(&pl011_obj);
}