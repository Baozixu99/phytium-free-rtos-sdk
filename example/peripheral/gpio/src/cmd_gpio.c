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
 * FilePath: cmd_gpio.c
 * Date: 2022-06-28 14:42:53
 * LastEditTime: 2022-06-28 14:42:53
 * Description:  This files is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"

#include "../src/shell.h"
#include "fgpio_os.h"
#include "gpio_io_irq.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static boolean GpioParseIndex(const char *str, u32 *pin)
{
	FASSERT(str && pin);
	u32 id_num = 0;
	char port = 'a';
	u32 pin_num = 0;
    u32 port_num = 0;

    if (3 != sscanf(str, "%d-%c-%d", &id_num, &port, &pin_num))
	{
		printf("Parse as %d-%c-%d", id_num, port, pin_num);
		return FALSE;
	}

	if ((id_num >= FGPIO_NUM) || 
	    ((port != 'a') && (port != 'b')) ||
		( pin_num >= FGPIO_PIN_NUM))
	{
		printf("Wrong pin index");
		return FALSE;
	}

    port_num = (('a' == port) ? 0 : 1); /* 0 = port-a, 1 = port-b */
    *pin = FFREERTOS_GPIO_PIN_INDEX(id_num, port_num, pin_num);
    printf("pin index = 0x%x\r\n", *pin);

    return TRUE;
}

static int GpioCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    const char *in_pin_str = "3-a-4";
    const char *out_pin_str = "3-a-5";
    u32 in_pin, out_pin;

    if (!strcmp(argv[1], "io-irq"))
    {
        if (argc >= 3)
        {
            out_pin_str = argv[2];
        }

        if (argc >= 4)
        {
            in_pin_str = argv[3];
        }

        if ((FALSE == GpioParseIndex(out_pin_str, &out_pin)) || 
            (FALSE == GpioParseIndex(in_pin_str, &in_pin)))
        {
            return -2;
        }

        ret = (int)FFreeRTOSRunGpioIOIrq(out_pin, in_pin);
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), gpio, GpioCmdEntry, test freertos gpio driver);