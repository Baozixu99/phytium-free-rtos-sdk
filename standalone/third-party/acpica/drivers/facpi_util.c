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
 * Modified from NetBSD sys/dev/acpi/acpi_util.c with NETBSD LICENSE
 *
 * FilePath: facpi_util.c
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
#define ACPI_MATCHSCORE_CLS 1 /* matched _CLS */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FACPI_DEBUG_TAG     "FACPI"
#define FACPI_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_WARN(format, ...) FT_DEBUG_PRINT_W(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_INFO(format, ...) FT_DEBUG_PRINT_I(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_VERBOS(format, ...) \
    FT_DEBUG_PRINT_V(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/
int pmatch(const char *, const char *, const char **);

static void acpi_clean_node(ACPI_HANDLE, void *);
static ACPI_STATUS acpi_dsd_property(ACPI_HANDLE, const char *, ACPI_BUFFER *,
                                     ACPI_OBJECT_TYPE, ACPI_OBJECT **);
/*****************************************************************************/
static const char *const acpicpu_ids[] = {"ACPI0007", NULL};

/*
 * Evaluate an integer object.
 */
ACPI_STATUS
acpi_eval_integer(ACPI_HANDLE handle, char *path, ACPI_INTEGER *valp)
{
    ACPI_OBJECT obj;
    ACPI_BUFFER buf;
    ACPI_STATUS rv;

    if (handle == NULL)
    {
        handle = ACPI_ROOT_OBJECT;
    }

    (void)memset(&obj, 0, sizeof(obj));
    buf.Pointer = &obj;
    buf.Length = sizeof(obj);

    rv = AcpiEvaluateObject(handle, path, NULL, &buf);

    if (ACPI_FAILURE(rv))
    {
        return rv;
    }

    /* Check that evaluation produced a return value. */
    if (buf.Length == 0)
    {
        return AE_NULL_OBJECT;
    }

    if (obj.Type != ACPI_TYPE_INTEGER)
    {
        return AE_TYPE;
    }

    if (valp != NULL)
    {
        *valp = obj.Integer.Value;
    }

    return AE_OK;
}

/*
 * Evaluate an integer object with a single integer input parameter.
 */
ACPI_STATUS
acpi_eval_set_integer(ACPI_HANDLE handle, char *path, ACPI_INTEGER val)
{
    ACPI_OBJECT_LIST arg;
    ACPI_OBJECT obj;

    if (handle == NULL)
    {
        handle = ACPI_ROOT_OBJECT;
    }

    obj.Type = ACPI_TYPE_INTEGER;
    obj.Integer.Value = val;

    arg.Count = 1;
    arg.Pointer = &obj;

    return AcpiEvaluateObject(handle, path, &arg, NULL);
}

/*
 * Evaluate a (Unicode) string object.
 */
ACPI_STATUS
acpi_eval_string(ACPI_HANDLE handle, char *path, char **stringp)
{
    ACPI_OBJECT *obj;
    ACPI_BUFFER buf;
    ACPI_STATUS rv;

    rv = acpi_eval_struct(handle, path, &buf);

    if (ACPI_FAILURE(rv))
    {
        return rv;
    }

    obj = buf.Pointer;

    if (obj->Type != ACPI_TYPE_STRING)
    {
        rv = AE_TYPE;
        goto out;
    }

    if (obj->String.Length == 0)
    {
        rv = AE_BAD_DATA;
        goto out;
    }

    *stringp = ACPI_ALLOCATE(obj->String.Length + 1);

    if (*stringp == NULL)
    {
        rv = AE_NO_MEMORY;
        goto out;
    }

    (void)memcpy(*stringp, obj->String.Pointer, obj->String.Length);

    (*stringp)[obj->String.Length] = '\0';

out:
    ACPI_FREE(buf.Pointer);

    return rv;
}

/*
 * Evaluate a structure. Caller must free buf.Pointer by ACPI_FREE().
 */
ACPI_STATUS
acpi_eval_struct(ACPI_HANDLE handle, char *path, ACPI_BUFFER *buf)
{

    if (handle == NULL)
    {
        handle = ACPI_ROOT_OBJECT;
    }

    buf->Pointer = NULL;
    buf->Length = ACPI_ALLOCATE_LOCAL_BUFFER;

    return AcpiEvaluateObject(handle, path, NULL, buf);
}

/*
 * Evaluate a reference handle from an element in a package.
 */
ACPI_STATUS
acpi_eval_reference_handle(ACPI_OBJECT *elm, ACPI_HANDLE *handle)
{

    if (elm == NULL || handle == NULL)
    {
        return AE_BAD_PARAMETER;
    }

    switch (elm->Type)
    {

        case ACPI_TYPE_ANY:
        case ACPI_TYPE_LOCAL_REFERENCE:

            if (elm->Reference.Handle == NULL)
            {
                return AE_NULL_ENTRY;
            }

            *handle = elm->Reference.Handle;

            return AE_OK;

        case ACPI_TYPE_STRING:
            return AcpiGetHandle(NULL, elm->String.Pointer, handle);

        default:
            return AE_TYPE;
    }
}

/*
 * Iterate over all objects in a package, and pass them all
 * to a function. If the called function returns non-AE_OK,
 * the iteration is stopped and that value is returned.
 */
ACPI_STATUS
acpi_foreach_package_object(ACPI_OBJECT *pkg, ACPI_STATUS (*func)(ACPI_OBJECT *, void *), void *arg)
{
    ACPI_STATUS rv = AE_OK;
    uint32_t i;

    if (pkg == NULL)
    {
        return AE_BAD_PARAMETER;
    }

    if (pkg->Type != ACPI_TYPE_PACKAGE)
    {
        return AE_TYPE;
    }

    for (i = 0; i < pkg->Package.Count; i++)
    {

        rv = (*func)(&pkg->Package.Elements[i], arg);

        if (ACPI_FAILURE(rv))
        {
            break;
        }
    }

    return rv;
}

/*
 * Fetch data info the specified (empty) ACPI buffer.
 * Caller must free buf.Pointer by ACPI_FREE().
 */
ACPI_STATUS
acpi_get(ACPI_HANDLE handle, ACPI_BUFFER *buf, ACPI_STATUS (*getit)(ACPI_HANDLE, ACPI_BUFFER *))
{

    buf->Pointer = NULL;
    buf->Length = ACPI_ALLOCATE_LOCAL_BUFFER;

    return (*getit)(handle, buf);
}

/*
 * Return a complete pathname from a handle.
 *
 * Note that the function uses static data storage;
 * if the data is needed for future use, it should be
 * copied before any subsequent calls overwrite it.
 */
const char *acpi_name_netbsd(ACPI_HANDLE handle)
{
    static char name[80];
    ACPI_BUFFER buf;
    ACPI_STATUS rv;

    if (handle == NULL)
    {
        handle = ACPI_ROOT_OBJECT;
    }

    buf.Pointer = name;
    buf.Length = sizeof(name);

    rv = AcpiGetName(handle, ACPI_FULL_PATHNAME, &buf);

    if (ACPI_FAILURE(rv))
    {
        return "UNKNOWN";
    }

    return name;
}

/*
 * Match given IDs against _HID and _CIDs.
 */
int acpi_match_hid(ACPI_DEVICE_INFO *ad, const char *const *ids)
{
    uint32_t i, n;
    char *id;

    while (*ids)
    {

        if ((ad->Valid & ACPI_VALID_HID) != 0)
        {

            if (pmatch(ad->HardwareId.String, *ids, NULL) == 2)
            {
                return 1;
            }
        }

        if ((ad->Valid & ACPI_VALID_CID) != 0)
        {

            n = ad->CompatibleIdList.Count;

            for (i = 0; i < n; i++)
            {

                id = ad->CompatibleIdList.Ids[i].String;

                if (pmatch(id, *ids, NULL) == 2)
                {
                    return 1;
                }
            }
        }

        ids++;
    }

    return 0;
}

/*
 * Match a PCI-defined bass-class, sub-class, and programming interface
 * against a handle's _CLS object.
 */
int acpi_match_class(ACPI_HANDLE handle, uint8_t pci_class, uint8_t pci_subclass, uint8_t pci_interface)
{
    ACPI_BUFFER buf;
    ACPI_OBJECT *obj;
    ACPI_STATUS rv;
    int match = 0;

    rv = acpi_eval_struct(handle, "_CLS", &buf);
    if (ACPI_FAILURE(rv))
    {
        goto done;
    }

    obj = buf.Pointer;
    if (obj->Type != ACPI_TYPE_PACKAGE)
    {
        goto done;
    }
    if (obj->Package.Count != 3)
    {
        goto done;
    }
    if (obj->Package.Elements[0].Type != ACPI_TYPE_INTEGER ||
        obj->Package.Elements[1].Type != ACPI_TYPE_INTEGER ||
        obj->Package.Elements[2].Type != ACPI_TYPE_INTEGER)
    {
        goto done;
    }

    match = obj->Package.Elements[0].Integer.Value == pci_class &&
            obj->Package.Elements[1].Integer.Value == pci_subclass &&
            obj->Package.Elements[2].Integer.Value == pci_interface;

done:
    if (buf.Pointer)
    {
        ACPI_FREE(buf.Pointer);
    }
    return match ? ACPI_MATCHSCORE_CLS : 0;
}

/*
 * Match a device node from a handle.
 */
FAcpiDevNode *acpi_match_node(ACPI_HANDLE handle)
{
    FAcpiDevNode *ad;
    ACPI_STATUS rv;

    if (handle == NULL)
    {
        return NULL;
    }

    rv = AcpiGetData(handle, acpi_clean_node, (void **)&ad);

    if (ACPI_FAILURE(rv))
    {
        return NULL;
    }

    return ad;
}

/*
 * Permanently associate a device node with a handle.
 */
void acpi_match_node_init(FAcpiDevNode *ad)
{
    (void)AcpiAttachData(ad->ad_handle, acpi_clean_node, ad);
}

static void acpi_clean_node(ACPI_HANDLE handle, void *aux)
{
    /* Nothing. */
}

/*
 * Match a CPU from a handle. Returns NULL on failure.
 */
boolean acpi_match_cpu_handle(u32 cpu_id, ACPI_HANDLE hdl)
{
    boolean matched = FALSE;
    ACPI_DEVICE_INFO *di;
    ACPI_INTEGER val;
    ACPI_OBJECT *obj;
    ACPI_BUFFER buf;
    ACPI_STATUS rv;

    di = NULL;
    buf.Pointer = NULL;

    rv = AcpiGetObjectInfo(hdl, &di);

    if (ACPI_FAILURE(rv))
    {
        return matched;
    }

    switch (di->Type)
    {

        case ACPI_TYPE_DEVICE:

            if (acpi_match_hid(di, acpicpu_ids) == 0)
            {
                goto out;
            }

            rv = acpi_eval_integer(hdl, "_UID", &val);

            if (ACPI_FAILURE(rv))
            {
                goto out;
            }

            break;

        case ACPI_TYPE_PROCESSOR:

            rv = acpi_eval_struct(hdl, NULL, &buf);

            if (ACPI_FAILURE(rv))
            {
                goto out;
            }

            obj = buf.Pointer;
            val = obj->Processor.ProcId;
            break;

        default:
            goto out;
    }

    if (val == cpu_id)
    {
        matched = TRUE;
    }

out:
    if (di != NULL)
    {
        ACPI_FREE(di);
    }

    if (buf.Pointer != NULL)
    {
        ACPI_FREE(buf.Pointer);
    }

    return matched;
}

/*
 * Device-Specific Data (_DSD) support
 */

static UINT8 acpi_dsd_uuid[ACPI_UUID_LENGTH] = {0x14, 0xd8, 0xff, 0xda, 0xba, 0x6e,
                                                0x8c, 0x4d, 0x8a, 0x91, 0xbc, 0x9b,
                                                0xbf, 0x4a, 0xa3, 0x01};

static ACPI_STATUS acpi_dsd_property(ACPI_HANDLE handle, const char *prop, ACPI_BUFFER *pbuf,
                                     ACPI_OBJECT_TYPE type, ACPI_OBJECT **ret)
{
    ACPI_OBJECT *obj, *uuid, *props, *pobj, *propkey, *propval;
    ACPI_STATUS rv;
    int n;

    rv = AcpiEvaluateObjectTyped(handle, "_DSD", NULL, pbuf, ACPI_TYPE_PACKAGE);
    if (ACPI_FAILURE(rv))
    {
        return rv;
    }

    props = NULL;
    obj = (ACPI_OBJECT *)pbuf->Pointer;
    for (n = 0; (n + 1) < obj->Package.Count; n += 2)
    {
        uuid = &obj->Package.Elements[n];
        if (uuid->Buffer.Length == ACPI_UUID_LENGTH &&
            memcmp(uuid->Buffer.Pointer, acpi_dsd_uuid, ACPI_UUID_LENGTH) == 0)
        {
            props = &obj->Package.Elements[n + 1];
            break;
        }
    }
    if (props == NULL)
    {
        return AE_NOT_FOUND;
    }

    for (n = 0; n < props->Package.Count; n++)
    {
        pobj = &props->Package.Elements[n];
        if (pobj->Type != ACPI_TYPE_PACKAGE || pobj->Package.Count != 2)
        {
            continue;
        }
        propkey = (ACPI_OBJECT *)&pobj->Package.Elements[0];
        propval = (ACPI_OBJECT *)&pobj->Package.Elements[1];
        if (propkey->Type != ACPI_TYPE_STRING)
        {
            continue;
        }
        if (strcmp(propkey->String.Pointer, prop) != 0)
        {
            continue;
        }

        if (propval->Type != type)
        {
            return AE_TYPE;
        }
        else
        {
            *ret = propval;
            return AE_OK;
        }
        break;
    }

    return AE_NOT_FOUND;
}

ACPI_STATUS
acpi_dsd_integer(ACPI_HANDLE handle, const char *prop, ACPI_INTEGER *val)
{
    ACPI_OBJECT *propval;
    ACPI_STATUS rv;
    ACPI_BUFFER buf;

    buf.Pointer = NULL;
    buf.Length = ACPI_ALLOCATE_BUFFER;

    rv = acpi_dsd_property(handle, prop, &buf, ACPI_TYPE_INTEGER, &propval);
    if (ACPI_SUCCESS(rv))
    {
        *val = propval->Integer.Value;
    }

    if (buf.Pointer != NULL)
    {
        ACPI_FREE(buf.Pointer);
    }
    return rv;
}

ACPI_STATUS
acpi_dsd_string(ACPI_HANDLE handle, const char *prop, char **val)
{
    ACPI_OBJECT *propval;
    ACPI_STATUS rv;
    ACPI_BUFFER buf;

    buf.Pointer = NULL;
    buf.Length = ACPI_ALLOCATE_BUFFER;

    rv = acpi_dsd_property(handle, prop, &buf, ACPI_TYPE_STRING, &propval);
    if (ACPI_SUCCESS(rv))
    {
        *val = strdup(propval->String.Pointer);
    }

    if (buf.Pointer != NULL)
    {
        ACPI_FREE(buf.Pointer);
    }
    return rv;
}

ACPI_STATUS
acpi_dsd_bool(ACPI_HANDLE handle, const char *prop, bool *val)
{
    ACPI_STATUS rv;
    ACPI_INTEGER ival;

    rv = acpi_dsd_integer(handle, prop, &ival);
    if (ACPI_SUCCESS(rv))
    {
        *val = ival != 0;
    }

    return rv;
}

/*
 * Device Specific Method (_DSM) support
 */

ACPI_STATUS
acpi_dsm_typed(ACPI_HANDLE handle, uint8_t *uuid, ACPI_INTEGER rev, ACPI_INTEGER func,
               const ACPI_OBJECT *arg3, ACPI_OBJECT_TYPE return_type, ACPI_OBJECT **return_obj)
{
    ACPI_OBJECT_LIST arg;
    ACPI_OBJECT obj[4];
    ACPI_BUFFER buf;
    ACPI_STATUS status;

    arg.Count = 4;
    arg.Pointer = obj;

    obj[0].Type = ACPI_TYPE_BUFFER;
    obj[0].Buffer.Length = ACPI_UUID_LENGTH;
    obj[0].Buffer.Pointer = uuid;

    obj[1].Type = ACPI_TYPE_INTEGER;
    obj[1].Integer.Value = rev;

    obj[2].Type = ACPI_TYPE_INTEGER;
    obj[2].Integer.Value = func;

    if (arg3 != NULL)
    {
        obj[3] = *arg3;
    }
    else
    {
        obj[3].Type = ACPI_TYPE_PACKAGE;
        obj[3].Package.Count = 0;
        obj[3].Package.Elements = NULL;
    }

    buf.Pointer = NULL;
    buf.Length = ACPI_ALLOCATE_BUFFER;

    if (return_obj == NULL && return_type == ACPI_TYPE_ANY)
    {
        status = AcpiEvaluateObject(handle, "_DSM", &arg, NULL);
    }
    else
    {
        *return_obj = NULL;
        status = AcpiEvaluateObjectTyped(handle, "_DSM", &arg, &buf, return_type);
    }
    if (ACPI_FAILURE(status))
    {
        return status;
    }
    if (return_obj != NULL)
    {
        *return_obj = buf.Pointer;
    }
    else if (buf.Pointer != NULL)
    {
        ACPI_FREE(buf.Pointer);
    }
    return AE_OK;
}

ACPI_STATUS
acpi_dsm_integer(ACPI_HANDLE handle, uint8_t *uuid, ACPI_INTEGER rev, ACPI_INTEGER func,
                 const ACPI_OBJECT *arg3, ACPI_INTEGER *ret)
{
    ACPI_OBJECT *obj;
    ACPI_STATUS status;

    status = acpi_dsm_typed(handle, uuid, rev, func, arg3, ACPI_TYPE_INTEGER, &obj);
    if (ACPI_FAILURE(status))
    {
        return status;
    }

    *ret = obj->Integer.Value;
    ACPI_FREE(obj);

    return AE_OK;
}

ACPI_STATUS
acpi_dsm(ACPI_HANDLE handle, uint8_t *uuid, ACPI_INTEGER rev, ACPI_INTEGER func,
         const ACPI_OBJECT *arg3, ACPI_OBJECT **return_obj)
{
    return acpi_dsm_typed(handle, uuid, rev, func, arg3, ACPI_TYPE_ANY, return_obj);
}

ACPI_STATUS
acpi_dsm_query(ACPI_HANDLE handle, uint8_t *uuid, ACPI_INTEGER rev, ACPI_INTEGER *ret)
{
    ACPI_OBJECT *obj;
    ACPI_STATUS status;
    uint8_t *data;
    u_int n;

    status = acpi_dsm(handle, uuid, rev, 0, NULL, &obj);
    if (ACPI_FAILURE(status))
    {
        return status;
    }

    if (obj->Type == ACPI_TYPE_INTEGER)
    {
        *ret = obj->Integer.Value;
    }
    else if (obj->Type == ACPI_TYPE_BUFFER && obj->Buffer.Length <= 8)
    {
        *ret = 0;
        data = (uint8_t *)obj->Buffer.Pointer;
        for (n = 0; n < obj->Buffer.Length; n++)
        {
            *ret |= (uint64_t)data[n] << (n * 8);
        }
    }
    else
    {
        status = AE_TYPE;
    }

    ACPI_FREE(obj);

    return status;
}

ACPI_TABLE_HEADER *acpi_map_rsdt(FAcpi *acpi)
{
    ACPI_PHYSICAL_ADDRESS paddr;
    ACPI_TABLE_RSDP *rsdp;

    paddr = AcpiOsGetRootPointer();

    if (paddr == 0)
    {
        return NULL;
    }

    rsdp = AcpiOsMapMemory(paddr, sizeof(ACPI_TABLE_RSDP));

    if (rsdp == NULL)
    {
        return NULL;
    }

    if (rsdp->Revision > 1 && rsdp->XsdtPhysicalAddress)
    {
        paddr = rsdp->XsdtPhysicalAddress;
    }
    else
    {
        paddr = rsdp->RsdtPhysicalAddress;
    }

    AcpiOsUnmapMemory(rsdp, sizeof(ACPI_TABLE_RSDP));

    return AcpiOsMapMemory(paddr, sizeof(ACPI_TABLE_HEADER));
}

void acpi_unmap_rsdt(FAcpi *acpi, ACPI_TABLE_HEADER *rsdt)
{

    if (rsdt == NULL)
    {
        return;
    }

    AcpiOsUnmapMemory(rsdt, sizeof(ACPI_TABLE_HEADER));
}

ACPI_STATUS acpi_madt_map(FAcpi *acpi)
{
    ACPI_STATUS rv;

    if (acpi->madt_header != NULL)
    {
        return AE_ALREADY_EXISTS;
    }

    rv = AcpiGetTable(ACPI_SIG_MADT, 1, &(acpi->madt_header));

    if (ACPI_FAILURE(rv))
    {
        return rv;
    }

    return AE_OK;
}

void acpi_madt_unmap(FAcpi *acpi)
{
    acpi->madt_header = NULL;
}

ACPI_STATUS acpi_gtdt_map(FAcpi *acpi)
{
    ACPI_STATUS rv;

    if (acpi->gtdt_header != NULL)
    {
        return AE_ALREADY_EXISTS;
    }

    rv = AcpiGetTable(ACPI_SIG_GTDT, 1, &(acpi->gtdt_header));

    if (ACPI_FAILURE(rv))
    {
        return rv;
    }

    return AE_OK;
}

void acpi_gtdt_unmap(FAcpi *acpi)
{
    acpi->gtdt_header = NULL;
}

ACPI_GTDT_WATCHDOG *acpi_find_wdt_table(FAcpi *acpi)
{
    ACPI_STATUS Status;
    ACPI_GTDT_HEADER *Subtable;
    ACPI_GTDT_WATCHDOG *GtdtWdt = NULL;
    ACPI_TABLE_HEADER *Table = acpi->gtdt_header;
    UINT32 Length = Table->Length;
    UINT32 Offset = sizeof(ACPI_TABLE_GTDT);
    UINT32 SubtableLength;
    UINT32 GtCount;
    ACPI_GTDT_TIMER_ENTRY *GtxTable;
    FASSERT(Table);

    /* Rev 3 fields */
    Subtable = ACPI_ADD_PTR(ACPI_GTDT_HEADER, Table, Offset);
    if (Table->Revision > 2)
    {
        SubtableLength = sizeof(ACPI_GTDT_EL2);
        Offset += SubtableLength;
    }

    Subtable = ACPI_ADD_PTR(ACPI_GTDT_HEADER, Table, Offset);

    /* Subtables */
    while (Offset < Table->Length)
    {
        /* Common subtable header */
        GtCount = 0;
        switch (Subtable->Type)
        {
            case ACPI_GTDT_TYPE_TIMER_BLOCK:

                SubtableLength = sizeof(ACPI_GTDT_TIMER_BLOCK);
                GtCount = (ACPI_CAST_PTR(ACPI_GTDT_TIMER_BLOCK, Subtable))->TimerCount;

                break;

            case ACPI_GTDT_TYPE_WATCHDOG:

                SubtableLength = sizeof(ACPI_GTDT_WATCHDOG);
                GtdtWdt = (ACPI_GTDT_WATCHDOG *)Subtable;
                break;

            default:
                /* Cannot continue on unknown type - no length */
                AcpiOsPrintf("\n**** Unknown GTDT subtable type 0x%X\n", Subtable->Type);
                return NULL;
        }

        /* Point to end of current subtable (each subtable above is of fixed length) */
        Offset += SubtableLength;

        /* If there are any Gt Timer Blocks from above, dump them now */
        if (GtCount)
        {
            GtxTable = ACPI_ADD_PTR(ACPI_GTDT_TIMER_ENTRY, Subtable, SubtableLength);
            SubtableLength += GtCount * sizeof(ACPI_GTDT_TIMER_ENTRY);

            while (GtCount)
            {
                Offset += sizeof(ACPI_GTDT_TIMER_ENTRY);
                GtxTable++;
                GtCount--;
            }
        }

        /* Point to next subtable */
        Subtable = ACPI_ADD_PTR(ACPI_GTDT_HEADER, Subtable, SubtableLength);
    }

    return GtdtWdt;
}

static void acpi_make_name(FAcpiDevNode *ad, uint32_t name)
{
    ACPI_NAME_UNION *anu;
    int clear, i;

    anu = (ACPI_NAME_UNION *)&name;
    ad->ad_name[4] = '\0';

    for (i = 3, clear = 0; i >= 0; i--)
    {

        if (clear == 0 && anu->Ascii[i] == '_')
        {
            ad->ad_name[i] = '\0';
        }
        else
        {
            ad->ad_name[i] = anu->Ascii[i];
            clear = 1;
        }
    }

    if (ad->ad_name[0] == '\0')
    {
        ad->ad_name[0] = '_';
    }
}

static void acpi_activate_device(ACPI_HANDLE handle, ACPI_DEVICE_INFO **di)
{
}

ACPI_STATUS acpi_make_devnode(ACPI_HANDLE handle, uint32_t level, void *context, void **status)
{
    FAcpiWalkContext *awc = context;
    FAcpi *acpi = awc->acpi;
    FAcpiDevNode *ad;
    ACPI_DEVICE_INFO *devinfo;
    ACPI_OBJECT_TYPE type;
    ACPI_STATUS rv;

    rv = AcpiGetObjectInfo(handle, &devinfo);

    if (ACPI_FAILURE(rv))
    {
        return AE_OK; /* Do not terminate the walk. */
    }

    type = devinfo->Type;

    switch (type)
    {

        case ACPI_TYPE_DEVICE:
            acpi_activate_device(handle, &devinfo);
            /* FALLTHROUGH */

        case ACPI_TYPE_PROCESSOR:
        case ACPI_TYPE_THERMAL:
        case ACPI_TYPE_POWER:

            FASSERT_MSG(acpi->max_dev_node < FACPI_MAX_DEV_NODE_NUM,
                        "Too many devices > %d", acpi->max_dev_node);
            ad = &acpi->dev_nodes[acpi->max_dev_node++];

            ad->ad_notify = NULL;

            ad->ad_type = type;
            ad->ad_handle = handle;
            ad->ad_devinfo = devinfo;

            ad->acpi = acpi;

            acpi_match_node_init(ad);
            acpi_make_name(ad, devinfo->Name);

            /*
		 * Identify wake GPEs from the _PRW. Note that
		 * AcpiUpdateAllGpes() must be called afterwards.
		 */
            if (ad->ad_devinfo->Type == ACPI_TYPE_DEVICE)
            {
                FACPI_INFO("Maybe wakeup power device");
            }

            FACPI_INFO("[%d] Found device %s, type %d, handle %p", acpi->max_dev_node,
                       ad->ad_name, ad->ad_type, ad->ad_handle);
            break;

        default:
            ACPI_FREE(devinfo);
            break;
    }

    return AE_OK;
}