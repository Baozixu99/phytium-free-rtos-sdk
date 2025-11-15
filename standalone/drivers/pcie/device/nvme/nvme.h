/*
 * Copyright (c) 2022 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from FreeBSD original driver made by Jim Harris
 * with contributions from Alexander Motin and Wojciech Macek
 */

#ifndef NVME_H
#define NVME_H

#include "dlist.h"
#include "nvme_helpers.h"
#include "nvme_namespace.h"
#include "nvme_cmd.h"
#include "ftypes.h"
#include "fpcie_ecam_msix.h"
#include "nvme_helpers.h"


struct nvme_registers {
    uint32_t    cap_lo; /* controller capabilities, offset: 0x00 */
    uint32_t    cap_hi; /* offset: 0x04 */
    uint32_t    vs;     /* version, offset: 0x08 */
    uint32_t    intms;  /* interrupt mask set, offset: 0x0C */
    uint32_t    intmc;  /* interrupt mask clear, offset: 0x10 */
    uint32_t    cc;     /* controller configuration, offset: 0x14 */
    uint32_t    reserved1; /* offset: 0x18 */
    uint32_t    csts;   /* controller status, offset: 0x1C */
    uint32_t    nssr;   /* NVM Subsystem Reset, offset: 0x20 */
    uint32_t    aqa;    /* admin queue attributes, offset: 0x24 */
    uint64_t    asq;    /* admin submission queue base addr, offset: 0x28 */
    uint64_t    acq;    /* admin completion queue base addr, offset: 0x30 */
    uint32_t    cmbloc; /* Controller Memory Buffer Location, offset: 0x38 */
    uint32_t    cmbsz;  /* Controller Memory Buffer Size, offset: 0x3C */
    uint32_t    bpinfo; /* Boot Partition Information, offset: 0x40 */
    uint32_t    bprsel; /* Boot Partition Read Select, offset: 0x44 */
    uint64_t    bpmbl;  /* Boot Partition Memory Buffer Location, offset: 0x48 */
    uint64_t    cmbmsc; /* Controller Memory Buffer Memory Space Control, offset: 0x50 */
    uint32_t    cmbsts; /* Controller Memory Buffer Status, offset: 0x58 */
    uint8_t     reserved3[3492]; /* 5Ch - DFFh, offset: 0x5C ~ 0xDFC (0x5C + 3492 - 1 = 0xDFC) */
    uint32_t    pmrcap; /* Persistent Memory Capabilities, offset: 0xE00 */
    uint32_t    pmrctl; /* Persistent Memory Region Control, offset: 0xE04 */
    uint32_t    pmrsts; /* Persistent Memory Region Status, offset: 0xE08 */
    uint32_t    pmrebs; /* Persistent Memory Region Elasticity Buffer Size, offset: 0xE0C */
    uint32_t    pmrswtp; /* Persistent Memory Region Sustained Write Throughput, offset: 0xE10 */
    uint32_t    pmrmsc_lo; /* Persistent Memory Region Controller Memory Space Control, offset: 0xE14 */
    uint32_t    pmrmsc_hi; /* offset: 0xE18 */
    uint8_t     reserved4[484]; /* E1Ch - FFFh, offset: 0xE1C ~ 0xFFC (0xE1C + 484 - 1 = 0xFFC) */
    struct {
        uint32_t    sq_tdbl; /* submission queue tail doorbell, offset base: 0x1000 (需结合 CAP.DSTRD 动态计算多队列，单队列示例 offset: 0x1000) */
        uint32_t    cq_hdbl; /* completion queue head doorbell, offset: 0x1004 (单队列示例，多队列按公式算) */
    } doorbell[1];
};

struct nvme_power_state {
	/** Maximum Power */
	uint16_t	mp;
	uint8_t		ps_rsvd1;

	/** Max Power Scale, Non-Operational State */
	uint8_t		mps_nops;

	/** Entry Latency */
	uint32_t	enlat;

	/** Exit Latency */
	uint32_t	exlat;

	/** Relative Read Throughput */
	uint8_t		rrt;

	/** Relative Read Latency */
	uint8_t		rrl;

	/** Relative Write Throughput */
	uint8_t		rwt;

	/** Relative Write Latency */
	uint8_t		rwl;

	/** Idle Power */
	uint16_t	idlp;

	/** Idle Power Scale */
	uint8_t		ips;
	uint8_t		ps_rsvd8;

	/** Active Power */
	uint16_t	actp;

	/** Active Power Workload, Active Power Scale */
	uint8_t		apw_aps;

	uint8_t		ps_rsvd10[9];
} __packed;



struct nvme_controller_data {
	/* bytes 0-255: controller capabilities and features */

	/** pci vendor id */
	uint16_t		vid;

	/** pci subsystem vendor id */
	uint16_t		ssvid;

#define NVME_SERIAL_NUMBER_LENGTH	20
#define NVME_MODEL_NUMBER_LENGTH	40
#define NVME_FIRMWARE_REVISION_LENGTH	8

	/** serial number */
	uint8_t			sn[NVME_SERIAL_NUMBER_LENGTH];

	/** model number */
	uint8_t			mn[NVME_MODEL_NUMBER_LENGTH];

	/** firmware revision */
	uint8_t			fr[NVME_FIRMWARE_REVISION_LENGTH];

	/** recommended arbitration burst */
	uint8_t			rab;

	/** ieee oui identifier */
	uint8_t			ieee[3];

	/** multi-interface capabilities */
	uint8_t			mic;

	/** maximum data transfer size */
	uint8_t			mdts;

	/** Controller ID */
	uint16_t		ctrlr_id;

	/** Version */
	uint32_t		ver;

	/** RTD3 Resume Latency */
	uint32_t		rtd3r;

	/** RTD3 Enter Latency */
	uint32_t		rtd3e;

	/** Optional Asynchronous Events Supported */
	uint32_t		oaes;	/* bitfield really */

	/** Controller Attributes */
	uint32_t		ctratt;	/* bitfield really */

	/** Read Recovery Levels Supported */
	uint16_t		rrls;

	uint8_t			reserved1[9];

	/** Controller Type */
	uint8_t			cntrltype;

	/** FRU Globally Unique Identifier */
	uint8_t			fguid[16];

	/** Command Retry Delay Time 1 */
	uint16_t		crdt1;

	/** Command Retry Delay Time 2 */
	uint16_t		crdt2;

	/** Command Retry Delay Time 3 */
	uint16_t		crdt3;

	uint8_t			reserved2[122];

	/* bytes 256-511: admin command set attributes */

	/** optional admin command support */
	uint16_t		oacs;

	/** abort command limit */
	uint8_t			acl;

	/** asynchronous event request limit */
	uint8_t			aerl;

	/** firmware updates */
	uint8_t			frmw;

	/** log page attributes */
	uint8_t			lpa;

	/** error log page entries */
	uint8_t			elpe;

	/** number of power states supported */
	uint8_t			npss;

	/** admin vendor specific command configuration */
	uint8_t			avscc;

	/** Autonomous Power State Transition Attributes */
	uint8_t			apsta;

	/** Warning Composite Temperature Threshold */
	uint16_t		wctemp;

	/** Critical Composite Temperature Threshold */
	uint16_t		cctemp;

	/** Maximum Time for Firmware Activation */
	uint16_t		mtfa;

	/** Host Memory Buffer Preferred Size */
	uint32_t		hmpre;

	/** Host Memory Buffer Minimum Size */
	uint32_t		hmmin;

	/** Name space capabilities  */
	struct {
		/* if nsmgmt, report tnvmcap and unvmcap */
		uint8_t    tnvmcap[16];
		uint8_t    unvmcap[16];
	} __packed untncap;

	/** Replay Protected Memory Block Support */
	uint32_t		rpmbs; /* Really a bitfield */

	/** Extended Device Self-test Time */
	uint16_t		edstt;

	/** Device Self-test Options */
	uint8_t			dsto; /* Really a bitfield */

	/** Firmware Update Granularity */
	uint8_t			fwug;

	/** Keep Alive Support */
	uint16_t		kas;

	/** Host Controlled Thermal Management Attributes */
	uint16_t		hctma; /* Really a bitfield */

	/** Minimum Thermal Management Temperature */
	uint16_t		mntmt;

	/** Maximum Thermal Management Temperature */
	uint16_t		mxtmt;

	/** Sanitize Capabilities */
	uint32_t		sanicap; /* Really a bitfield */

	/** Host Memory Buffer Minimum Descriptor Entry Size */
	uint32_t		hmminds;

	/** Host Memory Maximum Descriptors Entries */
	uint16_t		hmmaxd;

	/** NVM Set Identifier Maximum */
	uint16_t		nsetidmax;

	/** Endurance Group Identifier Maximum */
	uint16_t		endgidmax;

	/** ANA Transition Time */
	uint8_t			anatt;

	/** Asymmetric Namespace Access Capabilities */
	uint8_t			anacap;

	/** ANA Group Identifier Maximum */
	uint32_t		anagrpmax;

	/** Number of ANA Group Identifiers */
	uint32_t		nanagrpid;

	/** Persistent Event Log Size */
	uint32_t		pels;

	uint8_t			reserved3[156];
	/* bytes 512-703: nvm command set attributes */

	/** submission queue entry size */
	uint8_t			sqes;

	/** completion queue entry size */
	uint8_t			cqes;

	/** Maximum Outstanding Commands */
	uint16_t		maxcmd;

	/** number of namespaces */
	uint32_t		nn;

	/** optional nvm command support */
	uint16_t		oncs;

	/** fused operation support */
	uint16_t		fuses;

	/** format nvm attributes */
	uint8_t			fna;

	/** volatile write cache */
	uint8_t			vwc;

	/** Atomic Write Unit Normal */
	uint16_t		awun;

	/** Atomic Write Unit Power Fail */
	uint16_t		awupf;

	/** NVM Vendor Specific Command Configuration */
	uint8_t			nvscc;

	/** Namespace Write Protection Capabilities */
	uint8_t			nwpc;

	/** Atomic Compare & Write Unit */
	uint16_t		acwu;
	uint16_t		reserved6;

	/** SGL Support */
	uint32_t		sgls;

	/** Maximum Number of Allowed Namespaces */
	uint32_t		mnan;

	/* bytes 540-767: Reserved */
	uint8_t			reserved7[224];

	/** NVM Subsystem NVMe Qualified Name */
	uint8_t			subnqn[256];

	/* bytes 1024-1791: Reserved */
	uint8_t			reserved8[768];

	/* bytes 1792-2047: NVMe over Fabrics specification */
	uint8_t			reserved9[256];

	/* bytes 2048-3071: power state descriptors */
	struct nvme_power_state power_state[32];

	/* bytes 3072-4095: vendor specific */
	uint8_t			vs[1024];
} __packed __aligned(4);



static inline 	void nvme_controller_data_swapbytes(struct nvme_controller_data *s)
{

#if _BYTE_ORDER != _LITTLE_ENDIAN
	s->vid = sys_le16_to_cpu(s->vid);
	s->ssvid = sys_le16_to_cpu(s->ssvid);
	s->ctrlr_id = sys_le16_to_cpu(s->ctrlr_id);
	s->ver = sys_le32_to_cpu(s->ver);
	s->rtd3r = sys_le32_to_cpu(s->rtd3r);
	s->rtd3e = sys_le32_to_cpu(s->rtd3e);
	s->oaes = sys_le32_to_cpu(s->oaes);
	s->ctratt = sys_le32_to_cpu(s->ctratt);
	s->rrls = sys_le16_to_cpu(s->rrls);
	s->crdt1 = sys_le16_to_cpu(s->crdt1);
	s->crdt2 = sys_le16_to_cpu(s->crdt2);
	s->crdt3 = sys_le16_to_cpu(s->crdt3);
	s->oacs = sys_le16_to_cpu(s->oacs);
	s->wctemp = sys_le16_to_cpu(s->wctemp);
	s->cctemp = sys_le16_to_cpu(s->cctemp);
	s->mtfa = sys_le16_to_cpu(s->mtfa);
	s->hmpre = sys_le32_to_cpu(s->hmpre);
	s->hmmin = sys_le32_to_cpu(s->hmmin);
	s->rpmbs = sys_le32_to_cpu(s->rpmbs);
	s->edstt = sys_le16_to_cpu(s->edstt);
	s->kas = sys_le16_to_cpu(s->kas);
	s->hctma = sys_le16_to_cpu(s->hctma);
	s->mntmt = sys_le16_to_cpu(s->mntmt);
	s->mxtmt = sys_le16_to_cpu(s->mxtmt);
	s->sanicap = sys_le32_to_cpu(s->sanicap);
	s->hmminds = sys_le32_to_cpu(s->hmminds);
	s->hmmaxd = sys_le16_to_cpu(s->hmmaxd);
	s->nsetidmax = sys_le16_to_cpu(s->nsetidmax);
	s->endgidmax = sys_le16_to_cpu(s->endgidmax);
	s->anagrpmax = sys_le32_to_cpu(s->anagrpmax);
	s->nanagrpid = sys_le32_to_cpu(s->nanagrpid);
	s->pels = sys_le32_to_cpu(s->pels);
	s->maxcmd = sys_le16_to_cpu(s->maxcmd);
	s->nn = sys_le32_to_cpu(s->nn);
	s->oncs = sys_le16_to_cpu(s->oncs);
	s->fuses = sys_le16_to_cpu(s->fuses);
	s->awun = sys_le16_to_cpu(s->awun);
	s->awupf = sys_le16_to_cpu(s->awupf);
	s->acwu = sys_le16_to_cpu(s->acwu);
	s->sgls = sys_le32_to_cpu(s->sgls);
	s->mnan = sys_le32_to_cpu(s->mnan);
#else
	ARG_UNUSED(s);
#endif
}

#define NVME_PCIE_BAR_IDX 0

#define NVME_REQUEST_AMOUNT (CONFIG_NVME_ADMIN_ENTRIES +	\
			     CONFIG_NVME_IO_ENTRIES)

/* admin queue + io queue(s) */
#define NVME_PCIE_MSIX_VECTORS 1 + CONFIG_NVME_IO_QUEUES

#define NVME_QUEUE_ALLOCATE(name, n_entries)				\
	static struct nvme_command cmd_##name[n_entries] __aligned(0x1000); \
	static struct nvme_completion cpl_##name[n_entries] __aligned(0x1000); \
									\
	static struct nvme_cmd_qpair name = {				\
		.num_entries = n_entries,				\
		.cmd = cmd_##name,					\
		.cpl = cpl_##name,					\
	}

#define NVME_ADMINQ_ALLOCATE(n, n_entries)		\
	NVME_QUEUE_ALLOCATE(admin_##n, n_entries)
#define NVME_IOQ_ALLOCATE(n, n_entries)			\
	NVME_QUEUE_ALLOCATE(io_##n, n_entries)

struct FNvmeMsixVector
{
    u32 irq;
	u32 address;
	u16 eventid;
} ;

typedef struct nvme_controller nvme_controller_t;
typedef u32 (*nvme_msi_x_setup)(nvme_controller_t *ctrlr,u32 max_vec,u32 start_flgs) ;
typedef void (*nvme_interrupt_setup)(u32 vector_idx,void *irq_fun,void *irq_arg,u32 start_flgs);

struct nvme_controller {
	uintptr base ; /* bar address */

	struct disk_info *disk;
	uint32_t id;
	struct nvme_controller_data cdata;

	uint32_t num_io_queues;
	struct nvme_cmd_qpair *adminq;
	struct nvme_cmd_qpair *ioq;

	uint32_t ready_timeout_in_ms;

	/** LO and HI capacity mask */
	uint32_t cap_lo;
	uint32_t cap_hi;

	/** Page size and log2(page_size) - 12 that we're currently using */
	uint32_t page_size;
	uint32_t mps;

	/** doorbell stride */
	uint32_t dstrd;

	/** maximum i/o size in bytes */
	uint32_t max_xfer_size;

	struct nvme_namespace ns[CONFIG_NVME_MAX_NAMESPACES];

	/* cmd */
	struct nvme_prp_list prp_list_pool[CONFIG_NVME_PRP_LIST_AMOUNT];
	sys_dlist_t free_prp_list;

	struct nvme_request request_pool[NVME_REQUEST_AMOUNT];
	sys_dlist_t free_request;
	sys_dlist_t pending_request;

	/* interrupt callback */
	nvme_interrupt_setup interrupt_setup_fun;
	nvme_msi_x_setup msi_x_setup ;
	/* msi-x   */
	struct FNvmeMsixVector msix_vectors[NVME_PCIE_MSIX_VECTORS];


};

static inline
boolean nvme_controller_has_dataset_mgmt(struct nvme_controller *ctrlr)
{
	/* Assumes cd was byte swapped by nvme_controller_data_swapbytes() */
	return ((ctrlr->cdata.oncs >> NVME_CTRLR_DATA_ONCS_DSM_SHIFT) &
		NVME_CTRLR_DATA_ONCS_DSM_MASK);
}



int nvme_controller_init(struct nvme_controller *nvme_ctrlr,nvme_msi_x_setup msi_x_setup ,nvme_interrupt_setup interrupt_setup_fun) ;


#endif /* NVME_H */
