/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fsec_trng.c
 * Created Date: 2025-10-09 15:57:36
 * Last Modified: 2025-10-11 09:53:04
 * Description:  This file is for get rand num.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0  kangkaiping     2025-10-11            first release
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include "ftypes.h"
#include "fdebug.h"

static void printbytes(u8 *src, u32 size)
{
	u32 index;

	for (index = 0; index < size; index++) {
		if (index % 16 == 0) {
			printf("\n\r");
		}
		printf("0x%02X, ", src[index]);
	}
	printf("\n\r");
}

/**
 * 生成指定长度的随机字节
 *
 * @param buf 输出缓冲区
 * @param len 要生成的字节长度
 * @return 1 表示成功
 */
int rand_bytes(uint8_t *buf, size_t len)
{
	uintptr val=0x80101000;
	buf = (uint8_t *)val;
    printbytes(buf, len);
    return 1;
}