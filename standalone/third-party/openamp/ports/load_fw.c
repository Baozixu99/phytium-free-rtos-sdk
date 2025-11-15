/*
 * Load firmware example
 *
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform_info.h"
#include "load_fw.h"
#include "rsc_table.h"

extern const struct image_store_ops mem_image_store_ops;


void print_vring(const struct fw_rsc_vdev_vring *vring)
{
    printf("Vring da: 0x%x\n", vring->da);
    printf("Vring align: 0x%x\n", vring->align);
    printf("Vring num: 0x%x\n", vring->num);
    printf("Vring notifyid: 0x%x\n", vring->notifyid);
    printf("Vring reserved: 0x%x\n", vring->reserved);
}

/* 打印 vdev 结构 */
void print_vdev(const struct fw_rsc_vdev *vdev)
{
    printf("Vdev type: 0x%x\n", vdev->type);
    printf("Vdev id: 0x%x\n", vdev->id);
    printf("Vdev notifyid: 0x%x\n", vdev->notifyid);
    printf("Vdev dfeatures: 0x%x\n", vdev->dfeatures);
    printf("Vdev gfeatures: 0x%x\n", vdev->gfeatures);
    printf("Vdev config_len: 0x%x\n", vdev->config_len);
    printf("Vdev status: 0x%x\n", vdev->status);
    printf("Vdev num_of_vrings: 0x%x\n", vdev->num_of_vrings);

    for (int i = 0; i < vdev->num_of_vrings; i++)
    {
        print_vring(&(vdev->vring[i]));
    }
}

/* 打印整个资源表结构 */
void print_resource_table(const struct remote_resource_table *table)
{
    printf("Resource table version: 0x%x\n", table->version);
    printf("Resource table num: 0x%x\n", table->num);
    printf("Resource table reserved: 0x%x 0x%x\n", table->reserved[0], table->reserved[1]);
    printf("Resource table offsets: 0x%x 0x%x\n", table->offset[0], table->offset[1]);

    print_vdev(&(table->rpmsg_vdev));
    print_vring(&(table->rpmsg_vring0));
    print_vring(&(table->rpmsg_vring1));
}

int load_exectuable_block(struct remoteproc *rproc, const struct image_store_ops *store_ops,
                          void *store, const char *img_path)
{
    int ret;

    (void)img_path;
    if (rproc == NULL)
    {
        return -EINVAL;
    }
    /* Configure remoteproc to get ready to load executable */
    remoteproc_config(rproc, NULL);

    /* Load remoteproc executable */
    printf("Start to load executable with remoteproc_load() \r\n");
    ret = remoteproc_load(rproc, NULL, store, (struct image_store_ops *)store_ops, NULL);
    if (ret)
    {
        printf("failed to load firmware\r\n");
        return ret;
    }

    /* Start the processor */
    ret = remoteproc_start(rproc);
    if (ret)
    {
        printf("failed to start processor\r\n");
        return ret;
    }
    printf("successfully started the processor\r\n");

    return 0;
}
