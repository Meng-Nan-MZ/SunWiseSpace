#include "test_suspend_delete_validation.h"
#include "xprintf.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

/* 测试配置 */
#define TEST_STACK_SIZE     (configMINIMAL_STACK_SIZE)
#define TEST_PRIORITY       (tskIDLE_PRIORITY + 2)

/* 测试任务句柄 */
static TaskHandle_t xTestTaskHandle = NULL;

/* 测试任务 */
static void vTestTask(void *pvParameters)
{
    xprintf("测试任务启动\r\n");
    
    // 任务运行一段时间
    vTaskDelay(pdMS_TO_TICKS(100));
    
    xprintf("测试任务完成，准备删除自己\r\n");
    
    // 删除自己
    vTaskDelete(NULL);
}

/* 测试vTaskSuspend对已删除任务的处理 */
void vTestSuspendDeletedTask(void)
{
    xprintf("\n========== vTaskSuspend对已删除任务的测试 ==========\n");
    
    // 创建测试任务
    if (xTaskCreate(vTestTask, "TestTask", TEST_STACK_SIZE, NULL, TEST_PRIORITY, &xTestTaskHandle) != pdPASS)
    {
        xprintf("错误: 无法创建测试任务\r\n");
        return;
    }
    
    xprintf("测试任务创建成功，句柄: %p\r\n", xTestTaskHandle);
    
    // 等待任务执行并删除自己
    vTaskDelay(pdMS_TO_TICKS(200));
    
    // 等待空闲任务处理删除的任务（释放内存）
    xprintf("等待空闲任务处理删除的任务...\r\n");
    vTaskDelay(pdMS_TO_TICKS(1000));  // 等待1秒让空闲任务处理
    
    xprintf("\n--- 测试1: 尝试挂起已删除的任务 ---\r\n");
    xprintf("任务句柄: %p\r\n", xTestTaskHandle);
    xprintf("调用vTaskSuspend前...\r\n");
    
    // 尝试挂起已删除的任务
    vTaskSuspend(xTestTaskHandle);
    
    xprintf("调用vTaskSuspend后（没有崩溃）\r\n");
    
    xprintf("\n--- 测试2: 尝试恢复已删除的任务 ---\r\n");
    xprintf("调用vTaskResume前...\r\n");
    
    // 尝试恢复已删除的任务
    vTaskResume(xTestTaskHandle);
    
    xprintf("调用vTaskResume后（没有崩溃）\r\n");
    
    xprintf("\n--- 测试3: 检查任务状态 ---\r\n");
    eTaskState taskState = eTaskGetState(xTestTaskHandle);
    xprintf("任务状态: %d (eDeleted=%d, eInvalid=%d)\r\n", taskState, eDeleted, eInvalid);
    
    xprintf("\n========== 测试完成 ==========\n");
}

/* 测试vTaskSuspend对无效指针的处理 */
void vTestSuspendInvalidPointer(void)
{
    xprintf("\n========== vTaskSuspend对无效指针的测试 ==========\n");
    
    // 创建一个无效的任务句柄
    TaskHandle_t invalidHandle = (TaskHandle_t)0x12345678;
    
    xprintf("--- 测试4: 尝试挂起无效指针 ---\r\n");
    xprintf("无效句柄: %p\r\n", invalidHandle);
    xprintf("调用vTaskSuspend前...\r\n");
    
    // 尝试挂起无效指针
    vTaskSuspend(invalidHandle);
    
    xprintf("调用vTaskSuspend后（没有崩溃）\r\n");
    
    xprintf("\n--- 测试5: 尝试恢复无效指针 ---\r\n");
    xprintf("调用vTaskResume前...\r\n");
    
    // 尝试恢复无效指针
    vTaskResume(invalidHandle);
    
    xprintf("调用vTaskResume后（没有崩溃）\r\n");
    
    xprintf("\n========== 测试完成 ==========\n");
}

/* 启动所有测试 */
void vStartSuspendDeleteValidationTest(void)
{
    vTestSuspendDeletedTask();
    vTaskDelay(pdMS_TO_TICKS(100));
    vTestSuspendInvalidPointer();
}
