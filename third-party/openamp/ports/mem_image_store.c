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
 * FilePath: mem_image_store.c
 * Created Date: 2024-05-06 14:48:29
 * Last Modified: 2025-05-20 10:11:03
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#include <errno.h>
#include "load_fw.h"
#include <openamp/remoteproc_loader.h>


int mem_image_open(void *store, const char *path, const void **image_data)
{
	struct mem_file *image = store;
	const void *fw_base = image->base;

	(void)(path);
	if (image_data == NULL) {
		printf("%s: input image_data is NULL\r\n", __func__);
		return -EINVAL;
	}
	*image_data = fw_base;
	/* return an abitrary length, as the whole firmware is in memory */
	return 0x100;
}

void mem_image_close(void *store)
{
	/* The image is in memory, does nothing */
	(void)store;
}

int mem_image_load(void *store, size_t offset, size_t size,
		   const void **data, metal_phys_addr_t pa,
		   struct metal_io_region *io,
		   char is_blocking)
{
	struct mem_file *image = store;
	const void *fw_base = image->base;

	(void)is_blocking;

	printf("%s: offset=0x%x, size=0x%x\n\r",
		__func__, offset, size);
	if (pa == METAL_BAD_PHYS) {
		if (data == NULL) {
			printf("%s: data is NULL while pa is ANY\r\n",
				__func__);
			return -EINVAL;
		}
		*data = (const void *)((const char *)fw_base + offset);
	} else {
		void *va;

		if (io == NULL) {
			printf("%s, io is NULL while pa is not ANY\r\n",
				__func__);
			return -EINVAL;
		}
		va = metal_io_phys_to_virt(io, pa);
		if (va == NULL) {
			printf("%s: no va is found\r\n", __func__);
			return -EINVAL;
		}
		memcpy(va, (const void *)((const char *)fw_base + offset), size);
	}

	return (int)size;
}

const struct image_store_ops mem_image_store_ops = {
	.open = mem_image_open,
	.close = mem_image_close,
	.load = mem_image_load,
	.features = SUPPORT_SEEK,
};

