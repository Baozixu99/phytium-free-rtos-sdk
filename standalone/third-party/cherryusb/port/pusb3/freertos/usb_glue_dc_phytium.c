#include "usbd_core.h"
#include "ftypes.h"

void usb_sys_mem_init(void);
void usb_sys_mem_deinit(void);
void usb_dc_setup_pusb3_interrupt(u32 id);
void usb_dc_revoke_pusb3_interrupt(u32 id);

void usb_dc_low_level_init(void)
{
    usb_sys_mem_init();
    usb_dc_setup_pusb3_interrupt(0U);
}

void usb_dc_low_level_deinit(void)
{
    usb_dc_revoke_pusb3_interrupt(0U);
    usb_sys_mem_deinit();
}

__WEAK void USBOTG_IRQHandler(uint8_t busid)
{
}

__WEAK void USBD_IRQHandler(uint8_t busid)
{
}
