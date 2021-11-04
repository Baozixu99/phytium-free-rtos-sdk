/*
 Test for thread local storage support.
*/

#include <string.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "ft_types.h"
#include "task.h"
#include "unity.h"
#include "ft_assert.h"

static __thread int tl_test_var1;
static __thread u8 tl_test_var2 = 55;
static __thread u16 tl_test_var3 = 44;
static __thread u8 tl_test_arr_var[10];
static __thread struct test_tls_var {
    int f32;
    u8 f8;
    u16 f16;
    u8 farr[10];
} tl_test_struct_var;

static void TaskTestTls(void *arg)
{
    TEST_ASSERT(arg != NULL);
    bool *running = (bool *)arg;
    u32 tp = (u32) -1;
    int test_var1_old = 0;
    u8 test_var2_old = 0;
    u16 test_var3_old = 0;
    int f32_old = 0;
    u8 f8_old = 0;
    u16 f16_old = 0;
    
    srand(*(TickType_t*)xTaskGetCurrentTaskHandle());
    int step = (rand() % 10) + 1;
    for (int i = 0; i < 5; i++) {
        // printf("Task[%x]: var = 0x%x 0x%x step=%d\n", tp, tl_test_var1, tl_test_var2, step);
        if (i == 0) {
            TEST_ASSERT_EQUAL(0, tl_test_var1);
            TEST_ASSERT_EQUAL(55, tl_test_var2);
            TEST_ASSERT_EQUAL(44, tl_test_var3);
            for (u8 k = 0; k < sizeof(tl_test_arr_var); k++) {
                TEST_ASSERT_EQUAL(0, tl_test_arr_var[k]);
            }
            TEST_ASSERT_EQUAL(0, tl_test_struct_var.f32);
            TEST_ASSERT_EQUAL(0, tl_test_struct_var.f8);
            TEST_ASSERT_EQUAL(0, tl_test_struct_var.f16);
            for (u8 k = 0; k < sizeof(tl_test_struct_var.farr); k++) {
                TEST_ASSERT_EQUAL(0, tl_test_struct_var.farr[k]);
            }
        } else {
            TEST_ASSERT_EQUAL(test_var1_old + step, tl_test_var1);
            TEST_ASSERT_EQUAL(test_var2_old + step, tl_test_var2);
            TEST_ASSERT_EQUAL(test_var3_old + step, tl_test_var3);
            for (u8 k = 0; k < sizeof(tl_test_arr_var); k++) {
                TEST_ASSERT_EQUAL((i - 1) * step, tl_test_arr_var[k]);
            }
            TEST_ASSERT_EQUAL(f32_old + step, tl_test_struct_var.f32);
            TEST_ASSERT_EQUAL(f8_old + step, tl_test_struct_var.f8);
            TEST_ASSERT_EQUAL(f16_old + step, tl_test_struct_var.f16);
            for (u8 k = 0; k < sizeof(tl_test_struct_var.farr); k++) {
                TEST_ASSERT_EQUAL((i - 1) * step, tl_test_struct_var.farr[k]);
            }
        }

        test_var1_old = tl_test_var1;
        test_var2_old = tl_test_var2;
        test_var3_old = tl_test_var3;
        f32_old = tl_test_struct_var.f32;
        f8_old = tl_test_struct_var.f8;
        f16_old = tl_test_struct_var.f16;
        tl_test_var1 += step;
        tl_test_var2 += step;
        tl_test_var3 += step;
        memset(tl_test_arr_var, i * step, sizeof(tl_test_arr_var));
        tl_test_struct_var.f32 += step;
        tl_test_struct_var.f8 += step;
        tl_test_struct_var.f16 += step;
        memset(tl_test_struct_var.farr, i * step, sizeof(tl_test_struct_var.farr));
        vTaskDelay(10);
    }

    if (running) {
        *running = false;
        vTaskDelete(NULL);
    }
}

void ThreadLocalStorageTest(void)
{
    
    const size_t stack_size = 1024;
    StackType_t s_stack[stack_size]; /* with 8KB test task stack (default) this test still has ~3KB headroom */
    StaticTask_t s_task;
    bool running= true;
    
    TaskHandle_t handle = xTaskCreateStatic(TaskTestTls, "task_test_tls", stack_size, &running,
                                  3, s_stack, &s_task);

    /*Check static task was successfully allocated*/
    FT_ASSERTVOID(NULL!= handle);  

    /* Make sure idle task can clean up s_task, before it goes out of scope */
    vTaskDelay(10); 
   
    printf("ThreadLocalStorageTest end\n");
    while(1)
    {
        vTaskDelay(200);
    }
}
