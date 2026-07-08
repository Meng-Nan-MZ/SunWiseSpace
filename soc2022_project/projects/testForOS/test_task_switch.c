#include "test_task_switch.h"
#include "xprintf.h"
#include "XYR3201.h"
#include <stdint.h>

/* 测试配置 */
#define TASK_SWITCH_TEST_COUNT    1
#define TASK_SWITCH_TEST_STACK_SIZE     (configMINIMAL_STACK_SIZE * 2)
#define TASK_SWITCH_TEST_PRIORITY_HIGH  (tskIDLE_PRIORITY + 3)
#define TASK_SWITCH_TEST_PRIORITY_LOW   (tskIDLE_PRIORITY + 2)

/* 测试数据 */
static volatile uint32_t g_switchTimes[TASK_SWITCH_TEST_COUNT];
static volatile uint32_t g_testCount = 0;
static volatile uint32_t g_t1 = 0;  // 任务1放弃CPU的时间戳
static volatile uint32_t g_t2 = 0;  // 任务2开始执行的时间戳

/* 任务句柄 */
static TaskHandle_t xHighTaskHandle = NULL;
static TaskHandle_t xLowTaskHandle = NULL;

static inline uint32_t get_cpu_cycles(void)
{
    uint32_t cycles;
    __asm__ volatile ("rdcycle %0" : "=r" (cycles));
    return cycles;
}
 
/* 高优先级任务 */
static void vHighPriorityTask(void *pvParameters)
{ 
    xprintf("========== 任务切换时间测试 ==========\r\n");
    xprintf("测试说明: 测量从任务1放弃执行到任务2开始执行的时间\r\n");
    xprintf("高优先级任务1启动\r\n");
    
    // 记录任务1放弃CPU前的时间戳t1
    g_t1 = get_cpu_cycles();
    
    // 放弃CPU，让低优先级任务2执行
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 显示测试结果
    uint32_t switch_time = g_switchTimes[0];
    xprintf("任务1记录放弃CPU时间: %lu cycles\r\n", g_t1);
    xprintf("\r\n========== 任务切换时间结果 ==========\r\n");
    xprintf("任务切换时间: %lu CPU cycles\r\n", switch_time);
    
    xprintf("========== 测试完成 ==========\r\n\r\n");
    
    // 删除任务
    vTaskDelete(NULL);
}
 
/* 低优先级任务 */
static void vLowPriorityTask(void *pvParameters)
{
    // 记录任务2开始执行的时间戳t2
    g_t2 = get_cpu_cycles();
    xprintf("低优先级任务2启动\r\n");
    xprintf("任务2记录开始执行时间: %lu cycles\r\n", g_t2);
    
    // 计算切换时间
    uint32_t switch_time = g_t2 - g_t1;
    g_switchTimes[0] = switch_time;
    
    xprintf("任务切换时间: %lu CPU cycles\r\n", switch_time);
    
    // 更新测试计数
    g_testCount = 1;
    
    xprintf("低优先级任务2完成\r\n");
    
    // 删除任务
    vTaskDelete(NULL);
}
 
 
/* 启动任务切换测试 */
void vStartTaskSwitchTest(void)
{
    xprintf("\n========== 任务切换时间测试 ==========\n");
    
    // 重置测试数据
    g_testCount = 0;
    g_switchTimes[0] = 0;
    
    // 创建高优先级任务1
    if (xTaskCreate(vHighPriorityTask, "HighTask", TASK_SWITCH_TEST_STACK_SIZE, 
                    NULL, TASK_SWITCH_TEST_PRIORITY_HIGH, &xHighTaskHandle) != pdPASS)
    {
        xprintf("错误: 无法创建高优先级任务1\r\n");
        return;
    }

    // 创建低优先级任务2
    if (xTaskCreate(vLowPriorityTask, "LowTask", TASK_SWITCH_TEST_STACK_SIZE, 
                    NULL, TASK_SWITCH_TEST_PRIORITY_LOW, &xLowTaskHandle) != pdPASS)
    {
        xprintf("错误: 无法创建低优先级任务2\r\n");
        vTaskDelete(xHighTaskHandle);
        return;
    }
    
    xprintf("任务切换时间测试任务创建成功\r\n");
}