/*
 * Copyright (c) 2022 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#include "nvme.h"
#include "nvme_config.h"
#include "nvme_disk.h"
#include "nvme_cmd.h"
#include "nvme_intr.h"

#include "fdrivers_port.h"
#define NVME_DISK_DEBUG_TAG "NVME_DISK"
#define NVME_DISK_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(NVME_DISK_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_DISK_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(NVME_DISK_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_DISK_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(NVME_DISK_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_DISK_DEBUG_D(format, ...) FT_DEBUG_PRINT_D(NVME_DISK_DEBUG_TAG, format, ##__VA_ARGS__)


static inline void nvme_lock(struct disk_info *disk)
{

}

static inline void nvme_unlock(struct disk_info *disk)
{

}

int nvme_disk_status(struct disk_info *disk)
{
	return 0;
}

int nvme_disk_read(struct disk_info *disk,
			  uint8_t *data_buf,
			  uint32_t start_sector,
			  uint32_t num_sector)
{
	struct nvme_namespace *ns = CONTAINER_OF(disk->name,
						 struct nvme_namespace, name[0]);

	struct nvme_controller *nvme_ctrlr = ns->ctrlr;
	struct nvme_completion_poll_status status =
		NVME_CPL_STATUS_POLL_INIT(status,nvme_ctrlr);
	struct nvme_request *request;
	uint32_t payload_size;
	int ret = 0;

	if (!NVME_IS_BUFFER_DWORD_ALIGNED(data_buf)) {
		NVME_DISK_DEBUG_W("Data buffer pointer needs to be 4-bytes aligned");
		return -EINVAL;
	}

	nvme_lock(disk);

	payload_size = num_sector * nvme_namespace_get_sector_size(ns);
	request = nvme_allocate_request_vaddr(nvme_ctrlr,(void *)data_buf, payload_size,
					      nvme_completion_poll_cb, &status);
	if (request == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	nvme_namespace_read_cmd(&request->cmd, ns->id,
				start_sector, num_sector);

	/* We use only the first ioq atm
	 * ToDo: use smp cpu id and use it to select ioq
	 */
	nvme_cmd_qpair_submit_request(ns->ctrlr->ioq, request);

	nvme_completion_poll(&status);
	if (nvme_cpl_status_is_error(&status)) {
		NVME_DISK_DEBUG_W("Reading at sector %u (count %d) on disk %s failed",
			start_sector, num_sector, ns->name);
		nvme_completion_print(&status.cpl);
		ret = -EIO;
	}
out:
	nvme_unlock(disk);
	return ret;
}

int nvme_disk_write(struct disk_info *disk,
			   const uint8_t *data_buf,
			   uint32_t start_sector,
			   uint32_t num_sector)
{
	struct nvme_namespace *ns = CONTAINER_OF(disk->name,
						 struct nvme_namespace, name[0]);
	struct nvme_controller *nvme_ctrlr = ns->ctrlr;
	struct nvme_completion_poll_status status =
		NVME_CPL_STATUS_POLL_INIT(status,nvme_ctrlr);
	struct nvme_request *request;
	uint32_t payload_size;
	int ret = 0;

	if (!NVME_IS_BUFFER_DWORD_ALIGNED(data_buf)) {
		NVME_DISK_DEBUG_W("Data buffer pointer needs to be 4-bytes aligned");
		return -EINVAL;
	}

	nvme_lock(disk);

	payload_size = num_sector * nvme_namespace_get_sector_size(ns);

	request = nvme_allocate_request_vaddr(nvme_ctrlr,(void *)data_buf, payload_size,
					      nvme_completion_poll_cb, &status);
	if (request == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	nvme_namespace_write_cmd(&request->cmd, ns->id,
				 start_sector, num_sector);

	/* We use only the first ioq atm
	 * ToDo: use smp cpu id and use it to select ioq
	 */
	nvme_cmd_qpair_submit_request(ns->ctrlr->ioq, request);

	nvme_completion_poll(&status);
	if (nvme_cpl_status_is_error(&status)) {
		NVME_DISK_DEBUG_W("Writing at sector %u (count %d) on disk %s failed",
			start_sector, num_sector, ns->name);
		nvme_completion_print(&status.cpl);
		ret = -EIO;
	}
out:
	nvme_unlock(disk);
	return ret;
}

int nvme_disk_flush(struct disk_info *disk)
{

	struct nvme_namespace *ns = (struct nvme_namespace *)disk->ns;
	struct nvme_controller *nvme_ctrlr = ns->ctrlr;
	struct nvme_completion_poll_status status =
		NVME_CPL_STATUS_POLL_INIT(status,nvme_ctrlr);
	struct nvme_request *request;

	request = nvme_allocate_request_null(nvme_ctrlr,nvme_completion_poll_cb, &status);
	if (request == NULL) {
		return -ENOMEM;
	}

	nvme_namespace_flush_cmd(&request->cmd, ns->id);

	/* We use only the first ioq
	 * ToDo: use smp cpu id and use it to select ioq
	 */
	nvme_cmd_qpair_submit_request(ns->ctrlr->ioq, request);

	nvme_completion_poll(&status);
	if (nvme_cpl_status_is_error(&status)) {
		NVME_DISK_DEBUG_E("Flushing disk %s failed", ns->name);
		nvme_completion_print(&status.cpl);
		return -EIO;
	}

	return 0;
}

int nvme_disk_ioctl(struct disk_info *disk, uint8_t cmd, void *buff)
{
	struct nvme_namespace *ns = CONTAINER_OF(disk->name,
						 struct nvme_namespace, name[0]);
	int ret = 0;

	nvme_lock(disk);

	switch (cmd) {
	case DISK_IOCTL_GET_SECTOR_COUNT:
		if (!buff) {
			ret = -EINVAL;
			break;
		}

		*(uint32_t *)buff = nvme_namespace_get_num_sectors(ns);

		break;
	case DISK_IOCTL_GET_SECTOR_SIZE:
		if (!buff) {
			ret = -EINVAL;
			break;
		}

		*(uint32_t *)buff = nvme_namespace_get_sector_size(ns);

		break;
	case DISK_IOCTL_GET_ERASE_BLOCK_SZ:
		if (!buff) {
			ret = -EINVAL;
			break;
		}

		*(uint32_t *)buff = nvme_namespace_get_sector_size(ns);

		break;
	case DISK_IOCTL_CTRL_DEINIT:
	case DISK_IOCTL_CTRL_SYNC:
		ret = nvme_disk_flush(disk);
		break;
	case DISK_IOCTL_CTRL_INIT:
		ret = 0;
		break;
	default:
		ret = -EINVAL;
	}

	nvme_unlock(disk);
	return ret;
}

int nvme_disk_init(struct disk_info *disk,void *ns)
{
	disk->ns=ns ;
	return nvme_disk_ioctl(disk, DISK_IOCTL_CTRL_INIT, NULL);
}

