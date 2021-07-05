
// #include "printf-stdarg.h"
#include "ft_types.h"
#include "exception.h"
#include <stdio.h>
#include "aarch64.h"

u64 raw_read_elr_el1(void)
{
    u64 elr_el1;

    __asm__ __volatile__("mrs %0, ELR_EL1\n\t"
                         : "=r"(elr_el1)
                         :
                         : "memory");

    return elr_el1;
}

u64 raw_read_sp_el1(void)
{
    // u64 sp_el1;

    // __asm__ __volatile__("mrs %0, sp\n\t"
    //                      : "=r"(sp_el1)
    //                      :
    //                      : "memory");

    return 0;
}

void _origin_print(void)
{
    printf("check is here \r\n");
}

void test_freertos_print(u64 x0, u64 x1, u64 x2)
{
    printf("x0 %x \r\n", x0);
    printf("x1 %x \r\n", x1);
    printf("x2 %x \r\n", x2);
    printf("esr_el1 %x \r\n", get_esr_el1());
    // printf("elr_el1 %x \r\n", raw_read_elr_el1());
    // printf("spsr_el1 %x \r\n", raw_read_spsr_el1());
    // printf("sp_el1 %x \r\n", raw_read_sp_el1());
    while (1)
    {
        /* code */
    }
}

void handle_exception(exception_frame *exc)
{
    printf("elr_el1 %x \r\n", raw_read_elr_el1());
    printf("An exception occur:\r\n");
    printf("exc_type: ");
    printf("%x", exc->exc_type);
    printf("\r\nESR: ");
    printf("%x", exc->exc_esr);
    printf("  SP: ");
    printf("%x", exc->exc_sp);
    printf(" ELR: ");
    printf("%x", exc->exc_elr);
    printf(" SPSR: ");
    printf("%x", exc->exc_spsr);
    printf("\r\n x0: ");
    printf("%x", exc->x0);
    printf("  x1: ");
    printf("%x", exc->x1);
    printf("  x2: ");
    printf("%x", exc->x2);
    printf("  x3: ");
    printf("%x", exc->x3);
    printf("\r\n x4: ");
    printf("%x", exc->x4);
    printf("  x5: ");
    printf("%x", exc->x5);
    printf("  x6: ");
    printf("%x", exc->x6);
    printf("  x7: ");
    printf("%x", exc->x7);
    printf("\r\n x8: ");
    printf("%x", exc->x8);
    printf("  x9: ");
    printf("%x", exc->x9);
    printf(" x10: ");
    printf("%x", exc->x10);
    printf(" x11: ");
    printf("%x", exc->x11);
    printf("\r\nx12: ");
    printf("%x", exc->x12);
    printf(" x13: ");
    printf("%x", exc->x13);
    printf(" x14: ");
    printf("%x", exc->x14);
    printf(" x15: ");
    printf("%x", exc->x15);
    printf("\r\nx16: ");
    printf("%x", exc->x16);
    printf(" x17: ");
    printf("%x", exc->x17);
    printf(" x18: ");
    printf("%x", exc->x18);
    printf(" x19: ");
    printf("%x", exc->x19);
    printf("\r\nx20: ");
    printf("%x", exc->x20);
    printf(" x21: ");
    printf("%x", exc->x21);
    printf(" x22: ");
    printf("%x", exc->x22);
    printf(" x23: ");
    printf("%x", exc->x23);
    printf("\r\nx24: ");
    printf("%x", exc->x24);
    printf(" x25: ");
    printf("%x", exc->x25);
    printf(" x26: ");
    printf("%x", exc->x26);
    printf(" x27: ");
    printf("%x", exc->x27);
    printf("\r\nx28: ");
    printf("%x", exc->x28);
    printf(" x29: ");
    printf("%x", exc->x29);
    printf(" x30: ");
    printf("%x", exc->x30);
}