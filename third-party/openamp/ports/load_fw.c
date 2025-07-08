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
 * FilePath: load_fw.c
 * Created Date: 2024-05-06 14:48:29
 * Last Modified: 2025-05-20 10:10:17
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#include "platform_info.h"
#include "load_fw.h"
#include "rsc_table.h"

extern const struct image_store_ops mem_image_store_ops;


void print_vring(const struct fw_rsc_vdev_vring *vring) {
    printf("Vring da: 0x%x\n", vring->da);
    printf("Vring align: 0x%x\n", vring->align);
    printf("Vring num: 0x%x\n", vring->num);
    printf("Vring notifyid: 0x%x\n", vring->notifyid);
    printf("Vring reserved: 0x%x\n", vring->reserved);
}

/* 打印 vdev 结构 */
void print_vdev(const struct fw_rsc_vdev *vdev) {
    printf("Vdev type: 0x%x\n", vdev->type);
    printf("Vdev id: 0x%x\n", vdev->id);
    printf("Vdev notifyid: 0x%x\n", vdev->notifyid);
    printf("Vdev dfeatures: 0x%x\n", vdev->dfeatures);
    printf("Vdev gfeatures: 0x%x\n", vdev->gfeatures);
    printf("Vdev config_len: 0x%x\n", vdev->config_len);
    printf("Vdev status: 0x%x\n", vdev->status);
    printf("Vdev num_of_vrings: 0x%x\n", vdev->num_of_vrings);

    for (int i = 0; i < vdev->num_of_vrings; i++) {
        print_vring(&(vdev->vring[i]));
    }
}

/* 打印整个资源表结构 */
void print_resource_table(const struct remote_resource_table *table) {
    printf("Resource table version: 0x%x\n", table->version);
    printf("Resource table num: 0x%x\n", table->num);
    printf("Resource table reserved: 0x%x 0x%x\n", table->reserved[0], table->reserved[1]);
    printf("Resource table offsets: 0x%x 0x%x\n", table->offset[0], table->offset[1]);

    print_vdev(&(table->rpmsg_vdev));
    print_vring(&(table->rpmsg_vring0));
    print_vring(&(table->rpmsg_vring1));
}

int load_exectuable_block(struct remoteproc *rproc,
			  const struct image_store_ops *store_ops, void *store,
			  const char *img_path)
{
	int ret;

	(void)img_path;
	if (rproc == NULL)
		return -EINVAL;
	/* Configure remoteproc to get ready to load executable */
	remoteproc_config(rproc, NULL);

	/* Load remoteproc executable */
	printf("Start to load executable with remoteproc_load() \r\n");
	ret = remoteproc_load(rproc, NULL, store, (struct image_store_ops *)store_ops, NULL);
	if (ret) {
		printf("failed to load firmware\r\n");
		return ret;
	}
	
	/* Start the processor */
	ret = remoteproc_start(rproc);
	if (ret) {
		printf("failed to start processor\r\n");
		return ret;
	}
	printf("successfully started the processor\r\n");

	return 0;
}
