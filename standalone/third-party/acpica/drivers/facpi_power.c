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
 * Modified from NetBSD sys/dev/acpi/acpi_power.c with NETBSD LICENSE
 *
 * FilePath: facpi_power.c
 * Date: 2025-04-10 16:20:52
 * LastEditTime: 2025-04-10 16:20:52
 * Description:  This file is for acpi power device functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2025/7/30    init commit
 */
/***************************** Include Files *********************************/
#include <limits.h>
#include "fdrivers_port.h"
#include "facpi.h"
#include "facpi_util.h"

/************************** Constant Definitions *****************************/
#define _COMPONENT 0
ACPI_MODULE_NAME("RESOURCE")

#define ACPI_STATE_ERROR   -1

/*
 * ACPI driver capabilities (ad_flags).
 */
#define ACPI_DEVICE_POWER  BIT(0) /* Support for D-states  */
#define ACPI_DEVICE_WAKEUP BIT(1) /* Support for wake-up */
#define ACPI_DEVICE_EJECT  BIT(2) /* Support for "ejection" */
#define ACPI_DEVICE_DOCK   BIT(3) /* Support for docking */

#define ACPI_STA_POW_OFF   0x00
#define ACPI_STA_POW_ON    0x01
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FACPI_DEBUG_TAG    "FACPI"
#define FACPI_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_WARN(format, ...) FT_DEBUG_PRINT_W(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_INFO(format, ...) FT_DEBUG_PRINT_I(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_VERBOS(format, ...) \
    FT_DEBUG_PRINT_V(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/
static FAcpiPowerResource *acpi_power_res_init(FAcpi *, ACPI_HANDLE);
static FAcpiPowerResource *acpi_power_res_get(FAcpi *, ACPI_HANDLE);
static ACPI_STATUS acpi_power_get_direct(FAcpiDevNode *);
static ACPI_STATUS acpi_power_get_indirect(FAcpiDevNode *);
static ACPI_STATUS acpi_power_switch(FAcpiDevNode *, int, bool);
static ACPI_STATUS acpi_power_res_ref(FAcpiPowerResource *, ACPI_HANDLE);
static ACPI_STATUS acpi_power_res_deref(FAcpiPowerResource *, ACPI_HANDLE);
static ACPI_STATUS acpi_power_res_sta(ACPI_OBJECT *, void *);

static ACPI_OBJECT *acpi_power_pkg_get(ACPI_HANDLE, int);
static const char *acpi_xname(ACPI_HANDLE);
/*****************************************************************************/
static FAcpiPowerResource *acpi_power_res_init(FAcpi *acpi, ACPI_HANDLE hdl)
{
    FAcpiPowerResource *tmp = NULL;
    FAcpiPowerResource *res = NULL;
    ACPI_OBJECT *obj;
    ACPI_BUFFER buf;
    ACPI_STATUS rv;
    size_t i;

    rv = acpi_eval_struct(hdl, NULL, &buf);

    if (ACPI_FAILURE(rv))
    {
        goto out;
    }

    obj = buf.Pointer;

    if (obj->Type != ACPI_TYPE_POWER)
    {
        rv = AE_TYPE;
        goto out;
    }

    res = &(acpi->res_head[acpi->max_pwr_res++]);
    res->res_handle = hdl;
    res->res_level = obj->PowerResource.SystemLevel;
    res->res_order = obj->PowerResource.ResourceOrder;

    (void)strlcpy(res->res_name, acpi_xname(hdl), sizeof(res->res_name));

    for (i = 0; i < ARRAY_SIZE(res->res_ref); i++)
    {
        res->res_ref[i] = NULL;
    }

    ACPI_DEBUG_PRINT((ACPI_DB_INFO,
                      "%s added to the "
                      "power resource queue\n",
                      res->res_name));

out:
    if (buf.Pointer != NULL)
    {
        ACPI_FREE(buf.Pointer);
    }

    return res;
}

static FAcpiPowerResource *acpi_power_res_get(FAcpi *acpi, ACPI_HANDLE hdl)
{
    FAcpiPowerResource *res;

    for (u32 i = 0; i < FACPI_MAX_PWR_RES_NUM; i++)
    {
        res = &(acpi->res_head[i]);
        if ((res->res_handle != 0) && (res->res_handle == hdl))
        {
            return res;
        }
    }

    return acpi_power_res_init(acpi, hdl);
}

bool acpi_power_register(ACPI_HANDLE hdl)
{
    return true;
}

void acpi_power_deregister(FAcpi *acpi, ACPI_HANDLE hdl)
{
    FAcpiDevNode *ad = acpi_match_node(hdl);
    FAcpiPowerResource *res;

    if (ad == NULL)
    {
        return;
    }

    /*
	 * Remove all references in each resource.
	 */
    for (u32 i = 0; i < FACPI_MAX_PWR_RES_NUM; i++)
    {
        res = &(acpi->res_head[i]);
        if (res->res_handle != 0)
        {
            (void)acpi_power_res_deref(res, ad->ad_handle);
        }
    }
    acpi->max_pwr_res = 0;
}

/*
 * Get the D-state of an ACPI device node.
 */
bool acpi_power_get(ACPI_HANDLE hdl, int *state)
{
    FAcpiDevNode *ad = acpi_match_node(hdl);
    ACPI_STATUS rv;

    if (ad == NULL)
    {
        return false;
    }

    /*
	 * As _PSC may be broken, first try to
	 * retrieve the power state indirectly
	 * via power resources.
	 */
    rv = acpi_power_get_indirect(ad);

    if (ACPI_FAILURE(rv))
    {
        rv = acpi_power_get_direct(ad);
    }

    if (ACPI_FAILURE(rv))
    {
        goto fail;
    }

    FASSERT(ad->ad_state != ACPI_STATE_ERROR);

    if (ad->ad_state < ACPI_STATE_D0 || ad->ad_state > ACPI_STATE_D3)
    {
        rv = AE_BAD_VALUE;
        goto fail;
    }

    if (state != NULL)
    {
        *state = ad->ad_state;
    }

    return true;

fail:
    ad->ad_state = ACPI_STATE_ERROR;

    if (state != NULL)
    {
        *state = ad->ad_state;
    }

    ACPI_DEBUG_PRINT((ACPI_DB_INFO,
                      "failed to get power state "
                      "for %s: %s\n",
                      ad->ad_name, AcpiFormatException(rv)));

    return false;
}

static ACPI_STATUS acpi_power_get_direct(FAcpiDevNode *ad)
{
    ACPI_INTEGER val = 0;
    ACPI_STATUS rv;

    rv = acpi_eval_integer(ad->ad_handle, "_PSC", &val);

    FASSERT((uint64_t)val < INT_MAX);

    ad->ad_state = (int)val;

    return rv;
}

static ACPI_STATUS acpi_power_get_indirect(FAcpiDevNode *ad)
{
    ACPI_OBJECT *pkg;
    ACPI_STATUS rv;
    int i;

    FASSERT(ACPI_STATE_D0 == 0 && ACPI_STATE_D1 == 1);
    FASSERT(ACPI_STATE_D2 == 2 && ACPI_STATE_D3 == 4);

    /*
	 * The device is in a given D-state if all resources are on.
	 * To derive this, evaluate all elements in each _PRx package
	 * (x = 0 ... 3) and break if the noted condition becomes true.
	 */
    for (ad->ad_state = ACPI_STATE_D3, i = 0; i < ACPI_STATE_D3; i++)
    {

        pkg = acpi_power_pkg_get(ad->ad_handle, i);

        if (pkg == NULL)
        {
            continue;
        }

        /*
		 * For each element in the _PRx package, evaluate _STA
		 * and return AE_OK only if all power resources are on.
		 */
        rv = acpi_foreach_package_object(pkg, acpi_power_res_sta, ad);

        if (ACPI_FAILURE(rv) && rv != AE_CTRL_FALSE)
        {
            goto out;
        }

        if (ACPI_SUCCESS(rv))
        {
            ad->ad_state = i;
            goto out;
        }

        ACPI_FREE(pkg);
        pkg = NULL;
    }

    FASSERT(ad->ad_state == ACPI_STATE_D3);

    return AE_OK;

out:
    ACPI_FREE(pkg);

    return rv;
}

/*
 * Set the D-state of an ACPI device node.
 */
bool acpi_power_set(ACPI_HANDLE hdl, int state)
{
    FAcpiDevNode *ad = acpi_match_node(hdl);
    ACPI_STATUS rv;
    char path[5];
    int old;

    if (ad == NULL)
    {
        return false;
    }

    if (state < ACPI_STATE_D0 || state > ACPI_STATE_D3)
    {
        rv = AE_BAD_PARAMETER;
        goto fail;
    }

    if (acpi_power_get(ad->ad_handle, &old) != true)
    {
        rv = AE_NOT_FOUND;
        goto fail;
    }

    FASSERT(ad->ad_state == old);
    FASSERT(ad->ad_state != ACPI_STATE_ERROR);

    if (ad->ad_state == state)
    {
        rv = AE_ALREADY_EXISTS;
        goto fail;
    }

    /*
	 * It is only possible to go to D0 ("on") from D3 ("off").
	 */
    if (ad->ad_state == ACPI_STATE_D3 && state != ACPI_STATE_D0)
    {
        rv = AE_BAD_PARAMETER;
        goto fail;
    }

    /*
	 * We first sweep through the resources required for the target
	 * state, turning things on and building references. After this
	 * we dereference the resources required for the current state,
	 * turning the resources off as we go.
	 */
    rv = acpi_power_switch(ad, state, true);

    if (ACPI_FAILURE(rv) && rv != AE_CTRL_CONTINUE)
    {
        goto fail;
    }

    rv = acpi_power_switch(ad, ad->ad_state, false);

    if (ACPI_FAILURE(rv) && rv != AE_CTRL_CONTINUE)
    {
        goto fail;
    }

    /*
	 * Last but not least, invoke the power state switch method,
	 * if available. Because some systems use only _PSx for the
	 * power state transitions, we do this even if there is no _PRx.
	 */
    (void)snprintf(path, sizeof(path), "_PS%d", state);
    (void)AcpiEvaluateObject(ad->ad_handle, path, NULL, NULL);

    ACPI_DEBUG_PRINT((ACPI_DB_INFO,
                      "%s turned from "
                      "D%d to D%d\n",
                      ad->ad_name, old, state));

    ad->ad_state = state;

    return true;

fail:
    ad->ad_state = ACPI_STATE_ERROR;

    ACPI_DEBUG_PRINT((ACPI_DB_INFO,
                      "failed to set power state to D%d "
                      "for %s: %s\n",
                      state, ad->ad_name, AcpiFormatException(rv)));

    return false;
}

static ACPI_STATUS acpi_power_switch(FAcpiDevNode *ad, int state, bool on)
{
    ACPI_OBJECT *elm, *pkg;
    ACPI_STATUS rv = AE_OK;
    ACPI_HANDLE hdl;
    uint32_t i, n;

    /*
	 * For each element in the _PRx package, fetch
	 * the reference handle, search for this handle
	 * from the power resource queue, and turn the
	 * resource behind the handle on or off.
	 */
    pkg = acpi_power_pkg_get(ad->ad_handle, state);

    if (pkg == NULL)
    {
        return AE_CTRL_CONTINUE;
    }

    n = pkg->Package.Count;

    for (i = 0; i < n; i++)
    {

        elm = &pkg->Package.Elements[i];
        rv = acpi_eval_reference_handle(elm, &hdl);

        if (ACPI_FAILURE(rv))
        {
            continue;
        }

        (void)acpi_power_res(ad->acpi, hdl, ad->ad_handle, on);
    }

    ACPI_FREE(pkg);

    return rv;
}

ACPI_STATUS
acpi_power_res(FAcpi *acpi, ACPI_HANDLE hdl, ACPI_HANDLE ref, bool on)
{
    FAcpiPowerResource *res;
    char *str;
    ACPI_STATUS rv;

    /*
	 * Search for the resource.
	 */
    res = acpi_power_res_get(acpi, hdl);

    if (res == NULL)
    {
        return AE_NOT_FOUND;
    }

    if (ref == NULL)
    {
        return AE_BAD_PARAMETER;
    }

    /*
	 * Adjust the reference counting. This is
	 * necessary since a single power resource
	 * can be shared by multiple devices.
	 */
    if (on)
    {
        rv = acpi_power_res_ref(res, ref);
        str = "_ON";
    }
    else
    {
        rv = acpi_power_res_deref(res, ref);
        str = "_OFF";
    }

    if (ACPI_FAILURE(rv))
    {
        return rv;
    }

    /*
	 * Turn the resource on or off.
	 */
    return AcpiEvaluateObject(res->res_handle, str, NULL, NULL);
}

static ACPI_STATUS acpi_power_res_ref(FAcpiPowerResource *res, ACPI_HANDLE ref)
{
    size_t i, j = SIZE_MAX;

    for (i = 0; i < ARRAY_SIZE(res->res_ref); i++)
    {

        /*
		 * Do not error out if the handle
		 * has already been referenced.
		 */
        if (res->res_ref[i] == ref)
        {
            return AE_OK;
        }

        if (j == SIZE_MAX && res->res_ref[i] == NULL)
        {
            j = i;
        }
    }

    if (j == SIZE_MAX)
    {
        return AE_LIMIT;
    }

    res->res_ref[j] = ref;

    ACPI_DEBUG_PRINT((ACPI_DB_INFO,
                      "%s referenced "
                      "by %s\n",
                      res->res_name, acpi_xname(ref)));

    return AE_OK;
}

static ACPI_STATUS acpi_power_res_deref(FAcpiPowerResource *res, ACPI_HANDLE ref)
{
    size_t i;

    for (i = 0; i < ARRAY_SIZE(res->res_ref); i++)
    {

        if (res->res_ref[i] != ref)
        {
            continue;
        }

        res->res_ref[i] = NULL;

        ACPI_DEBUG_PRINT((ACPI_DB_INFO,
                          "%s dereferenced "
                          "by %s\n",
                          res->res_name, acpi_xname(ref)));

        return AE_OK;
    }

    return AE_ABORT_METHOD;
}

static ACPI_STATUS acpi_power_res_sta(ACPI_OBJECT *elm, void *arg)
{
    ACPI_INTEGER val;
    ACPI_HANDLE hdl;
    ACPI_STATUS rv;

    rv = acpi_eval_reference_handle(elm, &hdl);

    if (ACPI_FAILURE(rv))
    {
        goto fail;
    }

    rv = acpi_eval_integer(hdl, "_STA", &val);

    if (ACPI_FAILURE(rv))
    {
        goto fail;
    }

    FASSERT((uint64_t)val < INT_MAX);

    if ((int)val != ACPI_STA_POW_ON && (int)val != ACPI_STA_POW_OFF)
    {
        return AE_BAD_VALUE;
    }

    if ((int)val != ACPI_STA_POW_ON)
    {
        return AE_CTRL_FALSE; /* XXX: Not an error. */
    }

    return AE_OK;

fail:
    if (rv == AE_CTRL_FALSE)
    {
        rv = AE_ERROR;
    }

    ACPI_DEBUG_PRINT((ACPI_DB_DEBUG_OBJECT,
                      "failed to evaluate _STA "
                      "for %s: %s\n",
                      acpi_xname(hdl), AcpiFormatException(rv)));

    return rv;
}

static ACPI_OBJECT *acpi_power_pkg_get(ACPI_HANDLE hdl, int state)
{
    char path[5] = "_PR?";
    ACPI_OBJECT *obj;
    ACPI_BUFFER buf;
    ACPI_STATUS rv;

    path[3] = '0' + state;

    rv = acpi_eval_struct(hdl, path, &buf);

    if (ACPI_FAILURE(rv))
    {
        goto fail;
    }

    if (buf.Length == 0)
    {
        rv = AE_LIMIT;
        goto fail;
    }

    obj = buf.Pointer;

    if (obj->Type != ACPI_TYPE_PACKAGE)
    {
        rv = AE_TYPE;
        goto fail;
    }

    if (obj->Package.Count == 0)
    {
        rv = AE_LIMIT;
        goto fail;
    }

    return obj;

fail:
    if (buf.Pointer != NULL)
    {
        ACPI_FREE(buf.Pointer);
    }

    ACPI_DEBUG_PRINT((ACPI_DB_DEBUG_OBJECT,
                      "failed to evaluate %s for "
                      "%s: %s\n",
                      path, acpi_xname(hdl), AcpiFormatException(rv)));

    return NULL;
}

void acpi_power_add(FAcpiDevNode *ad)
{
}

/*
 * XXX: Move this to acpi_util.c by refactoring
 *	acpi_name_netbsd() to optionally return a single name.
 */
static const char *acpi_xname(ACPI_HANDLE hdl)
{
    static char str[5];
    ACPI_BUFFER buf;
    ACPI_STATUS rv;

    buf.Pointer = str;
    buf.Length = sizeof(str);

    rv = AcpiGetName(hdl, ACPI_SINGLE_NAME, &buf);

    if (ACPI_FAILURE(rv))
    {
        return "????";
    }

    return str;
}