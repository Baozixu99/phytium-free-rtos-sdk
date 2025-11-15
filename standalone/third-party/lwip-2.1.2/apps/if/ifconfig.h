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
 * FilePath: ifconfig.h
 * Date: 2022-12-13 09:47:52
 * LastEditTime: 2022-12-13 09:47:52
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */

#ifndef _IFCONFIG_H
#define _IFCONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

    void SetIf(char *netif_name, char *ip_addr, char *gw_addr, char *nm_addr);
    void ListIf(void);
    void SetMtu(char *netif_name, char *Mtu_value);
#ifdef __cplusplus
}
#endif


#endif
