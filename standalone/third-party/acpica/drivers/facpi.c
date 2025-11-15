/*
 * Copyright : (C) 2025 Phytium Information Technology, Inc.
 * All Rights Reserved.
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
 * Modified from NetBSD sys/dev/acpi/acpi.c with NETBSD LICENSE
 *
 * FilePath: facpi.c
 * Date: 2025-04-10 16:20:52
 * LastEditTime: 2025-04-10 16:20:52
 * Description:  This file is for acpi user interface definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2025/7/30    init commit
 */
/***************************** Include Files *********************************/
#include "fdrivers_port.h"
#include "facpi.h"
#include "facpi_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FACPI_DEBUG_TAG "FACPI"
#define FACPI_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_WARN(format, ...) FT_DEBUG_PRINT_W(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_INFO(format, ...) FT_DEBUG_PRINT_I(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_VERBOS(format, ...) \
    FT_DEBUG_PRINT_V(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/
static FError FAcpiAcpicaEnable(FAcpi *instance);
static FError FAcpiAcpicaBuildtree(FAcpi *instance);

/*****************************************************************************/
FError FAcpiCfgInitialize(FAcpi *instance, const FAcpiConfig *config)
{
    FASSERT(instance && config);
    FError ret = FACPI_SUCCESS;

    if (FT_COMPONENT_IS_READY == instance->is_ready)
    {
        FACPI_WARN("Device is already initialized!!!");
    }

    if (&instance->config != config)
    {
        instance->config = *config;
    }

    ret = FAcpiAcpicaEnable(instance);
    if (FACPI_SUCCESS != ret)
    {
        return ret;
    }

    ret = FAcpiAcpicaBuildtree(instance);
    if (FACPI_SUCCESS != ret)
    {
        return ret;
    }

    /*
	 * Update GPE information.
	 *
	 * Note that this must be called after
	 * all GPE handlers have been installed.
	 */
    (void)AcpiUpdateAllGpes();

    if (FACPI_SUCCESS == ret)
    {
        instance->is_ready = FT_COMPONENT_IS_READY;
        FACPI_INFO("Device initialize success !!!");
    }

    return ret;
}

void FAcpiDeInitialize(FAcpi *instance)
{
    AcpiTerminate();
    instance->is_ready = 0;
}

static FError FAcpiAcpicaEnable(FAcpi *instance)
{
    FError ret = FACPI_SUCCESS;
    ACPI_TABLE_HEADER *rsdt;
    ACPI_STATUS rv;

    if (AcpiOsGetRootPointer() != instance->config.rsdp_root)
    {
        FACPI_ERROR("Invalid RSDP root pointer: %p, expected: %p",
                    (void *)AcpiOsGetRootPointer(), (void *)instance->config.rsdp_root);
        return FACPI_ERR_RSDP_INVALID;
    }

    /*
	 * Start up ACPICA.
	 */
    AcpiGbl_EnableInterpreterSlack = true;

    rv = AcpiInitializeSubsystem();
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("%s: failed to initialize subsystem", __func__);
        return FACPI_ERR_ACPICA_INIT_FAILED;
    }

    /*
	 * Allocate space for RSDT/XSDT and DSDT,
	 * but allow resizing if more tables exist.
	 */
    rv = AcpiInitializeTables(NULL, 16, FALSE);
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("%s: failed to initialize tables", __func__);
        ret = FACPI_ERR_ACPICA_INIT_FAILED;
        goto fail;
    }

    rv = AcpiLoadTables();
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("%s: failed to load tables", __func__);
        ret = FACPI_ERR_ACPICA_INIT_FAILED;
        goto fail;
    }

    rsdt = acpi_map_rsdt(instance);
    if (rsdt == NULL)
    {
        FACPI_ERROR("%s: failed to map RSDT", __func__);
        ret = FACPI_ERR_ACPICA_INIT_FAILED;
        goto fail;
    }

    FACPI_INFO("Intel ACPICA %08x", ACPI_CA_VERSION);
    FACPI_INFO("ACPI: X/RSDT: OemId <%6.6s,%8.8s,%08x>, "
               "AslId <%4.4s,%08x>",
               rsdt->OemId, rsdt->OemTableId, rsdt->OemRevision, rsdt->AslCompilerId,
               rsdt->AslCompilerRevision);

    /*
	 * Bring ACPICA on-line.
	 */
    rv = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("%s: failed to online subsystem", __func__);
        ret = FACPI_ERR_ACPICA_INIT_FAILED;
        goto fail;
    }

    rv = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("%s: failed to initialize objects", __func__);
        ret = FACPI_ERR_ACPICA_INIT_FAILED;
        goto fail;
    }

    rv = acpi_madt_map(instance);
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("%s: failed to map madt", __func__);
        ret = FACPI_ERR_ACPICA_INIT_FAILED;
        goto fail;
    }

    rv = acpi_gtdt_map(instance);
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("%s: failed to map gtdt", __func__);
        ret = FACPI_ERR_ACPICA_INIT_FAILED;
        goto fail;
    }

    return ret;
fail:
    (void)AcpiTerminate();
    return ret;
}

static FError FAcpiAcpicaBuildtree(FAcpi *instance)
{
    FAcpiWalkContext awc;

    /*
	 * Get the root scope handles.
	 */
    FASSERT(ARRAY_SIZE(instance->acpi_scopes) == 4);

    (void)AcpiGetHandle(ACPI_ROOT_OBJECT, "\\_PR_", &instance->acpi_scopes[0]);
    (void)AcpiGetHandle(ACPI_ROOT_OBJECT, "\\_SB_", &instance->acpi_scopes[1]);
    (void)AcpiGetHandle(ACPI_ROOT_OBJECT, "\\_SI_", &instance->acpi_scopes[2]);
    (void)AcpiGetHandle(ACPI_ROOT_OBJECT, "\\_TZ_", &instance->acpi_scopes[3]);

    /*
	 * Make the root node.
	 */
    awc.acpi = instance;
    (void)acpi_make_devnode(ACPI_ROOT_OBJECT, 0, &awc, NULL);

    /*
	 * Build the internal namespace.
	 */
    awc.parent = &instance->dev_nodes[0];
    (void)AcpiWalkNamespace(ACPI_TYPE_ANY, ACPI_ROOT_OBJECT, UINT32_MAX,
                            acpi_make_devnode, NULL, &awc, NULL);

    return FACPI_SUCCESS;
}