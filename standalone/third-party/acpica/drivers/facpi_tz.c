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
 * Modified from NetBSD sys/dev/acpi/acpi_tz.c with NETBSD LICENSE
 *
 * FilePath: facpi_tz.c
 * Date: 2025-04-10 16:20:52
 * LastEditTime: 2025-04-10 16:20:52
 * Description:  This file is for acpi thermal zone definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2025/7/30    init commit
 */
/***************************** Include Files *********************************/
#include <stdlib.h>
#include "fdrivers_port.h"
#include "facpi.h"
#include "facpi_util.h"

/************************** Constant Definitions *****************************/
#define _COMPONENT 0
ACPI_MODULE_NAME("RESOURCE")

#define ACPI_NOTIFY_TZ_ZONE    0x80
#define ACPI_NOTIFY_TZ_TRIP    0x81
#define ACPI_NOTIFY_TZ_DEVLIST 0x82

#define ATZ_F_CRITICAL         0x01 /* zone critical */
#define ATZ_F_HOT              0x02 /* zone hot */
#define ATZ_F_PASSIVE          0x04 /* zone passive cooling */
#define ATZ_F_PASSIVEONLY      0x08 /* zone is passive cooling only */

#define ATZ_ACTIVE_NONE        -1

/*
 * The constants are as follows:
 *
 *   ATZ_TZP_RATE	default polling interval (30 seconds) if no _TZP
 *   ATZ_NLEVELS	number of cooling levels for _ACx and _ALx
 *   ATZ_ZEROC		0 C, measured in 0.1 Kelvin
 *   ATZ_TMP_INVALID	temporarily invalid temperature
 *   ATZ_ZONE_EXPIRE	zone info refetch interval (15 minutes)
 */
#define ATZ_TZP_RATE           300
#define ATZ_NLEVELS            10
#define ATZ_ZEROC              2732
#define ATZ_TMP_INVALID        0xffffffff
#define ATZ_ZONE_EXPIRE        9000

/*
 * All temperatures are reported in 0.1 Kelvin.
 * The ACPI specification assumes that K = C + 273.2
 * rather than the nominal 273.15 used by envsys(4).
 */
#define ATZ2UKELVIN(t)         ((t)*100000 - 50000)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FACPI_DEBUG_TAG        "FACPI"
#define FACPI_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_WARN(format, ...) FT_DEBUG_PRINT_W(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_INFO(format, ...) FT_DEBUG_PRINT_I(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_VERBOS(format, ...) \
    FT_DEBUG_PRINT_V(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/
static void acpitz_get_status(void *);
static void acpitz_get_zone(void *, int);
static void acpitz_get_zone_quiet(void *);
static char *acpitz_celcius_string(int);
static void acpitz_poll_status(FAcpiTz *tz, uint32_t *tmp);
static void acpitz_power_off(FAcpiTz *);
static void acpitz_power_zone(FAcpiTz *, int, int);
static void acpitz_sane_temp(uint32_t *tmp);
static ACPI_STATUS acpitz_switch_cooler(ACPI_OBJECT *, void *);
static void acpitz_notify_handler(ACPI_HANDLE, uint32_t, void *);
static int acpitz_get_integer(FAcpiTz *, char *, uint32_t *);
/*****************************************************************************/
FError FAcpiTzInitialize(FAcpi *acpi, FAcpiTz *tz)
{
    FASSERT(acpi && tz);
    FError ret = FACPI_SUCCESS;
    FAcpiDevNode *ad;
    ACPI_INTEGER val;
    ACPI_STATUS rv;

    for (u32 i = 0; i < acpi->max_dev_node; i++)
    {
        ad = &acpi->dev_nodes[i];
        if (ad->ad_devinfo->Type != ACPI_TYPE_THERMAL)
        {
            continue;
        }

        tz->node = ad;
        tz->acpi = acpi;
        break;
    }

    if ((tz->acpi == NULL) || (tz->node == NULL))
    {
        FACPI_ERROR("No thermal device found");
        return FACPI_ERR_TZ_INIT_FAILED;
    }

    tz->first = TRUE;
    tz->have_fan = FALSE;

    /*
	 * The _TZP (ACPI 4.0, p. 430) defines the recommended
	 * polling interval (in tenths of seconds). A value zero
	 * means that polling "should not be necessary".
	 */
    rv = acpi_eval_integer(tz->node->ad_handle, "_TZP", &val);
    if (ACPI_SUCCESS(rv) && val != 0)
    {
        tz->zone.tzp = val;
    }

    FACPI_INFO("polling interval %d.%d seconds", tz->zone.tzp / 10, tz->zone.tzp % 10);

    tz->zone_expire = ATZ_ZONE_EXPIRE / tz->zone.tzp;

    acpitz_get_zone(tz, 1);
    acpitz_get_status(tz);

    (void)acpi_power_register(tz->node->ad_handle);

    return ret;
}

void FAcpiTzDeInitialize(FAcpiTz *tz)
{
    ACPI_HANDLE hdl;
    ACPI_BUFFER al;
    ACPI_STATUS rv;

    /*
	 * Although the device itself should not contain any power
	 * resources, we have possibly used the resources of active
	 * cooling devices. To unregister these, first fetch a fresh
	 * active cooling zone, and then detach the resources from
	 * the reference handles contained in the cooling zone.
	 */
    acpitz_get_zone(tz, 0);

    for (u32 i = 0; i < ATZ_NLEVELS; i++)
    {

        if (tz->zone.al[i].Pointer == NULL)
        {
            continue;
        }

        al = tz->zone.al[i];
        rv = acpi_eval_reference_handle(al.Pointer, &hdl);

        if (ACPI_SUCCESS(rv))
        {
            acpi_power_deregister(tz->acpi, hdl);
        }

        ACPI_FREE(tz->zone.al[i].Pointer);
    }
}

FError FAcpiTzGetTemperature(FAcpiTz *tz, u32 *temperature)
{
    acpitz_poll_status(tz, temperature);
    return FACPI_SUCCESS;
}

static void acpitz_get_zone_quiet(void *dev)
{
    FAcpiTz *tz = dev;
    acpitz_get_zone(tz, 0);
}

static void acpitz_get_status(void *dev)
{
    FAcpiTz *tz = dev;
    uint32_t tmp, fmin, fmax, fcurrent;
    int active, changed, flags, i;

    tz->zone_expire--;

    if (tz->zone_expire <= 0)
    {
        tz->zone_expire = ATZ_ZONE_EXPIRE / tz->zone.tzp;

        ACPI_DEBUG_PRINT((ACPI_DB_INFO, "%s: zone refetch forced\n", "tz"));

        acpitz_get_zone(tz, 0);
    }

    if (acpitz_get_integer(tz, "_TMP", &tmp) != 0)
    {
        return;
    }

    ACPI_DEBUG_PRINT((ACPI_DB_INFO, "%s: zone temperature is %s C\n", "tz",
                      acpitz_celcius_string(tmp)));

    /*
	 * As noted in ACPI 4.0 (p. 420), the temperature
	 * thresholds are conveyed in the optional _ACx
	 * object (x = 0 ... 9). The smaller the x, the
	 * greater the cooling level. We prefer to keep
	 * the highest cooling mode when in "active".
	 */
    active = ATZ_ACTIVE_NONE;

    for (i = ATZ_NLEVELS - 1; i >= 0; i--)
    {

        if (tz->zone.ac[i] == ATZ_TMP_INVALID)
        {
            continue;
        }

        if (tz->zone.ac[i] <= tmp)
        {
            active = i;
        }
    }
}

void acpitz_poll_status(FAcpiTz *tz, uint32_t *tmp)
{
    uint32_t val;

    if (acpitz_get_integer(tz, "_TMP", &val) != 0)
    {
        return;
    }

    if (tmp)
    {
        *tmp = abs(val - ATZ_ZEROC);
    }

    FACPI_INFO("cpu temperature is %s C", acpitz_celcius_string(val));

    return;
}

static char *acpitz_celcius_string(int dk)
{
    static char buf[10];
    int dc;

    dc = abs(dk - ATZ_ZEROC);

    (void)snprintf(buf, sizeof(buf), "%s%d.%d", (dk >= ATZ_ZEROC) ? "" : "-", dc / 10, dc % 10);

    return buf;
}

static ACPI_STATUS acpitz_switch_cooler(ACPI_OBJECT *obj, void *arg)
{
    int flag, pwr_state;
    ACPI_HANDLE cooler;
    ACPI_STATUS rv;

    /*
	 * The _ALx object is a package in which the elements
	 * are reference handles to an active cooling device
	 * (typically PNP0C0B, ACPI fan device). Try to turn
	 * on (or off) the power resources behind these handles
	 * to start (or terminate) the active cooling.
	 */
    flag = *(int *)arg;
    pwr_state = (flag != 0) ? ACPI_STATE_D0 : ACPI_STATE_D3;

    rv = acpi_eval_reference_handle(obj, &cooler);

    if (ACPI_FAILURE(rv))
    {
        return rv;
    }

    (void)acpi_power_set(cooler, pwr_state);

    return AE_OK;
}

/*
 * acpitz_power_zone:
 *
 *	Power on or off the i:th part of the zone zone.
 */
static void acpitz_power_zone(FAcpiTz *tz, int i, int on)
{

    FASSERT(i >= 0 && i < ATZ_NLEVELS);

    (void)acpi_foreach_package_object(tz->zone.al[i].Pointer, acpitz_switch_cooler, &on);
}

/*
 * acpitz_power_off:
 *
 *	Power off parts of the zone.
 */
static void acpitz_power_off(FAcpiTz *tz)
{
    int i;

    for (i = 0; i < ATZ_NLEVELS; i++)
    {

        if (tz->zone.al[i].Pointer == NULL)
        {
            continue;
        }

        acpitz_power_zone(tz, i, 0);
    }
}

static void acpitz_get_zone(void *dev, int verbose)
{
    FAcpiTz *tz = dev;
    int comma, i, valid_levels;
    ACPI_OBJECT *obj;
    ACPI_STATUS rv;
    char buf[5];

    if (tz->first != true)
    {
        acpitz_power_off(tz);

        for (i = 0; i < ATZ_NLEVELS; i++)
        {

            if (tz->zone.al[i].Pointer != NULL)
            {
                ACPI_FREE(tz->zone.al[i].Pointer);
            }

            tz->zone.al[i].Pointer = NULL;
        }
    }

    valid_levels = 0;

    for (i = 0; i < ATZ_NLEVELS; i++)
    {

        (void)snprintf(buf, sizeof(buf), "_AC%d", i);

        if (acpitz_get_integer(tz, buf, &tz->zone.ac[i]))
        {
            continue;
        }

        (void)snprintf(buf, sizeof(buf), "_AL%d", i);

        rv = acpi_eval_struct(tz->node->ad_handle, buf, &tz->zone.al[i]);

        if (ACPI_FAILURE(rv))
        {
            tz->zone.al[i].Pointer = NULL;
            continue;
        }

        obj = tz->zone.al[i].Pointer;

        if (obj->Type != ACPI_TYPE_PACKAGE || obj->Package.Count == 0)
        {
            tz->zone.al[i].Pointer = NULL;
            ACPI_FREE(obj);
            continue;
        }

        if (tz->first != false)
        {
            FACPI_INFO("active cooling level %d: %sC", i, acpitz_celcius_string(tz->zone.ac[i]));
        }

        valid_levels++;
    }

    /*
	 * A brief summary (ACPI 4.0, section 11.4):
	 *
	 *    _TMP : current temperature (in tenths of degrees)
	 *    _CRT : critical trip-point at which to shutdown
	 *    _HOT : critical trip-point at which to go to S4
	 *    _PSV : passive cooling policy threshold
	 *    _TC1 : thermal constant for passive cooling
	 *    _TC2 : thermal constant for passive cooling
	 */
    (void)acpitz_get_integer(tz, "_TMP", &tz->zone.tmp);
    (void)acpitz_get_integer(tz, "_CRT", &tz->zone.crt);
    (void)acpitz_get_integer(tz, "_HOT", &tz->zone.hot);
    (void)acpitz_get_integer(tz, "_PSV", &tz->zone.psv);
    (void)acpitz_get_integer(tz, "_TC1", &tz->zone.tc1);
    (void)acpitz_get_integer(tz, "_TC2", &tz->zone.tc2);

    /*
	 * If _RTV is not present or present and zero,
	 * values are absolute (see ACPI 4.0, 425).
	 */
    acpitz_get_integer(tz, "_RTV", &tz->zone.rtv);

    if (tz->zone.rtv == ATZ_TMP_INVALID)
    {
        tz->zone.rtv = 0;
    }

    acpitz_sane_temp(&tz->zone.tmp);
    acpitz_sane_temp(&tz->zone.crt);
    acpitz_sane_temp(&tz->zone.hot);
    acpitz_sane_temp(&tz->zone.psv);

    if (verbose != 0)
    {
        comma = 0;

        FACPI_INFO("levels: ");

        if (tz->zone.crt != ATZ_TMP_INVALID)
        {
            FACPI_INFO("critical %s C", acpitz_celcius_string(tz->zone.crt));
            comma = 1;
        }

        if (tz->zone.hot != ATZ_TMP_INVALID)
        {
            FACPI_INFO("%shot %s C", comma ? ", " : "", acpitz_celcius_string(tz->zone.hot));
            comma = 1;
        }

        if (tz->zone.psv != ATZ_TMP_INVALID)
        {
            FACPI_INFO("%spassive %s C", comma ? ", " : "",
                       acpitz_celcius_string(tz->zone.psv));
            comma = 1;
        }

        if (valid_levels == 0)
        {
            if (tz->first != false)
            {
                FACPI_INFO("%spassive cooling", comma ? ", " : "");
            }
        }
    }

    for (i = 0; i < ATZ_NLEVELS; i++)
    {
        acpitz_sane_temp(&tz->zone.ac[i]);
    }

    acpitz_power_off(tz);
    tz->first = false;
}

static void acpitz_notify_handler(ACPI_HANDLE hdl, uint32_t notify, void *opaque)
{
    ACPI_OSD_EXEC_CALLBACK func = NULL;
    FAcpiTz *tz = opaque;

    switch (notify)
    {

        case ACPI_NOTIFY_TZ_ZONE:
            func = acpitz_get_status;
            break;

        case ACPI_NOTIFY_TZ_TRIP:
        case ACPI_NOTIFY_TZ_DEVLIST:
            func = acpitz_get_zone_quiet;
            break;

        default:
            FACPI_WARN("unknown notify 0x%02X", notify);
            return;
    }

    (void)AcpiOsExecute(OSL_NOTIFY_HANDLER, func, tz);
}

static void acpitz_sane_temp(uint32_t *tmp)
{
    /* Sane temperatures are between 0 and 150 C. */
    if (*tmp < ATZ_ZEROC || *tmp > ATZ_ZEROC + 1500)
    {
        *tmp = ATZ_TMP_INVALID;
    }
}

static int acpitz_get_integer(FAcpiTz *tz, char *cm, uint32_t *val)
{
    ACPI_INTEGER tmp;
    ACPI_STATUS rv;

    rv = acpi_eval_integer(tz->node->ad_handle, cm, &tmp);

    if (ACPI_FAILURE(rv))
    {
        *val = ATZ_TMP_INVALID;

        ACPI_DEBUG_PRINT((ACPI_DB_DEBUG_OBJECT, "%s: failed to evaluate %s: %s\n", "tz",
                          cm, AcpiFormatException(rv)));

        return 1;
    }

    *val = tmp;

    return 0;
}