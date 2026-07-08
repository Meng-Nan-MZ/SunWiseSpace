#include "test_stability.h"
#include "xprintf.h"
#include "XYR3201.h"
#include <stdint.h>


#define TASK_STACK_SIZE          configMINIMAL_STACK_SIZE


static volatile uint32_t g_t1 = 0;
static volatile uint32_t g_t2 = 0;
static volatile uint32_t g_t3 = 0;
static volatile uint32_t g_t4 = 0;
static volatile uint32_t g_t5 = 0;
static volatile uint32_t g_t6 = 0;
static volatile uint32_t g_switchTime1 = 0;
static volatile uint32_t g_switchTime2 = 0;
static volatile uint32_t g_switchTime3 = 0;

/* CPU周期计数函数 */
static inline uint32_t get_cpu_cycles(void) {
    uint32_t cycles;
    __asm__ volatile("rdcycle %0" : "=r"(cycles));
    return cycles;
}

/* 高优先级任务1 */
static void vHighPriorityTask(void *pvParameters) {
    xprintf("========== 任务切换时间测试 ==========\r\n");
    xprintf("高优先级任务1启动\r\n");
    
    /* 1 */
    g_t1 = get_cpu_cycles();
    

    vTaskDelay(pdMS_TO_TICKS(100));
    
    /* 2 */
    g_t3 = get_cpu_cycles();
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    /* 3 */
    g_t5 = get_cpu_cycles();
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    xprintf("\r\n========== 任务切换时间结果 ==========\r\n");
    xprintf("第一次切换时间: %lu CPU cycles\r\n", g_switchTime1);
    xprintf("第二次切换时间: %lu CPU cycles\r\n", g_switchTime2);
    xprintf("第三次切换时间: %lu CPU cycles\r\n", g_switchTime3);
    
    if (g_switchTime1 > 0 && g_switchTime2 > 0 && g_switchTime3 > 0) {
        uint32_t avg = (g_switchTime1 + g_switchTime2 + g_switchTime3) / 3;
        xprintf("平均切换时间: %lu CPU cycles\r\n", avg);
    }
    
    xprintf("========== 测试完成 ==========\r\n\r\n");
    
    vTaskDelete(NULL);
}

/* 低优先级任务2 */
static void vLowPriorityTask(void *pvParameters) {

    g_t2 = get_cpu_cycles();
    
    /* 计算第一次切换时间 */
    if (g_t1 > 0 && g_t2 > g_t1) {
        g_switchTime1 = g_t2 - g_t1;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));

    g_t4 = get_cpu_cycles();
    
    /* 2 */
    if (g_t3 > 0 && g_t4 > g_t3) {
        g_switchTime2 = g_t4 - g_t3;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));

    g_t6 = get_cpu_cycles();
    
    /* 3*/
    if (g_t5 > 0 && g_t6 > g_t5) {
        g_switchTime3 = g_t6 - g_t5;
    }
    
    xprintf("任务2完成\r\n");
    vTaskDelete(NULL);
}

/* 负载任务 */
static void vLoadTask(void *pvParameters) {
    while (1) {
        volatile uint32_t dummy = 0;
        for (uint32_t i = 0; i < 50; i++) {
            dummy += i;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}


static void run_scenario_test(uint32_t loadTaskCount, const char *scenarioName) {
    BaseType_t xResult;
    uint32_t i;
    
    xprintf("\n========== %s ==========\r\n", scenarioName);
    xprintf("负载任务数量: %lu\r\n", (unsigned long)loadTaskCount);
    
    g_t1 = 0;
    g_t2 = 0;
    g_t3 = 0;
    g_t4 = 0;
    g_t5 = 0;
    g_t6 = 0;
    g_switchTime1 = 0;
    g_switchTime2 = 0;
    g_switchTime3 = 0;
    
    /* 创建负载任务 */
    for (i = 0; i < loadTaskCount; i++) {
        xResult = xTaskCreate(vLoadTask, "LoadTask", TASK_STACK_SIZE,
                             (void *)(uintptr_t)i, 0, NULL);
        if (xResult != pdPASS) {
            xprintf("错误: 无法创建负载任务 %lu\r\n", (unsigned long)i);
            return;
        }
    }
    
    /* 创建低优先级任务2 */
    xResult = xTaskCreate(vLowPriorityTask, "Task2", TASK_STACK_SIZE,
                         NULL, 2, NULL);
    if (xResult != pdPASS) {
        xprintf("错误: 无法创建任务2\r\n");
        return;
    }
    
    /* 创建高优先级任务1 */
    xResult = xTaskCreate(vHighPriorityTask, "Task1", TASK_STACK_SIZE,
                         NULL, 3, NULL);
    if (xResult != pdPASS) {
        xprintf("错误: 无法创建任务1\r\n");
        return;
    }
    
    xprintf("测试任务创建成功，开始测试...\r\n");
}

/* 启动任务切换稳定性测试 */
void vStartStabilityTest(void) {
    xprintf("\r\n\r\n");
    xprintf("测试不同负载下的任务切换时间\r\n");
    
    /* 测试场景1: 最小使用场景 - 0个负载任务（只有2个测量任务） */
    //run_scenario_test(0, "最小使用场景 (2个任务)");
    
    run_scenario_test(10, "一般使用场景 (12个任务)");
}

