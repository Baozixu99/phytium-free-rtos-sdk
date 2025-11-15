/*
 * @Author: zhangyan 1422953826@qq.com
 * @Date: 2025-05-19 17:31:47
 * @LastEditors: zhangyan 1422953826@qq.com
 * @LastEditTime: 2025-05-19 19:19:44
 * @FilePath: \phytium-standalone-sdk\third-party\lwip-2.1.2\ports\sockets_ext.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define IPV6_MULTICAST_IF   0x300
#define IPV6_MULTICAST_HOPS 0x301
#define IPV6_MULTICAST_LOOP 0x302

struct lwip_sock;

bool lwip_setsockopt_impl_ext(struct lwip_sock *sock, int level, int optname,
                              const void *optval, uint32_t optlen, int *err);
bool lwip_getsockopt_impl_ext(struct lwip_sock *sock, int level, int optname,
                              void *optval, uint32_t *optlen, int *err);
#ifdef __cplusplus
}
#endif
