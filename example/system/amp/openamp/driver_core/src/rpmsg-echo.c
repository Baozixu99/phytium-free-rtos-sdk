/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: rpmsg-echo.c
 * Date: 2022-03-08 22:00:15
 * LastEditTime: 2022-03-09 10:01:19
 * Description:  This is a sample demonstration application that showcases usage of rpmsg
 *  This application is meant to run on the remote CPU running baremetal code.
 *  This application echoes back data that was sent to it by the driver core.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/06/20      first release
 */

/***************************** Include Files *********************************/

#include <stdio.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg_service.h"
#include <metal/sleep.h>
#include "rsc_table.h"
#include "fcache.h"
#include "fdebug.h"

/************************** Constant Definitions *****************************/
/***************** Macros (Inline Functions) Definitions *********************/

#define SHUTDOWN_MSG                0xEF56A55A

#define     ECHO_DEV_DEVICE_DEBUG_TAG "    DEVICE_01"
#define     ECHO_DEV_DEVICE_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( ECHO_DEV_DEVICE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     ECHO_DEV_DEVICE_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( ECHO_DEV_DEVICE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     ECHO_DEV_DEVICE_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( ECHO_DEV_DEVICE_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Variable Definitions *****************************/
static struct rpmsg_endpoint lept;
static int shutdown_req = 0;

/************************** Function Prototypes ******************************/

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    (void)priv;
    (void)src;

    /* On reception of a shutdown we signal the application to terminate */
    if ((*(unsigned int *)data) == SHUTDOWN_MSG)
    {
        ECHO_DEV_DEVICE_DEBUG_I("Shutdown message is received.\r\n");
        shutdown_req = 1;
        return RPMSG_SUCCESS;
    }

#ifdef CONFIG_MEM_NORMAL
    FCacheDCacheInvalidateRange((intptr)data, len);
#endif

    /* Send data back to driver */
    if (rpmsg_send(ept, data, len) < 0)
    {
        ECHO_DEV_DEVICE_DEBUG_E("rpmsg_send failed.\r\n");
    }

    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    ECHO_DEV_DEVICE_DEBUG_I("Unexpected remote endpoint destroy.\r\n");
    shutdown_req = 1;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
static int app(struct rpmsg_device *rdev, void *priv)
{
    int ret;
    shutdown_req = 0;
    /* Initialize RPMSG framework */
    ECHO_DEV_DEVICE_DEBUG_I("Try to create rpmsg endpoint.\r\n");

    ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME, 0x4, RPMSG_ADDR_ANY, rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (ret)
    {
        ECHO_DEV_DEVICE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }

    ECHO_DEV_DEVICE_DEBUG_I("Successfully created rpmsg endpoint.\r\n");

    while (1)
    {
        platform_poll(priv);

        /* we got a shutdown request, exit */
        if (shutdown_req)
        {
            break;
        }
    }

    rpmsg_destroy_ept(&lept);

    return 0;
}


/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int rpmsg_echo(struct rpmsg_device *rdev, void *priv)
{
    metal_assert(rdev);
    metal_assert(priv);
    int ret;
    ECHO_DEV_DEVICE_DEBUG_I("Starting rpmsg_echo application...\r\n");
    ret = app(rdev, priv);
    if (ret != 0)
    {
        ECHO_DEV_DEVICE_DEBUG_E("Rpmsg_echo application error,code:0x%x",ret);
        return ret;
    }

    ECHO_DEV_DEVICE_DEBUG_I("Stopping rpmsg_echo application...\r\n");

    return ret;
}
