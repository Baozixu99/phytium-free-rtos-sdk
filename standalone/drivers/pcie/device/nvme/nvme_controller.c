/*
 * Copyright (c) 2022 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from FreeBSD original driver made by Jim Harris
 * with contributions from Alexander Motin, Wojciech Macek, and Warner Losh
 */


#include <errno.h>
#include "fdrivers_port.h"
#include "nvme_helpers.h"
#include "nvme.h"
#include "nvme_cmd.h"
#include "nvme_config.h"
#include "nvme_disk.h"
#include "fdrivers_port.h"
#include "fpcie_ecam_msix.h"
#include "nvme_intr.h"

#define NVME_DEBUG_TAG "NVME"
#define NVME_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(NVME_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(NVME_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(NVME_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_DEBUG_D(format, ...) FT_DEBUG_PRINT_D(NVME_DEBUG_TAG, format, ##__VA_ARGS__)


#define USEC_PER_MSEC 1000
#define MSEC_PER_SEC 1000

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

static int nvme_controller_wait_for_ready(struct nvme_controller *nvme_ctrlr,
					  const int desired_val)
{
	uintptr regs = nvme_ctrlr->base;
	u64 timeout = FDriverGetTimerTick() +
		MSEC_TO_TICKS(nvme_ctrlr->ready_timeout_in_ms);
	uint32_t delta_t = USEC_PER_MSEC;
	uint32_t csts;

	while (1) {
		csts = nvme_mmio_read_4(regs, csts);
		if (csts == NVME_GONE) {
			NVME_DEBUG_E("Controller is unreachable");
			return -EIO;
		}

		if (((csts >> NVME_CSTS_REG_RDY_SHIFT) &
		     NVME_CSTS_REG_RDY_MASK) == desired_val) {
			break;
		}

		if (timeout - FDriverGetTimerTick() < 0) {
			NVME_DEBUG_E("Timeout error");
			return -EIO;
		}

		FDriverUdelay(delta_t) ;
		delta_t = MIN((MSEC_PER_SEC * USEC_PER_MSEC), delta_t * 3 / 2);
	}

	return 0;
}


static int nvme_controller_disable(struct nvme_controller *nvme_ctrlr)
{
	uintptr regs = nvme_ctrlr->base;
	uint32_t cc, csts;
	uint8_t enabled, ready;
	int err;

	cc = nvme_mmio_read_4(regs, cc);
	csts = nvme_mmio_read_4(regs, csts);

	ready = (csts >> NVME_CSTS_REG_RDY_SHIFT) & NVME_CSTS_REG_RDY_MASK;

	enabled = (cc >> NVME_CC_REG_EN_SHIFT) & NVME_CC_REG_EN_MASK;
	if (enabled == 0) {
		NVME_DEBUG_W("nvme controller already disabled \r\n") ;
		/* Wait for RDY == 0 or timeout & fail */
		if (ready == 0) {
			return 0;
		}
		return nvme_controller_wait_for_ready(nvme_ctrlr, 0);
	}

	if (ready == 0) {
		/* EN == 1, wait for  RDY == 1 or timeout & fail */
		err = nvme_controller_wait_for_ready(nvme_ctrlr, 1);
		if (err != 0) {
			return err;
		}
	}
	/* disable controller */
	cc &= ~NVME_CC_REG_EN_MASK;
	nvme_mmio_write_4(regs, cc, cc);

	return nvme_controller_wait_for_ready(nvme_ctrlr, 0);
}

static int nvme_controller_enable(struct nvme_controller *nvme_ctrlr)
{
	uintptr regs = nvme_ctrlr->base;
	uint8_t enabled, ready;
	uint32_t cc, csts;
	int err;

	cc = nvme_mmio_read_4(regs, cc);
	csts = nvme_mmio_read_4(regs, csts);

	ready = (csts >> NVME_CSTS_REG_RDY_SHIFT) & NVME_CSTS_REG_RDY_MASK;

	enabled = (cc >> NVME_CC_REG_EN_SHIFT) & NVME_CC_REG_EN_MASK;
	if (enabled == 1) {
		if (ready == 1) {
			NVME_DEBUG_D("Already enabled");
			return 0;
		}

		return nvme_controller_wait_for_ready(nvme_ctrlr, 1);
	}

	/* EN == 0 already wait for RDY == 0 or timeout & fail */
	err = nvme_controller_wait_for_ready(nvme_ctrlr, 0);
	if (err != 0) {
		return err;
	}

	/* Initialization values for CC */
	cc = 0;
	/* set cc enable bit */
	cc |= 1 << NVME_CC_REG_EN_SHIFT;
	/* select NVM Command Set */
	cc |= 0 << NVME_CC_REG_CSS_SHIFT;
	/* select Round Robin */
	cc |= 0 << NVME_CC_REG_AMS_SHIFT;
	/* No notification */
	cc |= 0 << NVME_CC_REG_SHN_SHIFT;
	/* Size of queue entry submitted ，sizeof(struct nvme_command) */
	cc |= 6 << NVME_CC_REG_IOSQES_SHIFT; /* SQ entry size == 64 == 2^6 */
	/* Size of queue entry completed ，sizeof(struct nvme_completion) */
	cc |= 4 << NVME_CC_REG_IOCQES_SHIFT; /* CQ entry size == 16 == 2^4 */
	/* Set the host memory page size to cp_hi.mpsmin*/
	cc |= nvme_ctrlr->mps << NVME_CC_REG_MPS_SHIFT; 

	nvme_mmio_write_4(regs, cc, cc);

	return nvme_controller_wait_for_ready(nvme_ctrlr, 1);
}

static int nvme_controller_setup_admin_queues(struct nvme_controller *nvme_ctrlr)
{

	uintptr regs = nvme_ctrlr->base;
	uint32_t aqa, qsize;
	/* reset admin cmd and completion queue */
	nvme_cmd_qpair_reset(nvme_ctrlr->adminq);

	/* Admin queue is always id 0 */
	if (nvme_cmd_qpair_setup(nvme_ctrlr->adminq, nvme_ctrlr, 0) != 0) {
		NVME_DEBUG_E("Admin cmd qpair setup failed");
		return -EIO;
	}

	/* set admin submission queue base addr*/
	nvme_mmio_write_8(regs, asq, nvme_ctrlr->adminq->cmd_bus_addr);
	NVME_DEBUG_I("Admin Submission Queue set to 0x%lx", nvme_ctrlr->adminq->cmd_bus_addr);
	/* set Admin Completion Queue Base Address */
	nvme_mmio_write_8(regs, acq, nvme_ctrlr->adminq->cpl_bus_addr);
	NVME_DEBUG_I("Admin Completion Queue set to 0x%lx", nvme_ctrlr->adminq->cpl_bus_addr) ;

	/* acqs and asqs are 0-based. */
	qsize = CONFIG_NVME_ADMIN_ENTRIES - 1;
	aqa = 0;

	/* set Admin Completion Queues size */
	aqa = (qsize & NVME_AQA_REG_ACQS_MASK) << NVME_AQA_REG_ACQS_SHIFT;
	/* set Admin Submission Queue size */
	aqa |= (qsize & NVME_AQA_REG_ASQS_MASK) << NVME_AQA_REG_ASQS_SHIFT;

	nvme_mmio_write_4(regs, aqa, aqa);

	return 0;
}

static int nvme_controller_setup_io_queues(struct nvme_controller *nvme_ctrlr)
{
	struct nvme_completion_poll_status status;
	struct nvme_cmd_qpair *io_qpair;
	int cq_allocated, sq_allocated;
	int ret, idx;

	nvme_cpl_status_poll_init(nvme_ctrlr,&status);
	/* set f I/O Completion and Submission Queues unmber ,use num_io_queues */
	ret =  nvme_ctrlr_cmd_set_num_queues(nvme_ctrlr,
					     nvme_ctrlr->num_io_queues,
					     nvme_completion_poll_cb, &status);

	if (ret != 0) {
		return ret;
	}
	
	nvme_completion_poll(&status);

	if (nvme_cpl_status_is_error(&status)) {
		NVME_DEBUG_E("Could not set IO num queues to %u",
			nvme_ctrlr->num_io_queues);
		nvme_completion_print(&status.cpl);
		return -EIO;
	}
	
	/*
	 * Data in cdw0 is 0-based.
	 * Lower 16-bits indicate number of submission queues allocated.
	 * Upper 16-bits indicate number of completion queues allocated.
	 */
	sq_allocated = (status.cpl.cdw0 & 0xFFFF) + 1;
	cq_allocated = (status.cpl.cdw0 >> 16) + 1;

	/*
	 * Controller may allocate more queues than we requested,
	 * so use the minimum of the number requested and what was
	 * actually allocated.
	 */
	nvme_ctrlr->num_io_queues = MIN(nvme_ctrlr->num_io_queues,
					sq_allocated);
	nvme_ctrlr->num_io_queues = MIN(nvme_ctrlr->num_io_queues,
					cq_allocated);
	NVME_DEBUG_I("num_io_queues is %d \r\n",nvme_ctrlr->num_io_queues) ;
	for (idx = 0; idx < nvme_ctrlr->num_io_queues; idx++) {
		io_qpair = &nvme_ctrlr->ioq[idx];
		/*  set i/o pair struct */ 
		if (nvme_cmd_qpair_setup(io_qpair, nvme_ctrlr, idx+1) != 0) {
			NVME_DEBUG_E("IO cmd qpair %u setup failed", idx+1);
			return -EIO;
		}
		/* rest i/o pair cmd and completion queue */
		nvme_cmd_qpair_reset(io_qpair);
		nvme_cpl_status_poll_init(nvme_ctrlr,&status);

		/* creadte i/o completion queue */
		ret = nvme_ctrlr_cmd_create_io_cq(nvme_ctrlr, io_qpair,
						  nvme_completion_poll_cb,
						  &status);
		if (ret != 0) {
			return ret;
		}

		nvme_completion_poll(&status);
		if (nvme_cpl_status_is_error(&status)) {
			NVME_DEBUG_E("IO CQ creation failed");
			nvme_completion_print(&status.cpl);
			return -EIO;
		}
		nvme_cpl_status_poll_init(nvme_ctrlr,&status);
		/* create io submission queue */
		ret = nvme_ctrlr_cmd_create_io_sq(nvme_ctrlr, io_qpair,
						  nvme_completion_poll_cb,
						  &status);
		if (ret != 0) {
			return ret;
		}
		nvme_completion_poll(&status);
		if (nvme_cpl_status_is_error(&status)) {
			NVME_DEBUG_E("IO CQ creation failed");
			nvme_completion_print(&status.cpl);
			return -EIO;
		}
	}

	return 0;
}

static void nvme_controller_gather_info(struct nvme_controller *nvme_ctrlr)
{
	uintptr_t regs = nvme_ctrlr->base;
	uint32_t cap_lo, cap_hi, vs, pmrcap;
	uint8_t mpsmin, mpsmax, dstrd;
	uint32_t page_size_min, page_size_max, doorbell_stride;
	uint32_t timeout_ms;
	/* Read controller capabilities registers */
	nvme_ctrlr->cap_lo = cap_lo = nvme_mmio_read_4(regs, cap_lo);
	nvme_ctrlr->cap_hi = cap_hi = nvme_mmio_read_4(regs, cap_hi);

	/* Decode CAP_LO register fields */
	NVME_DEBUG_D("Controller Capabilities Low (CAP_LO): 0x%08x", cap_lo);
	NVME_DEBUG_D("  Maximum Queue Entries Supported (MQES): 0x%04x",
		NVME_CAP_LO_MQES(cap_lo));
	NVME_DEBUG_D("  Contiguous Queues Required (CQR): %s",
		NVME_CAP_LO_CQR(cap_lo) ? "Yes" : "No");
	NVME_DEBUG_D("  Arbitration Mechanism Supported (AMS): 0x%x",
		NVME_CAP_LO_AMS(cap_lo));
	NVME_DEBUG_D("  Timeout (TO): %u (Worst case: %u ms)",
		NVME_CAP_LO_TO(cap_lo),
		(NVME_CAP_LO_TO(cap_lo) + 1) * 500);

	/* Decode CAP_HI register fields */
	NVME_DEBUG_D("Controller Capabilities High (CAP_HI): 0x%08x", cap_hi);
	
	mpsmin = NVME_CAP_HI_MPSMIN(cap_hi);
	mpsmax = NVME_CAP_HI_MPSMAX(cap_hi);
	page_size_min = 1 << (12 + mpsmin);
	page_size_max = 1 << (12 + mpsmax);
	
	NVME_DEBUG_D("  Minimum Memory Page Size (MPSMIN): %u (2^(12+%u) = %u bytes)",
		mpsmin, mpsmin, page_size_min);
	NVME_DEBUG_D("  Maximum Memory Page Size (MPSMAX): %u (2^(12+%u) = %u bytes)",
		mpsmax, mpsmax, page_size_max);
	NVME_DEBUG_D("  Boot Partition Support (BPS): %s",
		NVME_CAP_HI_BPS(cap_hi) ? "Supported" : "Not Supported");
	
	NVME_DEBUG_D("  Command Sets Supported (CSS): 0x%02x",
		NVME_CAP_HI_CSS(cap_hi));
	NVME_DEBUG_D("    NVM Command Set Supported: %s",
		NVME_CAP_HI_CSS_NVM(cap_hi) ? "Yes" : "No");
	
	NVME_DEBUG_D("  NVM Subsystem Reset Support (NSSRS): %s",
		NVME_CAP_HI_NSSRS(cap_hi) ? "Supported" : "Not Supported");
	
	dstrd = NVME_CAP_HI_DSTRD(cap_hi);
	doorbell_stride = 1 << (2 + dstrd);
	NVME_DEBUG_D("  Doorbell Stride (DSTRD): %u (2^(2+%u) = %u bytes)",
		dstrd, dstrd, doorbell_stride);

	/* Read and decode version register */
	vs = nvme_mmio_read_4(regs, vs);
	NVME_DEBUG_D("NVMe Specification Version: 0x%08x", vs);
	NVME_DEBUG_D("  Major Version: %u", NVME_MAJOR(vs));
	NVME_DEBUG_D("  Minor Version: %u", NVME_MINOR(vs));

	/* Read and decode PMRCap register if supported */
	if (NVME_CAP_HI_PMRS(cap_hi)) {
		pmrcap = nvme_mmio_read_4(regs, pmrcap);
		NVME_DEBUG_D("Persistent Memory Range Capabilities (PMRCap): 0x%08x", pmrcap);
		NVME_DEBUG_D("  Base I/O Request (BIR): %u", NVME_PMRCAP_BIR(pmrcap));
		NVME_DEBUG_D("  Read Data Supported (RDS): %s",
			NVME_PMRCAP_RDS(pmrcap) ? "Yes" : "No");
		NVME_DEBUG_D("  Write Data Supported (WDS): %s",
			NVME_PMRCAP_WDS(pmrcap) ? "Yes" : "No");
		NVME_DEBUG_D("  Power Management Range Time Unit (PMRTU): %u",
			NVME_PMRCAP_PMRTU(pmrcap));
		NVME_DEBUG_D("  Power Management Range Write Buffer Maximum (PMRWBM): 0x%x",
			NVME_PMRCAP_PMRWBM(pmrcap));
		NVME_DEBUG_D("  Power Management Range Timeout (PMRTO): %u",
			NVME_PMRCAP_PMRTO(pmrcap));
		NVME_DEBUG_D("  Controller Memory Space Support (CMSS): %s",
			NVME_PMRCAP_CMSS(pmrcap) ? "Supported" : "Not Supported");
	}

	/* Configure controller parameters based on capabilities */
	nvme_ctrlr->dstrd = dstrd + 2;
	nvme_ctrlr->mps = mpsmin;
	nvme_ctrlr->page_size = page_size_min;
	
	NVME_DEBUG_D("Selected Memory Page Size: %u (2^(12+%u) = %u bytes)",
		nvme_ctrlr->mps, nvme_ctrlr->mps, nvme_ctrlr->page_size);

	timeout_ms = (NVME_CAP_LO_TO(cap_lo) + 1) * 500;
	nvme_ctrlr->ready_timeout_in_ms = timeout_ms;
	NVME_DEBUG_D("Ready Timeout: %u ms", timeout_ms);

	nvme_ctrlr->max_xfer_size = nvme_ctrlr->page_size / 8 * nvme_ctrlr->page_size;
	NVME_DEBUG_D("Maximum Transfer Size: %u bytes", nvme_ctrlr->max_xfer_size);
}

static int nvme_controller_pcie_configure(struct nvme_controller *nvme_ctrlr)
{

	
		uint32_t n_vectors;
		
		n_vectors = nvme_ctrlr->msi_x_setup(nvme_ctrlr,NVME_PCIE_MSIX_VECTORS,1)  ;

		if( n_vectors != NVME_PCIE_MSIX_VECTORS )
		{
			NVME_DEBUG_E("Could not allocate %u MSI-X vectors",
				NVME_PCIE_MSIX_VECTORS);
			return -EIO;
		}


	return 0;
}

static int nvme_controller_identify(struct nvme_controller *nvme_ctrlr)
{
	struct nvme_completion_poll_status status =
		NVME_CPL_STATUS_POLL_INIT(status,nvme_ctrlr);

	/* 5.15.3 Identify Controller data structure (CNS 01h) */
	nvme_ctrlr_cmd_identify_controller(nvme_ctrlr,
					   nvme_completion_poll_cb, &status);

	nvme_completion_poll(&status);
	if (nvme_cpl_status_is_error(&status)) {
		NVME_DEBUG_E("Could not identify the controller");
		nvme_completion_print(&status.cpl);
		return -EIO;
	}
	nvme_completion_print(&status.cpl);

	/*
	 * Use MDTS to ensure our default max_xfer_size doesn't exceed what the
	 * controller supports.
	 */
	if (nvme_ctrlr->cdata.mdts > 0) {
		nvme_ctrlr->max_xfer_size =
			MIN(nvme_ctrlr->max_xfer_size,
			    1 << (nvme_ctrlr->cdata.mdts + NVME_MPS_SHIFT +
				  NVME_CAP_HI_MPSMIN(nvme_ctrlr->cap_hi)));
	}

	return 0;
}

static void nvme_controller_setup_namespaces(struct nvme_controller *nvme_ctrlr)
{
	uint32_t i;

	for (i = 0;
	     i < MIN(nvme_ctrlr->cdata.nn, CONFIG_NVME_MAX_NAMESPACES); i++) {
		struct nvme_namespace *ns = &nvme_ctrlr->ns[i];

		if (nvme_namespace_construct(ns, i+1, nvme_ctrlr) != 0) {
			break;
		}

		NVME_DEBUG_D("Namespace id %u setup and running", i);
	}
}

int nvme_controller_init(struct nvme_controller *nvme_ctrlr,nvme_msi_x_setup msi_x_setup ,nvme_interrupt_setup interrupt_setup_fun)
{
	int ret;
	
	if(nvme_ctrlr == NULL)
	{
		return -EINVAL;
	}

	nvme_ctrlr->msi_x_setup = msi_x_setup ;
	nvme_ctrlr->interrupt_setup_fun = interrupt_setup_fun ;

	nvme_cmd_init(nvme_ctrlr);

	nvme_controller_gather_info(nvme_ctrlr);
	ret = nvme_controller_disable(nvme_ctrlr);
	if (ret != 0) {
		NVME_DEBUG_E("Controller cannot be disabled");
		return ret;
	}
	/* config  msi-x interrupt */
	ret = nvme_controller_pcie_configure(nvme_ctrlr);
	if (ret != 0) {
		return ret;
	}

	/* Initialize the data structure of the admin queues. */
	ret = nvme_controller_setup_admin_queues(nvme_ctrlr);
	if (ret != 0) {
		return ret;
	}
	/* enable nvme controller and config admin parameters 	*/
	ret = nvme_controller_enable(nvme_ctrlr);
	if (ret != 0) {
		NVME_DEBUG_E("Controller cannot be enabled");
		return ret;
	}
	
	/* create i/o queues */
	ret = nvme_controller_setup_io_queues(nvme_ctrlr);
	if (ret != 0) {
		return ret;
	}


	ret = nvme_controller_identify(nvme_ctrlr);
	if (ret != 0) {
		return ret;
	}	
	/* setup namespaces(logic space) */
	nvme_controller_setup_namespaces(nvme_ctrlr);

	return 0;
}

