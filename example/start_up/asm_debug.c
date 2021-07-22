/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-05 09:30:54
 * @LastEditTime: 2021-07-05 09:45:26
 * @Description:  This files is for 
 * 
 * @Modify History: 
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