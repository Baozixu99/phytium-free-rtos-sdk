/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * 
 * FilePath: fscmi_sensors.h
 * Date: 2023-01-12 20:53:01
 * LastEditTime: 2023-01-12 20:53:01
 * Description:  This file is for cpu sensor message struct
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2023/01/13 init
 */

#ifndef FSCMI_SENSORS_PROTOCOL_H
#define FSCMI_SENSORS_PROTOCOL_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fparameters.h"
#include "fscmi.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Sensor Type Enumerations */
enum FScmiSensorType
{
    NONE = 0x0,
    UNSPECIFIED,
    DEGRESS_C,
    DEGRESS_F,
    DEGRESS_K,
};

enum FScmiSensorProtocolCmd
{
    SENSOR_VERSION_GET = 0x0,
    SENSOR_ATTRIBUTES_GET = 0x1,
    SENSOR_DESCRIPTION_GET = 0x3,
    SENSOR_CONFIG_SET = 0x4,
    SENSOR_TRIP_POINT_SET = 0x5,
    SENSOR_READING_GET = 0x6,
};

struct FScmiMsgRespSensorAttributes
{
    u16 num_sensors;
    u8 max_requests;
    u8 reserved;
    u32 reg_addr_low;
    u32 reg_addr_high;
    u32 reg_size;
};

struct FScmiMsgRespSensorDescription
{
    u16 num_returned;
    u16 num_remaining;
    struct
    {
        u32 id;
        u32 attributes_low;
#define SUPPORTS_ASYNC_READ(x) ((x)&BIT(31))
#define NUM_TRIP_POINTS(x)     ((x)&0xff)
        u32 attributes_high;
#define SENSOR_TYPE(x)         ((x)&0xff)
#define SENSOR_SCALE(x)        (((x) >> 11) & 0x1f)
#define SENSOR_UPDATE_SCALE(x) (((x) >> 22) & 0x1f)
#define SENSOR_UPDATE_BASE(x)  (((x) >> 27) & 0x1f)
        u8 name[FSCMI_MAX_STR_SIZE];
    } desc[0];
};

struct FScmiMsgSetSensorConfig
{
    u32 id;
    u32 event_control;
};

struct FScmiMsgSetSensorTripPoint
{
    u32 id;
    u32 event_control;
#define SENSOR_TP_EVENT_MASK (0x3)
#define SENSOR_TP_DISABLED   0x0
#define SENSOR_TP_POSITIVE   0x1
#define SENSOR_TP_NEGATIVE   0x2
#define SENSOR_TP_BOTH       0x3
#define SENSOR_TP_ID(x)      (((x)&0xff) << 4)
    u32 value_low;
    u32 value_high;
};

struct FcmiMsgSensorReadingGet
{
    u32 id;
    u32 flags;
#define SENSOR_READ_ASYNC BIT(0)
};

FError FScmiSensorInit(FScmi *instance_p, boolean poll_completion);

#ifdef __cplusplus
}
#endif


#endif
