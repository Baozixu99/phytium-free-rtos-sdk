/*
 * Copyright (c) 2022 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from FreeBSD original driver made by Jim Harris
 */
#include <stdio.h>
#include "nvme_disk.h"
#include "nvme.h"
#include "nvme_cmd.h"
#include "nvme_config.h"
#include "nvme_intr.h"


#include "fdrivers_port.h"
#define NVME_NVME_DEBUG_TAG "NVME_NVME"
#define NVME_NVME_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(NVME_NVME_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_NVME_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(NVME_NVME_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_NVME_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(NVME_NVME_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_NVME_DEBUG_D(format, ...) FT_DEBUG_PRINT_D(NVME_NVME_DEBUG_TAG, format, ##__VA_ARGS__)



uint32_t nvme_namespace_get_sector_size(struct nvme_namespace *ns)
{
	uint8_t flbas_fmt, lbads;

	flbas_fmt = (ns->data.flbas >> NVME_NS_DATA_FLBAS_FORMAT_SHIFT) &
		NVME_NS_DATA_FLBAS_FORMAT_MASK;
	lbads = (ns->data.lbaf[flbas_fmt] >> NVME_NS_DATA_LBAF_LBADS_SHIFT) &
		NVME_NS_DATA_LBAF_LBADS_MASK;

	return 1 << lbads;
}

uint64_t nvme_namespace_get_num_sectors(struct nvme_namespace *ns)
{
	return ns->data.nsze;
}

uint64_t nvme_namespace_get_size(struct nvme_namespace *ns)
{
	return nvme_namespace_get_num_sectors(ns) *
		nvme_namespace_get_sector_size(ns);
}

uint32_t nvme_namespace_get_flags(struct nvme_namespace *ns)
{
	return ns->flags;
}

const char *nvme_namespace_get_serial_number(struct nvme_namespace *ns)
{
	return (const char *)ns->ctrlr->cdata.sn;
}

const char *nvme_namespace_get_model_number(struct nvme_namespace *ns)
{
	return (const char *)ns->ctrlr->cdata.mn;
}

const struct nvme_namespace_data *
nvme_namespace_get_data(struct nvme_namespace *ns)
{
	return &ns->data;
}

uint32_t nvme_namespace_get_stripesize(struct nvme_namespace *ns)
{
	if (((ns->data.nsfeat >> NVME_NS_DATA_NSFEAT_NPVALID_SHIFT) &
	     NVME_NS_DATA_NSFEAT_NPVALID_MASK) != 0) {
		uint32_t ss = nvme_namespace_get_sector_size(ns);

		if (ns->data.npwa != 0) {
			return (ns->data.npwa + 1) * ss;
		} else if (ns->data.npwg != 0) {
			return (ns->data.npwg + 1) * ss;
		}
	}

	return ns->boundary;
}

int nvme_namespace_construct(struct nvme_namespace *ns,
			     uint32_t id,
			     struct nvme_controller *ctrlr)
{
	struct nvme_completion_poll_status status =
		NVME_CPL_STATUS_POLL_INIT(status,ctrlr);
	uint8_t flbas_fmt;
	uint8_t vwc_present;

	ns->ctrlr = ctrlr;
	ns->id = id;

	/* Identify Namespace data structure (CNS 00h) */
	nvme_ctrlr_cmd_identify_namespace(ctrlr, id, &ns->data,
					  nvme_completion_poll_cb,
					  &status);
	nvme_completion_poll(&status);

	if (nvme_cpl_status_is_error(&status)) {
		NVME_NVME_DEBUG_D("Identifying NS id %d failed", id);
		return -EIO;
	}

	nvme_namespace_data_swapbytes(&ns->data);

	if (nvme_namespace_get_num_sectors(ns) == 0) {
		NVME_NVME_DEBUG_D("Namespace %d not present", id);
		return -ENODEV;
	}

	flbas_fmt = (ns->data.flbas >> NVME_NS_DATA_FLBAS_FORMAT_SHIFT) &
		NVME_NS_DATA_FLBAS_FORMAT_MASK;

	/* Note: format is a 0-based value, so > is appropriate here not >=. */
	if (flbas_fmt > ns->data.nlbaf) {
		NVME_NVME_DEBUG_D("NS id %d: lba format %d exceeds number supported (%d)",
			id, flbas_fmt, ns->data.nlbaf + 1);
		return -EIO;
	}

	ns->boundary = ns->data.noiob * nvme_namespace_get_sector_size(ns);

	if (nvme_controller_has_dataset_mgmt(ctrlr)) {
		ns->flags |= NVME_NS_DEALLOCATE_SUPPORTED;
	}

	vwc_present = (ctrlr->cdata.vwc >> NVME_CTRLR_DATA_VWC_PRESENT_SHIFT) &
		NVME_CTRLR_DATA_VWC_PRESENT_MASK;
	if (vwc_present) {
		ns->flags |= NVME_NS_FLUSH_SUPPORTED;
	}

	snprintf(ns->name, NVME_NAMESPACE_NAME_MAX_LENGTH, "nvme%dn%d",
		 ctrlr->id, ns->id-1);

	if (nvme_disk_init(&ns->disk,ns) != 0) {
		NVME_NVME_DEBUG_E("Could not register no disk subsystem");
	}
	ns->disk.name = ns->name ;
	ctrlr->disk = &ns->disk ;

	
	return 0;
}
