#include "test_semaphore.h"
#include "xprintf.h"

#define SIMPLE_TASK_STACK_SIZE  (configMINIMAL_STACK_SIZE)
#define PRODUCER_PRIORITY       (tskIDLE_PRIORITY + 3)
#define CONSUMER_PRIORITY       (tskIDLE_PRIORITY + 2)
#define CREATE_TEST_PRIORITY    (tskIDLE_PRIORITY + 1)

static SemaphoreHandle_t xTestSemaphore = NULL;
static volatile uint32_t ulTestCount = 0;

/* 辅助任务函数 - 用于阻塞测试 */
static void vHelperTask1(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(2000));  /* 等待2秒 */
    xSemaphoreGive((SemaphoreHandle_t)pvParameters);  /* 释放信号量 */
    xprintf("[信号量获取测试] 辅助任务1: 释放信号量\n");
    vTaskDelete(NULL);
}

/* 辅助任务函数 - 用于限时测试 */
static void vHelperTask2(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(500));  /* 等待500ms */
    xSemaphoreGive((SemaphoreHandle_t)pvParameters);  /* 释放信号量 */
    xprintf("[信号量获取测试] 辅助任务2: 在500ms后释放信号量\n");
    vTaskDelete(NULL);
}

/* 信号量创建函数实现 */
SEM_ID OSSemBCreate(SEM_B_STATE init)
{
    SemaphoreHandle_t xSemaphore = xSemaphoreCreateBinary();
    
    if (xSemaphore == NULL)
    {
        return NULL;
    }
    
    /* FreeRTOS二值信号量创建后初始状态是"已占用" */
    /* 如果初始状态为空闲，需要先释放信号量 */
    if (init == SEM_B_FREE)
    {
        xSemaphoreGive(xSemaphore);  /* 释放信号量，使其变为空闲状态 */
    }
    /* 如果初始状态为已占用，保持默认状态（已占用） */
    
    return xSemaphore;
}

SEM_ID OSSemCCreate(U32 initCount)
{
    /* 创建计数信号量，最大计数值设为初始计数值的2倍或至少10 */
    U32 maxCount = (initCount > 5) ? (initCount * 2) : 10;
    
    return xSemaphoreCreateCounting(maxCount, initCount);
}

static void vProducerTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for (uint32_t i = 1; i <= 3; i++)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
        
        if (xTestSemaphore != NULL)
        {
            xSemaphoreGive(xTestSemaphore);
            xprintf("[信号量测试] 生产者: 发送信号量 #%d\n", i);
        }
    }
    
    xprintf("[信号量测试] 生产者任务完成\n");
    vTaskDelete(NULL);
}

static void vConsumerTask(void *pvParameters)
{
    uint32_t ulReceived = 0;
    
    while (ulReceived < 3)
    {
        if (xTestSemaphore != NULL)
        {
            if (xSemaphoreTake(xTestSemaphore, pdMS_TO_TICKS(2000)) == pdTRUE)
            {
                ulReceived++;
                ulTestCount++;
                xprintf("[信号量测试] 消费者: 接收信号量 #%d\n", ulReceived);
                
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            else
            {
                xprintf("[信号量测试] 消费者: 接收超时\n");
                break;
            }
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(500));
    
    if (ulTestCount == 3)
    {
        xprintf("\n========== 信号量测试结果 ==========\n");
        xprintf("✅ 二进制信号量基本功能: 通过\n");
        xprintf("📊 成功传递 %d 个信号量\n", ulTestCount);
        xprintf("🎉 信号量测试完成！\n");
        xprintf("=====================================\n\n");
    }
    else
    {
        xprintf("\n========== 信号量测试结果 ==========\n");
        xprintf("❌ 二进制信号量基本功能: 失败\n");
        xprintf("📊 仅传递 %d/3 个信号量\n", ulTestCount);
        xprintf("⚠️  测试失败，请检查\n");
        xprintf("=====================================\n\n");
    }
    
    xprintf("[信号量测试] 消费者任务完成\n");
    vTaskDelete(NULL);
}

/* 信号量创建测试主任务 */
static void vSemaphoreCreateTestMain(void *pvParameters)
{
    SEM_ID xBinarySem = NULL;
    SEM_ID xCountingSem = NULL;
    //SEM_ID =0x1004EE00;
    uint32_t ulPassCount = 0;
    uint32_t ulFailCount = 0;
    
    xprintf("\n========== 信号量创建测试 ==========\n");
    xprintf("[INFO] 开始 FreeRTOS 信号量创建功能测试...\n\n");
    
    /* ========== 用例1: 创建二值信号量 ========== */
    xprintf("========== 用例1: 创建二值信号量 ==========\n");
    
    xBinarySem = OSSemBCreate(SEM_B_FREE);
    if (xBinarySem != NULL)
    {
        xprintf("[信号量创建测试] ✅ 二值信号量创建成功，句柄=0x%08X\n", (uint32_t)xBinarySem);
        ulPassCount++;
    }
    else
    {
        xprintf("[信号量创建测试] ❌ 二值信号量创建失败\n");
        ulFailCount++;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    /* ========== 用例2: 创建计数信号量 ========== */
    xprintf("\n========== 用例2: 创建计数信号量 ==========\n");
    
    xCountingSem = OSSemCCreate(5);
    if (xCountingSem != NULL)
    {
        xprintf("[信号量创建测试] ✅ 计数信号量创建成功，句柄=0x%08X\n", (uint32_t)xCountingSem);
        ulPassCount++;
    }
    else
    {
        xprintf("[信号量创建测试] ❌ 计数信号量创建失败\n");
        ulFailCount++;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    /* ========== 用例3: 删除信号量 ========== */
    xprintf("\n========== 用例3: 删除信号量 ==========\n");
    
    /* 删除二值信号量 */
    if (xBinarySem != NULL)
    {
        vSemaphoreDelete(xBinarySem);
        xprintf("[信号量创建测试] ✅ 二值信号量删除成功\n");
        xBinarySem = NULL;
    }
    
    /* 删除计数信号量 */
    if (xCountingSem != NULL)
    {
        vSemaphoreDelete(xCountingSem);
        xprintf("[信号量创建测试] ✅ 计数信号量删除成功\n");
        xCountingSem = NULL;
    }
    
    //xprintf("[信号量创建测试] 测试删除无效句柄...\n");
    //vSemaphoreDelete(InvalidSem);
    //xprintf("[信号量创建测试] ✅ 删除无效句柄: 安全通过\n");
    //ulPassCount++;

    vTaskDelete(NULL);
}

void vStartSemaphoreTest(void)
{
    BaseType_t xResult;
    
    xprintf("\n[INFO] 开始简化版 FreeRTOS 信号量测试...\n");
    xprintf("[INFO] 任务栈大小: %d 字节\n", SIMPLE_TASK_STACK_SIZE * sizeof(StackType_t));
    
    /* 启动信号量创建测试 */
    xResult = xTaskCreate(vSemaphoreCreateTestMain, "SemCreateTest", 
                          SIMPLE_TASK_STACK_SIZE * 2, NULL, 
                          CREATE_TEST_PRIORITY, NULL);
    if (xResult != pdPASS)
    {
        xprintf("[ERROR] 创建信号量创建测试任务失败\n");
        return;
    }
    
}

/* 信号量获取测试主任务 */
static void vSemaphoreAcquireTestMain(void *pvParameters)
{
    SEM_ID xTestSem = NULL;
    BaseType_t xResult;
    uint32_t ulPassCount = 0;
    uint32_t ulFailCount = 0;
    
    xprintf("\n========== 信号量获取测试 ==========\n");
    xprintf("[INFO] 开始 FreeRTOS 信号量获取功能测试...\n\n");
    
    /* 创建测试用的二值信号量 */
    xTestSem = OSSemBCreate(SEM_B_FREE);
    if (xTestSem == NULL)
    {
        xprintf("[信号量获取测试] ❌ 创建测试信号量失败\n");
        vTaskDelete(NULL);
        return;
    }
    
    xprintf("[信号量获取测试] ✅ 测试信号量创建成功，句柄=0x%08X\n", (uint32_t)xTestSem);
    
    /* ========== 用例1: 非阻塞方式锁定信号量 (5.1.9) ========== */
    xprintf("\n========== 用例1: 非阻塞方式锁定信号量 (5.1.9) ==========\n");
    
    /* 测试1.1: 信号量可用时，非阻塞获取应该成功 */
    xprintf("[信号量获取测试] 测试1.1: 信号量可用时的非阻塞获取\n");
    xResult = xSemaphoreTake(xTestSem, 0);  /* NO_WAIT = 0 */
    if (xResult == pdTRUE)
    {
        xprintf("[信号量获取测试] ✅ 非阻塞获取成功（信号量可用）\n");
        ulPassCount++;
        
        /* 释放信号量供后续测试使用 */
        xSemaphoreGive(xTestSem);
    }
    else
    {
        xprintf("[信号量获取测试] ❌ 非阻塞获取失败（信号量可用时不应失败）\n");
        ulFailCount++;
    }
    
    /* 测试1.2: 信号量不可用时，非阻塞获取应该失败但不阻塞 */
    xprintf("[信号量获取测试] 测试1.2: 信号量不可用时的非阻塞获取\n");
    xSemaphoreTake(xTestSem, 0);  /* 先占用信号量 */
    xResult = xSemaphoreTake(xTestSem, 0);  /* 再次尝试获取 */
    if (xResult == pdFALSE)
    {
        xprintf("[信号量获取测试] ✅ 非阻塞获取失败（信号量不可用时正确返回失败）\n");
        ulPassCount++;
    }
    else
    {
        xprintf("[信号量获取测试] ❌ 非阻塞获取成功（信号量不可用时不应成功）\n");
        ulFailCount++;
    }
    
    /* 释放信号量供后续测试使用 */
    xSemaphoreGive(xTestSem);
    
    vTaskDelay(pdMS_TO_TICKS(200));
    
    /* ========== 用例2: 阻塞方式锁定信号量 (5.1.10) ========== */
    xprintf("\n========== 用例2: 阻塞方式锁定信号量 (5.1.10) ==========\n");
    
    /* 测试2.1: 信号量可用时，阻塞获取应该立即成功 */
    xprintf("[信号量获取测试] 测试2.1: 信号量可用时的阻塞获取\n");
    xResult = xSemaphoreTake(xTestSem, portMAX_DELAY);  /* WAIT_FOREVER = portMAX_DELAY */
    if (xResult == pdTRUE)
    {
        xprintf("[信号量获取测试] ✅ 阻塞获取成功（信号量可用时立即成功）\n");
        ulPassCount++;
        
        /* 释放信号量供后续测试使用 */
        xSemaphoreGive(xTestSem);
    }
    else
    {
        xprintf("[信号量获取测试] ❌ 阻塞获取失败（信号量可用时不应失败）\n");
        ulFailCount++;
    }
    
    /* 测试2.2: 信号量不可用时，阻塞获取应该等待 */
    xprintf("[信号量获取测试] 测试2.2: 信号量不可用时的阻塞获取\n");
    xprintf("[信号量获取测试] 请等待...\n");
    
    /* 先占用信号量 */
    xSemaphoreTake(xTestSem, 0);
    
    /* 创建辅助任务来释放信号量 */
    TaskHandle_t xHelperTask = NULL;
    xTaskCreate(vHelperTask1, "Helper1", configMINIMAL_STACK_SIZE, xTestSem, tskIDLE_PRIORITY + 4, &xHelperTask);
    
    /* 尝试阻塞获取信号量 */
    TickType_t xStartTime = xTaskGetTickCount();
    xResult = xSemaphoreTake(xTestSem, portMAX_DELAY);
    TickType_t xEndTime = xTaskGetTickCount();
    
    if (xResult == pdTRUE)
    {
        TickType_t xWaitTime = xEndTime - xStartTime;
        xprintf("[信号量获取测试] ✅ 阻塞获取成功，等待时间: %u ticks\n", (unsigned)xWaitTime);
        ulPassCount++;
        xSemaphoreGive(xTestSem);
    }
    else
    {
        xprintf("[信号量获取测试] ❌ 阻塞获取失败（不应失败）\n");
        ulFailCount++;
    }
    
    vTaskDelay(pdMS_TO_TICKS(200));
    
    /* ========== 用例3: 限时等待方式获取信号量 (5.1.11) ========== */
    xprintf("\n========== 用例3: 限时等待方式获取信号量 (5.1.11) ==========\n");
    
    /* 测试3.1: 信号量可用时，限时获取应该立即成功 */
    xprintf("[信号量获取测试] 测试3.1: 信号量可用时的限时获取\n");
    xResult = xSemaphoreTake(xTestSem, pdMS_TO_TICKS(1000));  /* 1秒超时 */
    if (xResult == pdTRUE)
    {
        xprintf("[信号量获取测试] ✅ 限时获取成功（信号量可用时立即成功）\n");
        ulPassCount++;
        
        /* 释放信号量供后续测试使用 */
        xSemaphoreGive(xTestSem);
    }
    else
    {
        xprintf("[信号量获取测试] ❌ 限时获取失败（信号量可用时不应失败）\n");
        ulFailCount++;
    }
    
    /* 测试3.2: 信号量不可用时，限时获取应该超时 */
    xprintf("[信号量获取测试] 测试3.2: 信号量不可用时的限时获取\n");
    xprintf("[信号量获取测试] 请等待...\n");
    
    /* 先占用信号量 */
    xSemaphoreTake(xTestSem, 0);
    
    /* 尝试限时获取信号量 */
    xStartTime = xTaskGetTickCount();
    xResult = xSemaphoreTake(xTestSem, pdMS_TO_TICKS(1000));  /* 1秒超时 */
    xEndTime = xTaskGetTickCount();
    
    if (xResult == pdFALSE)
    {
        TickType_t xWaitTime = xEndTime - xStartTime;
        xprintf("[信号量获取测试] ✅ 限时获取超时（信号量不可用时正确超时），等待时间: %u ticks\n", (unsigned)xWaitTime);
        ulPassCount++;
    }
    else
    {
        xprintf("[信号量获取测试] ❌ 限时获取成功（信号量不可用时不应成功）\n");
        ulFailCount++;
    }
    
    /* 测试3.3: 信号量在超时前被释放，应该成功获取 */
    xprintf("[信号量获取测试] 测试3.3: 信号量在超时前被释放\n");
    xprintf("[信号量获取测试] 注意：此测试会等待500ms，请等待...\n");
    
    /* 创建辅助任务来在500ms后释放信号量 */
    xTaskCreate(vHelperTask2, "Helper2", configMINIMAL_STACK_SIZE, xTestSem, tskIDLE_PRIORITY + 4, NULL);
    
    /* 尝试限时获取信号量（1秒超时，但信号量会在500ms后被释放） */
    xStartTime = xTaskGetTickCount();
    xResult = xSemaphoreTake(xTestSem, pdMS_TO_TICKS(1000));  /* 1秒超时 */
    xEndTime = xTaskGetTickCount();
    
    if (xResult == pdTRUE)
    {
        TickType_t xWaitTime = xEndTime - xStartTime;
        xprintf("[信号量获取测试] ✅ 限时获取成功（信号量在超时前被释放），等待时间: %u ticks\n", (unsigned)xWaitTime);
        ulPassCount++;
    }
    else
    {
        xprintf("[信号量获取测试] ❌ 限时获取失败（信号量在超时前被释放时应成功）\n");
        ulFailCount++;
    }
    
    /* 清理资源 */
    if (xTestSem != NULL)
    {
        vSemaphoreDelete(xTestSem);
        xTestSem = NULL;
    }
    
    /* ========== 测试结果总结 ========== */
    vTaskDelay(pdMS_TO_TICKS(500));
    
    xprintf("\n========== 信号量获取测试结果 ==========\n");
    xprintf("✅ 通过: %u 个测试用例\n", (unsigned)ulPassCount);
    xprintf("❌ 失败: %u 个测试用例\n", (unsigned)ulFailCount);
    
    if (ulFailCount == 0)
    {
        xprintf("🎉 所有信号量获取测试通过！\n");
    }
    else
    {
        xprintf("⚠️  部分测试失败，请检查实现\n");
    }
    xprintf("=====================================\n\n");
    
    xprintf("[信号量获取测试] 测试任务完成\n");
    vTaskDelete(NULL);
}

/* 启动信号量获取测试 */
void vStartSemaphoreAcquireTest(void)
{
    BaseType_t xResult;
    
    xprintf("\n[INFO] 开始 FreeRTOS 信号量获取功能测试...\n");
    
    xResult = xTaskCreate(vSemaphoreAcquireTestMain, "SemAcquireTest", 
                          SIMPLE_TASK_STACK_SIZE * 2, NULL, 
                          CREATE_TEST_PRIORITY, NULL);
    
    if (xResult != pdPASS)
    {
        xprintf("[ERROR] 创建信号量获取测试任务失败\n");
    }
    else
    {
        xprintf("[INFO] 信号量获取测试任务启动成功\n");
    }
}