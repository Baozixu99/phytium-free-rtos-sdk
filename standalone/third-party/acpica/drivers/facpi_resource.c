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
 * Modified from NetBSD sys/dev/acpi/acpi_resource.c with NETBSD LICENSE
 *
 * FilePath: facpi_resource.c
 * Date: 2025-04-10 16:20:52
 * LastEditTime: 2025-04-10 16:20:52
 * Description:  This file is for acpi resource functions
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

#define _COMPONENT          0
ACPI_MODULE_NAME("RESOURCE")
/**************************** Type Definitions *******************************/
typedef void *device_t;

/*
 * acpi_resource_parse_ops:
 *
 *	The client of ACPI resources specifies these operations
 *	when the resources are parsed.
 */
struct acpi_resource_parse_ops
{
    void (*init)(device_t, void *, void **);
    void (*fini)(device_t, void *);

    void (*ioport)(device_t, void *, uint32_t, uint32_t);
    void (*iorange)(device_t, void *, uint32_t, uint32_t, uint32_t, uint32_t);

    void (*memory)(device_t, void *, uint64_t, uint64_t, uint64_t);
    void (*memrange)(device_t, void *, uint64_t, uint64_t, uint64_t, uint64_t);

    void (*irq)(device_t, void *, uint32_t, uint32_t);
    void (*drq)(device_t, void *, uint32_t);

    void (*start_dep)(device_t, void *, int);
    void (*end_dep)(device_t, void *);
};

struct resource_parse_callback_arg
{
    const struct acpi_resource_parse_ops *ops;
    bool include_producer;
    device_t dev;
    void *context;
};
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
static ACPI_STATUS acpi_resource_parse_callback(ACPI_RESOURCE *, void *);

/*****************************************************************************/
static ACPI_STATUS acpi_resource_parse_callback(ACPI_RESOURCE *res, void *context)
{
    struct resource_parse_callback_arg *arg = context;
    const struct acpi_resource_parse_ops *ops;
    int i;

    ACPI_FUNCTION_TRACE(__func__);

    ops = arg->ops;

    switch (res->Type)
    {
        case ACPI_RESOURCE_TYPE_END_TAG:
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "EndTag\n"));
            break;
        case ACPI_RESOURCE_TYPE_FIXED_IO:
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "FixedIo 0x%x/%u\n",
                              res->Data.FixedIo.Address, res->Data.FixedIo.AddressLength));
            if (ops->ioport)
            {
                (*ops->ioport)(arg->dev, arg->context, res->Data.FixedIo.Address,
                               res->Data.FixedIo.AddressLength);
            }
            break;

        case ACPI_RESOURCE_TYPE_IO:
            if (res->Data.Io.Minimum == res->Data.Io.Maximum)
            {
                ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Io 0x%x/%u\n",
                                  res->Data.Io.Minimum, res->Data.Io.AddressLength));
                if (ops->ioport)
                {
                    (*ops->ioport)(arg->dev, arg->context, res->Data.Io.Minimum,
                                   res->Data.Io.AddressLength);
                }
            }
            else
            {
                ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Io 0x%x-0x%x/%u\n", res->Data.Io.Minimum,
                                  res->Data.Io.Maximum, res->Data.Io.AddressLength));
                if (ops->iorange)
                {
                    (*ops->iorange)(arg->dev, arg->context, res->Data.Io.Minimum,
                                    res->Data.Io.Maximum, res->Data.Io.AddressLength,
                                    res->Data.Io.Alignment);
                }
            }
            break;

        case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "FixedMemory32 0x%x/%u\n",
                              res->Data.FixedMemory32.Address, res->Data.FixedMemory32.AddressLength));
            if (ops->memory)
            {
                (*ops->memory)(arg->dev, arg->context, res->Data.FixedMemory32.Address,
                               res->Data.FixedMemory32.AddressLength,
                               res->Data.FixedMemory32.Address);
            }
            break;

        case ACPI_RESOURCE_TYPE_MEMORY32:
            if (res->Data.Memory32.Minimum == res->Data.Memory32.Maximum)
            {
                ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Memory32 0x%x/%u\n",
                                  res->Data.Memory32.Minimum, res->Data.Memory32.AddressLength));
                if (ops->memory)
                {
                    (*ops->memory)(arg->dev, arg->context, res->Data.Memory32.Minimum,
                                   res->Data.Memory32.AddressLength, res->Data.Memory32.Minimum);
                }
            }
            else
            {
                ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Memory32 0x%x-0x%x/%u\n",
                                  res->Data.Memory32.Minimum, res->Data.Memory32.Maximum,
                                  res->Data.Memory32.AddressLength));
                if (ops->memrange)
                {
                    (*ops->memrange)(arg->dev, arg->context, res->Data.Memory32.Minimum,
                                     res->Data.Memory32.Maximum, res->Data.Memory32.AddressLength,
                                     res->Data.Memory32.Alignment);
                }
            }
            break;

        case ACPI_RESOURCE_TYPE_MEMORY24:
            if (res->Data.Memory24.Minimum == res->Data.Memory24.Maximum)
            {
                ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Memory24 0x%x/%u\n",
                                  res->Data.Memory24.Minimum, res->Data.Memory24.AddressLength));
                if (ops->memory)
                {
                    (*ops->memory)(arg->dev, arg->context, res->Data.Memory24.Minimum,
                                   res->Data.Memory24.AddressLength, res->Data.Memory24.Minimum);
                }
            }
            else
            {
                ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Memory24 0x%x-0x%x/%u\n",
                                  res->Data.Memory24.Minimum, res->Data.Memory24.Maximum,
                                  res->Data.Memory24.AddressLength));
                if (ops->memrange)
                {
                    (*ops->memrange)(arg->dev, arg->context, res->Data.Memory24.Minimum,
                                     res->Data.Memory24.Maximum, res->Data.Memory24.AddressLength,
                                     res->Data.Memory24.Alignment);
                }
            }
            break;

        case ACPI_RESOURCE_TYPE_IRQ:
            for (i = 0; i < res->Data.Irq.InterruptCount; i++)
            {
                ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "IRQ %u\n", res->Data.Irq.Interrupts[i]));
                if (ops->irq)
                {
                    (*ops->irq)(arg->dev, arg->context, res->Data.Irq.Interrupts[i],
                                res->Data.Irq.Triggering);
                }
            }
            break;

        case ACPI_RESOURCE_TYPE_DMA:
            for (i = 0; i < res->Data.Dma.ChannelCount; i++)
            {
                ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "DRQ %u\n", res->Data.Dma.Channels[i]));
                if (ops->drq)
                {
                    (*ops->drq)(arg->dev, arg->context, res->Data.Dma.Channels[i]);
                }
            }
            break;

        case ACPI_RESOURCE_TYPE_START_DEPENDENT:
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Start dependent functions: %u\n",
                              res->Data.StartDpf.CompatibilityPriority));
            if (ops->start_dep)
            {
                (*ops->start_dep)(arg->dev, arg->context, res->Data.StartDpf.CompatibilityPriority);
            }
            break;

        case ACPI_RESOURCE_TYPE_END_DEPENDENT:
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "End dependent functions\n"));
            if (ops->end_dep)
            {
                (*ops->end_dep)(arg->dev, arg->context);
            }
            break;

        case ACPI_RESOURCE_TYPE_ADDRESS32:
            /* XXX Only fixed size supported for now */
            if (res->Data.Address32.Address.AddressLength == 0)
            {
                break;
            }
#define ADDRESS32_FIXED2(r)                                       \
    ((r)->Data.Address32.MinAddressFixed == ACPI_ADDRESS_FIXED && \
     (r)->Data.Address32.MaxAddressFixed == ACPI_ADDRESS_FIXED)
            switch (res->Data.Address32.ResourceType)
            {
                case ACPI_MEMORY_RANGE:
                    if (ADDRESS32_FIXED2(res))
                    {
                        if (ops->memory)
                        {
                            (*ops->memory)(arg->dev, arg->context,
                                           res->Data.Address32.Address.Minimum,
                                           res->Data.Address32.Address.AddressLength,
                                           res->Data.Address32.Address.Minimum +
                                               res->Data.Address32.Address.TranslationOffset);
                        }
                    }
                    else
                    {
                        if (ops->memrange)
                        {
                            (*ops->memrange)(arg->dev, arg->context,
                                             res->Data.Address32.Address.Minimum,
                                             res->Data.Address32.Address.Maximum,
                                             res->Data.Address32.Address.AddressLength,
                                             res->Data.Address32.Address.Granularity);
                        }
                    }
                    break;
                case ACPI_IO_RANGE:
                    if (ADDRESS32_FIXED2(res))
                    {
                        if (ops->ioport)
                        {
                            (*ops->ioport)(arg->dev, arg->context,
                                           res->Data.Address32.Address.Minimum,
                                           res->Data.Address32.Address.AddressLength);
                        }
                    }
                    else
                    {
                        if (ops->iorange)
                        {
                            (*ops->iorange)(arg->dev, arg->context,
                                            res->Data.Address32.Address.Minimum,
                                            res->Data.Address32.Address.Maximum,
                                            res->Data.Address32.Address.AddressLength,
                                            res->Data.Address32.Address.Granularity);
                        }
                    }
                    break;
                case ACPI_BUS_NUMBER_RANGE:
                    ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES,
                                      "Address32/BusNumber unimplemented\n"));
                    break;
            }
#undef ADDRESS32_FIXED2
            break;

        case ACPI_RESOURCE_TYPE_ADDRESS16:
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Address16 unimplemented\n"));
            break;

        case ACPI_RESOURCE_TYPE_ADDRESS64:
#ifdef _LP64
            /* XXX Only fixed size supported for now */
            if (res->Data.Address64.Address.AddressLength == 0)
            {
                break;
            }
#define ADDRESS64_FIXED2(r)                                       \
    ((r)->Data.Address64.MinAddressFixed == ACPI_ADDRESS_FIXED && \
     (r)->Data.Address64.MaxAddressFixed == ACPI_ADDRESS_FIXED)
            switch (res->Data.Address64.ResourceType)
            {
                case ACPI_MEMORY_RANGE:
                    if (ADDRESS64_FIXED2(res))
                    {
                        if (ops->memory)
                        {
                            (*ops->memory)(arg->dev, arg->context,
                                           res->Data.Address64.Address.Minimum,
                                           res->Data.Address64.Address.AddressLength,
                                           res->Data.Address64.Address.Minimum +
                                               res->Data.Address64.Address.TranslationOffset);
                        }
                    }
                    else
                    {
                        if (ops->memrange)
                        {
                            (*ops->memrange)(arg->dev, arg->context,
                                             res->Data.Address64.Address.Minimum,
                                             res->Data.Address64.Address.Maximum,
                                             res->Data.Address64.Address.AddressLength,
                                             res->Data.Address64.Address.Granularity);
                        }
                    }
                    break;
                case ACPI_IO_RANGE:
                    if (ADDRESS64_FIXED2(res))
                    {
                        if (ops->ioport)
                        {
                            (*ops->ioport)(arg->dev, arg->context,
                                           res->Data.Address64.Address.Minimum,
                                           res->Data.Address64.Address.AddressLength);
                        }
                    }
                    else
                    {
                        if (ops->iorange)
                        {
                            (*ops->iorange)(arg->dev, arg->context,
                                            res->Data.Address64.Address.Minimum,
                                            res->Data.Address64.Address.Maximum,
                                            res->Data.Address64.Address.AddressLength,
                                            res->Data.Address64.Address.Granularity);
                        }
                    }
                    break;
                case ACPI_BUS_NUMBER_RANGE:
                    ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES,
                                      "Address64/BusNumber unimplemented\n"));
                    break;
            }
#undef ADDRESS64_FIXED2
#else
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Address64 unimplemented\n"));
#endif
            break;
        case ACPI_RESOURCE_TYPE_EXTENDED_ADDRESS64:
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Extended address64 unimplemented\n"));
            break;

        case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
            if (!arg->include_producer && res->Data.ExtendedIrq.ProducerConsumer != ACPI_CONSUMER)
            {
                ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "ignored ExtIRQ producer\n"));
                break;
            }
            for (i = 0; i < res->Data.ExtendedIrq.InterruptCount; i++)
            {
                ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "ExtIRQ %u\n",
                                  res->Data.ExtendedIrq.Interrupts[i]));
                if (ops->irq)
                {
                    (*ops->irq)(arg->dev, arg->context, res->Data.ExtendedIrq.Interrupts[i],
                                res->Data.ExtendedIrq.Triggering);
                }
            }
            break;

        case ACPI_RESOURCE_TYPE_GENERIC_REGISTER:
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "GenericRegister unimplemented\n"));
            break;

        case ACPI_RESOURCE_TYPE_VENDOR:
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "VendorSpecific unimplemented\n"));
            break;

        default:
            ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES, "Unknown resource type: %u\n", res->Type));
            break;
    }

    return_ACPI_STATUS(AE_OK);
}

/*
 * acpi_resource_parse:
 *
 *	Parse a device node's resources and fill them in for the
 *	client.
 *
 *	This API supports _CRS (current resources) and
 *	_PRS (possible resources).
 *
 *	Note that it might be nice to also locate ACPI-specific resource
 *	items, such as GPE bits.
 */
ACPI_STATUS
acpi_resource_parse(device_t dev, ACPI_HANDLE handle, char *path, void *arg,
                    const struct acpi_resource_parse_ops *ops)
{
    struct resource_parse_callback_arg cbarg;
    ACPI_STATUS rv;

    ACPI_FUNCTION_TRACE(__func__);

    if (ops->init)
    {
        (*ops->init)(dev, arg, &cbarg.context);
    }
    else
    {
        cbarg.context = arg;
    }
    cbarg.ops = ops;
    cbarg.dev = dev;
    cbarg.include_producer = false;

    rv = AcpiWalkResources(handle, path, acpi_resource_parse_callback, &cbarg);
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("ACPI: unable to get %s resources: %s\n", path, AcpiFormatException(rv));
        return_ACPI_STATUS(rv);
    }

    if (ops->fini)
    {
        (*ops->fini)(dev, cbarg.context);
    }

    return_ACPI_STATUS(AE_OK);
}

/*
 * acpi_resource_parse_any:
 *
 *	Parse a device node's resources and fill them in for the
 *	client. Like acpi_resource_parse, but doesn't skip ResourceProducer
 *	type resources.
 */
ACPI_STATUS
acpi_resource_parse_any(device_t dev, ACPI_HANDLE handle, char *path, void *arg,
                        const struct acpi_resource_parse_ops *ops)
{
    struct resource_parse_callback_arg cbarg;
    ACPI_STATUS rv;

    ACPI_FUNCTION_TRACE(__func__);

    if (ops->init)
    {
        (*ops->init)(dev, arg, &cbarg.context);
    }
    else
    {
        cbarg.context = arg;
    }
    cbarg.ops = ops;
    cbarg.dev = dev;
    cbarg.include_producer = true;

    rv = AcpiWalkResources(handle, path, acpi_resource_parse_callback, &cbarg);
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("ACPI: unable to get %s resources: %s\n", path, AcpiFormatException(rv));
        return_ACPI_STATUS(rv);
    }

    if (ops->fini)
    {
        (*ops->fini)(dev, cbarg.context);
    }

    return_ACPI_STATUS(AE_OK);
}