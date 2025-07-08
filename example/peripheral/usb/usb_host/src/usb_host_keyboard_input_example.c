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
 * FilePath: usb_host_keyboard_input_example.c
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

#include "usb_host_keyboard_input_example.h"
/************************** Constant Definitions *****************************/
#define HID_KEYCODE_TO_ASCII    \
    {0     , 0      }, /* 0x00 */ \
    {0     , 0      }, /* 0x01 */ \
    {0     , 0      }, /* 0x02 */ \
    {0     , 0      }, /* 0x03 */ \
    {'a'   , 'A'    }, /* 0x04 */ \
    {'b'   , 'B'    }, /* 0x05 */ \
    {'c'   , 'C'    }, /* 0x06 */ \
    {'d'   , 'D'    }, /* 0x07 */ \
    {'e'   , 'E'    }, /* 0x08 */ \
    {'f'   , 'F'    }, /* 0x09 */ \
    {'g'   , 'G'    }, /* 0x0a */ \
    {'h'   , 'H'    }, /* 0x0b */ \
    {'i'   , 'I'    }, /* 0x0c */ \
    {'j'   , 'J'    }, /* 0x0d */ \
    {'k'   , 'K'    }, /* 0x0e */ \
    {'l'   , 'L'    }, /* 0x0f */ \
    {'m'   , 'M'    }, /* 0x10 */ \
    {'n'   , 'N'    }, /* 0x11 */ \
    {'o'   , 'O'    }, /* 0x12 */ \
    {'p'   , 'P'    }, /* 0x13 */ \
    {'q'   , 'Q'    }, /* 0x14 */ \
    {'r'   , 'R'    }, /* 0x15 */ \
    {'s'   , 'S'    }, /* 0x16 */ \
    {'t'   , 'T'    }, /* 0x17 */ \
    {'u'   , 'U'    }, /* 0x18 */ \
    {'v'   , 'V'    }, /* 0x19 */ \
    {'w'   , 'W'    }, /* 0x1a */ \
    {'x'   , 'X'    }, /* 0x1b */ \
    {'y'   , 'Y'    }, /* 0x1c */ \
    {'z'   , 'Z'    }, /* 0x1d */ \
    {'1'   , '!'    }, /* 0x1e */ \
    {'2'   , '@'    }, /* 0x1f */ \
    {'3'   , '#'    }, /* 0x20 */ \
    {'4'   , '$'    }, /* 0x21 */ \
    {'5'   , '%'    }, /* 0x22 */ \
    {'6'   , '^'    }, /* 0x23 */ \
    {'7'   , '&'    }, /* 0x24 */ \
    {'8'   , '*'    }, /* 0x25 */ \
    {'9'   , '('    }, /* 0x26 */ \
    {'0'   , ')'    }, /* 0x27 */ \
    {'\r'  , '\r'   }, /* 0x28 */ \
    {'\x1b', '\x1b' }, /* 0x29 */ \
    {'\b'  , '\b'   }, /* 0x2a */ \
    {'\t'  , '\t'   }, /* 0x2b */ \
    {' '   , ' '    }, /* 0x2c */ \
    {'-'   , '_'    }, /* 0x2d */ \
    {'='   , '+'    }, /* 0x2e */ \
    {'['   , '{'    }, /* 0x2f */ \
    {']'   , '}'    }, /* 0x30 */ \
    {'\\'  , '|'    }, /* 0x31 */ \
    {'#'   , '~'    }, /* 0x32 */ \
    {';'   , ':'    }, /* 0x33 */ \
    {'\''  , '\"'   }, /* 0x34 */ \
    {'`'   , '~'    }, /* 0x35 */ \
    {','   , '<'    }, /* 0x36 */ \
    {'.'   , '>'    }, /* 0x37 */ \
    {'/'   , '?'    }, /* 0x38 */ \
                                  \
    {0     , 0      }, /* 0x39 */ \
    {0     , 0      }, /* 0x3a */ \
    {0     , 0      }, /* 0x3b */ \
    {0     , 0      }, /* 0x3c */ \
    {0     , 0      }, /* 0x3d */ \
    {0     , 0      }, /* 0x3e */ \
    {0     , 0      }, /* 0x3f */ \
    {0     , 0      }, /* 0x40 */ \
    {0     , 0      }, /* 0x41 */ \
    {0     , 0      }, /* 0x42 */ \
    {0     , 0      }, /* 0x43 */ \
    {0     , 0      }, /* 0x44 */ \
    {0     , 0      }, /* 0x45 */ \
    {0     , 0      }, /* 0x46 */ \
    {0     , 0      }, /* 0x47 */ \
    {0     , 0      }, /* 0x48 */ \
    {0     , 0      }, /* 0x49 */ \
    {0     , 0      }, /* 0x4a */ \
    {0     , 0      }, /* 0x4b */ \
    {0     , 0      }, /* 0x4c */ \
    {0     , 0      }, /* 0x4d */ \
    {0     , 0      }, /* 0x4e */ \
    {0     , 0      }, /* 0x4f */ \
    {0     , 0      }, /* 0x50 */ \
    {0     , 0      }, /* 0x51 */ \
    {0     , 0      }, /* 0x52 */ \
    {0     , 0      }, /* 0x53 */ \
                                  \
    {'/'   , '/'    }, /* 0x54 */ \
    {'*'   , '*'    }, /* 0x55 */ \
    {'-'   , '-'    }, /* 0x56 */ \
    {'+'   , '+'    }, /* 0x57 */ \
    {'\r'  , '\r'   }, /* 0x58 */ \
    {'1'   , 0      }, /* 0x59 */ \
    {'2'   , 0      }, /* 0x5a */ \
    {'3'   , 0      }, /* 0x5b */ \
    {'4'   , 0      }, /* 0x5c */ \
    {'5'   , '5'    }, /* 0x5d */ \
    {'6'   , 0      }, /* 0x5e */ \
    {'7'   , 0      }, /* 0x5f */ \
    {'8'   , 0      }, /* 0x60 */ \
    {'9'   , 0      }, /* 0x61 */ \
    {'0'   , 0      }, /* 0x62 */ \
    {'0'   , 0      }, /* 0x63 */ \
    {'='   , '='    }, /* 0x67 */ \

enum
{
    USB_HOST_OP_SUCCESS = 0,
    USB_HOST_OP_FAILED    = 1,
};

#define USB_INPUT_WAIT_TIME    (pdMS_TO_TICKS(20000UL))
#define TIMER_OUT              (USB_INPUT_WAIT_TIME + (pdMS_TO_TICKS(10000UL)))
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static u8 const keycode2ascii[128][2] =  { HID_KEYCODE_TO_ASCII };
static struct usbh_urb kbd_intin_urb;
static uint8_t kbd_buffer[128] __attribute__((aligned(sizeof(unsigned long)))) = {0};
static struct usb_osal_timer *kbd_timer = NULL;
static uintptr_t usb_id = 0;
static QueueHandle_t xQueue = NULL;
/***************** Macros (Inline Functions) Definitions *********************/
#define FUSB_DEBUG_TAG "USB-KEYBOARD"
#define FUSB_ERROR(format, ...) FT_DEBUG_PRINT_E(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_WARN(format, ...)  FT_DEBUG_PRINT_W(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_INFO(format, ...)  FT_DEBUG_PRINT_I(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_DEBUG(format, ...) FT_DEBUG_PRINT_D(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/


/*****************************************************************************/
/* look up new key in previous keys */
static inline boolean FindKeyInPrevInput(const struct usb_hid_kbd_report *report, u8 keycode)
{
    for (u8 i = 0; i < 6; i++)
    {
        if (report->key[i] == keycode)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void UsbKeyBoardHandleInput(struct usb_hid_kbd_report *input)
{
    static struct usb_hid_kbd_report prev_input = { 0, 0, {0} }; /* previous report to check key released */

    /* ------------- example code ignore control (non-printable) key affects ------------- */
    for (u8 i = 0; i < 6; i++)
    {
        if (input->key[i])
        {
            if (FindKeyInPrevInput(&prev_input, input->key[i]))
            {
                /* exist in previous report means the current key is holding */
            }
            else
            {
                /* not existed in previous report means the current key is pressed */
                boolean is_shift = input->modifier & (HID_MODIFER_LSHIFT | HID_MODIFER_RSHIFT);
                u8 ch = keycode2ascii[input->key[i]][is_shift ? 1 : 0];
                putchar(ch);
                if ('\r' == ch)
                {
                    putchar('\n');
                }

                fflush(stdout); /* flush right away, else libc will wait for newline */
            }
        }
    }

    prev_input = *input;
}

static void UsbKeyboardCallback(void *arg, int nbytes)
{
    struct usbh_hid *hid_class = (struct usbh_hid *)arg;

    if (nbytes > 0) 
    {
        UsbKeyBoardHandleInput((struct usb_hid_kbd_report *)kbd_buffer);
    }

    if (kbd_timer) 
    {
        usb_osal_timer_start(kbd_timer);
    }
}

static void UsbKeyboardTimeout(void *arg)
{
    struct usbh_hid *hid = (struct usbh_hid *)arg;

    usbh_int_urb_fill(&hid->intin_urb, hid->hport, hid->intin, kbd_buffer, hid->intin->wMaxPacketSize, 0, UsbKeyboardCallback, hid);
    usbh_submit_urb(&hid->intin_urb);
}

static void XhciUsbKeyboardTask(void *args)
{
    int ret;
    u8 id = (u8)usb_id;
    struct usbh_hid *kbd_class;
    const char *devname = (const char *)args;

    kbd_class = (struct usbh_hid *)usbh_find_class_instance(devname);
    if (kbd_class == NULL)
    {
        FUSB_ERROR("Do not find %s.", devname);
        goto task_exit;
    }

    kbd_timer = usb_osal_timer_create("keyboard_timer",
                                      USBH_GET_URB_INTERVAL(kbd_class->intin->bInterval, kbd_class->hport->speed),
                                      UsbKeyboardTimeout,
                                      kbd_class,
                                      false);
    if (kbd_timer != NULL)
    {
        usb_osal_timer_start(kbd_timer);
    }    

    /* check hid device information */
    printf("HID (/dev/input%d) info:\r\n", (char)kbd_class->minor);
    printf("    USB: 0x%04x\r\n", kbd_class->hport->device_desc.bcdUSB);
    printf("    Class: 0x%02x\r\n", kbd_class->hport->device_desc.bDeviceClass);
    printf("    Subclass: 0x%02x\r\n", kbd_class->hport->device_desc.bDeviceSubClass);
    printf("    Protocol: 0x%02x\r\n", kbd_class->hport->device_desc.bDeviceProtocol);
    printf("    Intr in: %d(mps) %d(interval)\r\n", kbd_class->intin->wMaxPacketSize, 
                kbd_class->intin->bInterval);
    printf("    Intr out: %d(mps) %d(interval)\r\n", kbd_class->intout->wMaxPacketSize,
                kbd_class->intout->bInterval);

    printf("Now you can type in some characters from keyboard \r\n");

    vTaskDelay(USB_INPUT_WAIT_TIME); /* wait user input for a while */

    printf("Input finished, exit...\r\n");

task_exit:
    ret = (ret == 0) ? USB_HOST_OP_SUCCESS : USB_HOST_OP_FAILED;
    xQueueSend(xQueue, &ret, 0);
    vTaskDelete(NULL);    
}

BaseType_t FFreeRTOSKeyboardInput(u32 id, const char *devname)
{
    BaseType_t ret = pdPASS;
    int task_res = USB_HOST_OP_SUCCESS;
    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        FUSB_ERROR("xQueue create failed.\r\n");
        goto exit;
    }

    usb_id = id;

    ret = xTaskCreate((TaskFunction_t)XhciUsbKeyboardTask,
                      (const char *)"XhciUsbKeyboardTask",
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

    if (task_res != USB_HOST_OP_SUCCESS)
    {
        printf("%s@%d: XHCI usb keyboard example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: XHCI usb keyboard example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}