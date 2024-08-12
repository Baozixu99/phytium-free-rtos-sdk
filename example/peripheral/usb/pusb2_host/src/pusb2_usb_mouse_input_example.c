/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: usb_input.c
 * Date: 2022-07-22 13:57:42
 * LastEditTime: 2022-07-22 13:57:43
 * Description:  This file is for the usb input functions.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/9/20  init commit
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fassert.h"
#include "fdebug.h"
#include "fkernel.h"

#include "usbh_core.h"
#include "usbh_hid.h"

#include "pusb2_host_example.h"
/************************** Constant Definitions *****************************/
enum
{
    PUSB2_USB_MOUSE_SUCCESS = 0,
    PUSB2_USB_MOUSE_FAIL    = 1,
};

#define PUSB2_INPUT_WAIT_TIME    (pdMS_TO_TICKS(20000UL))
#define TIMER_OUT               (PUSB2_INPUT_WAIT_TIME + (pdMS_TO_TICKS(10000UL)))
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static struct usbh_urb mouse_intin_urb;
static uint8_t mouse_buffer[128] __attribute__((aligned(sizeof(unsigned long)))) = {0};
static struct usb_osal_timer *mouse_timer = NULL;
static uintptr_t usb_id = 0;
static QueueHandle_t xQueue = NULL;
/***************** Macros (Inline Functions) Definitions *********************/
#define FUSB_DEBUG_TAG "USB-MOUSE"
#define FUSB_ERROR(format, ...) FT_DEBUG_PRINT_E(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_WARN(format, ...)  FT_DEBUG_PRINT_W(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_INFO(format, ...)  FT_DEBUG_PRINT_I(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_DEBUG(format, ...) FT_DEBUG_PRINT_D(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/


/*****************************************************************************/
static inline void UsbMouseLeftButtonCB(void)
{
    printf("<-\r\n");
}

static inline void UsbMouseRightButtonCB(void)
{
    printf("->\r\n");
}

static inline void UsbMouseMiddleButtonCB(void)
{
    printf("C\r\n");
}

static void UsbMouseHandleInput(struct usb_hid_mouse_report *input)
{
    /*------------- button state  -------------*/
    if (input->buttons & HID_MOUSE_INPUT_BUTTON_LEFT)
    {
        UsbMouseLeftButtonCB();
    }

    if (input->buttons & HID_MOUSE_INPUT_BUTTON_MIDDLE)
    {
        UsbMouseMiddleButtonCB();
    }

    if (input->buttons & HID_MOUSE_INPUT_BUTTON_RIGHT)
    {
        UsbMouseRightButtonCB();
    }

    /*------------- cursor movement -------------*/
    printf("[x:%d y:%d w:%d]\r\n", input->xdisp, input->ydisp, input->wdisp);
}

static void UsbMouseCallback(void *arg, int nbytes)
{
    struct usbh_hid *hid_class = (struct usbh_hid *)arg;

    if (nbytes > 0) 
    {
        UsbMouseHandleInput((struct usb_hid_mouse_report *)mouse_buffer);
    }

    if (mouse_timer)
    {
        usb_osal_timer_start(mouse_timer);
    }
}

static void UsbMouseTimeout(void *arg)
{
    struct usbh_hid *hid = (struct usbh_hid *)arg;

    usbh_int_urb_fill(&hid->intin_urb, hid->hport, hid->intin, mouse_buffer, hid->intin->wMaxPacketSize, 0, UsbMouseCallback, hid);
    usbh_submit_urb(&hid->intin_urb);
}

static void PUSB2UsbMouseTask(void *args)
{
    int ret;
    u8 id = (u8)usb_id;
    struct usbh_hid *mouse_class;
    const char *devname = (const char *)args;

    mouse_class = (struct usbh_hid *)usbh_find_class_instance(devname);
    if (mouse_class == NULL)
    {
        FUSB_ERROR("Do not find %s.", devname);
        goto task_exit;
    }

    mouse_timer = usb_osal_timer_create("mouse_timer",
                                      USBH_GET_URB_INTERVAL(mouse_class->intin->bInterval, mouse_class->hport->speed),
                                      UsbMouseTimeout,
                                      mouse_class,
                                      false);
    if (mouse_timer != NULL)
    {
        usb_osal_timer_start(mouse_timer);
    }    

    /* check hid device information */
    printf("HID (/dev/input%d) info:\r\n", (char)mouse_class->minor);
    printf("    USB: 0x%04x\r\n", mouse_class->hport->device_desc.bcdUSB);
    printf("    Class: 0x%02x\r\n", mouse_class->hport->device_desc.bDeviceClass);
    printf("    Subclass: 0x%02x\r\n", mouse_class->hport->device_desc.bDeviceSubClass);
    printf("    Protocol: 0x%02x\r\n", mouse_class->hport->device_desc.bDeviceProtocol);
    printf("    Intr in: %d(mps) %d(interval)\r\n", mouse_class->intin->wMaxPacketSize, 
                mouse_class->intin->bInterval);
    printf("    Intr out: %d(mps) %d(interval)\r\n", mouse_class->intout->wMaxPacketSize,
                mouse_class->intout->bInterval);

    printf("Now you can move and click connected mouse \r\n");

    vTaskDelay(PUSB2_INPUT_WAIT_TIME); /* wait user input for a while */

    printf("Input finished, exit...\r\n");

task_exit:
    ret = (ret == 0) ? PUSB2_USB_MOUSE_SUCCESS : PUSB2_USB_MOUSE_FAIL;
    xQueueSend(xQueue, &ret, 0);
    vTaskDelete(NULL);    
}

void usbh_hid_run(struct usbh_hid *hid_class)
{
    struct usbh_hubport *hport = hid_class->hport;
    uint8_t intf = hid_class->intf;
    
    if (HID_PROTOCOL_KEYBOARD == hport->config.intf[intf].altsetting[0].intf_desc.bInterfaceProtocol) 
    {
        printf("**/dev/input%d is keyboard \r\n", hid_class->minor);    
    } 
    else if (HID_PROTOCOL_MOUSE == hport->config.intf[intf].altsetting[0].intf_desc.bInterfaceProtocol) 
    {
        printf("**/dev/input%d is mouse \r\n", hid_class->minor);    
    }
}

BaseType_t FFreeRTOSRunPUSB2Mouse(u32 id, const char *devname)
{
    BaseType_t ret = pdPASS;
    int task_res = PUSB2_USB_MOUSE_SUCCESS;
    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        FUSB_ERROR("xQueue create failed.\r\n");
        goto exit;
    }

    usb_id = id;

    ret = xTaskCreate((TaskFunction_t)PUSB2UsbMouseTask,
                      (const char *)"PUSB2UsbMouseTask",
                      (uint16_t)4096,
                      (void *)devname,
                      (UBaseType_t)configMAX_PRIORITIES - 1,
                      NULL);
    if (ret != pdPASS)
    {
        FUSB_ERROR("xTaskCreate create failed.\r\n");
        goto exit;        
    }

    ret = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (ret != pdPASS)
    {
        FUSB_ERROR("xQueue receive timeout.\r\n");
        goto exit;        
    }

exit:
    if (xQueue)
    {
        vQueueDelete(xQueue);
    }

    if (task_res != PUSB2_USB_MOUSE_SUCCESS)
    {
        printf("%s@%d: PUSB2 usb mouse example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: PUSB2 usb mouse example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}