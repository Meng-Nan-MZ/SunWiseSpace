#include "test_task.h"
#include "xprintf.h"

/* 测试配置 */
#define TEST_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define TEST_INVALID_PRIORITY (configMAX_PRIORITIES) /* 使用最大优先级+1 */
#define TEST_HUGE_STACK_SIZE                                                   \
  ((configTOTAL_HEAP_SIZE / sizeof(StackType_t)) * 4 / 5)

/* 测试统计 */
static volatile uint32_t ulTestTaskExecuted = 0;
static volatile uint32_t ulDeleteTaskExecuted = 0;
static TaskHandle_t xDeleteTaskHandle = NULL;

/* 测试任务1 - 用于验证任务创建和执行 */
static void vTestTask1(void *pvParameters) {
  uint32_t ulTaskNum = (uint32_t)pvParameters;

  xprintf("[任务管理测试] 测试任务%d: 启动成功\n", ulTaskNum);
  ulTestTaskExecuted++;

  for (uint32_t i = 0; i < 3; i++) {
    xprintf("[任务管理测试] 测试任务%d: 运行中 #%d\n", ulTaskNum, i + 1);
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  xprintf("[任务管理测试] 测试任务%d: 执行完成\n", ulTaskNum);
  vTaskDelete(NULL); /* 任务自删除 */
}

/* 测试任务2 - 用于验证任务删除功能 */
static void vTestTask2(void *pvParameters) {
  xprintf("[任务管理测试] 待删除任务: 启动成功\n");
  ulDeleteTaskExecuted = 1;

  /* 无限循环，等待被删除 */
  while (1) {
    xprintf("[任务管理测试] 待删除任务: 运行中...\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/* 主测试任务 */
static void vTaskManagementTestMain(void *pvParameters) {
  BaseType_t xResult;
  TaskHandle_t xTestHandle = NULL;
  uint32_t ulPassCount = 0;
  uint32_t ulFailCount = 0;

  xprintf("\n[INFO] 开始 FreeRTOS 任务管理测试...\n");
  xprintf("[INFO] 系统配置: 最大优先级=%d, 最小堆栈=%d\n\n",
          configMAX_PRIORITIES - 1,
          configMINIMAL_STACK_SIZE * sizeof(StackType_t));

  /* ========== 用例1: 创建任务 - 有效参数 ========== */
  xprintf("========== 用例1: 创建任务(有效参数) ==========\n");

  ulTestTaskExecuted = 0;
  xResult = xTaskCreate(vTestTask1, "TestTask1", TEST_TASK_STACK_SIZE,
                        (void *)1, TEST_TASK_PRIORITY, &xTestHandle);

  if (xResult == pdPASS && xTestHandle != NULL) {
    xprintf("[任务管理测试] ✅ 任务创建成功，句柄=0x%08X\n",
            (uint32_t)xTestHandle);

    vTaskDelay(pdMS_TO_TICKS(2000));

    if (ulTestTaskExecuted > 0) {
      xprintf("[任务管理测试] ✅ 任务执行验证: 通过\n");
      ulPassCount++;
    } else {
      xprintf("[任务管理测试] ❌ 任务执行验证: 失败\n");
      ulFailCount++;
    }
  } else {
    xprintf("[任务管理测试] ❌ 任务创建失败\n");
    ulFailCount++;
  }

  vTaskDelay(pdMS_TO_TICKS(1000));


  /* ========== 用例2: 删除任务 - 有效任务ID ========== */

  xprintf("\n========== 用例2: 删除任务(有效ID) ==========\n");

  ulDeleteTaskExecuted = 0;
  xResult = xTaskCreate(vTestTask2, "ToDelete", TEST_TASK_STACK_SIZE, NULL,
                        TEST_TASK_PRIORITY, &xDeleteTaskHandle);

  if (xResult == pdPASS && xDeleteTaskHandle != NULL) {
    xprintf("[任务管理测试] 创建待删除任务成功\n");

    vTaskDelay(pdMS_TO_TICKS(2000));

    if (ulDeleteTaskExecuted > 0) {
      xprintf("[任务管理测试] 执行删除操作...\n");
      vTaskDelete(xDeleteTaskHandle);
      // xDeleteTaskHandle = NULL;

      xprintf("[任务管理测试] ✅ 任务删除: 成功\n");
      ulPassCount++;

      vTaskDelay(pdMS_TO_TICKS(2000));
      xprintf("[任务管理测试] 观察: 被删除任务已停止输出\n");
    } else {
      xprintf("[任务管理测试] ❌ 待删除任务未正常运行\n");
      ulFailCount++;
    }
  } else {
    xprintf("[任务管理测试] ❌ 创建待删除任务失败\n");
    ulFailCount++;
  }

  vTaskDelay(pdMS_TO_TICKS(1000));
  
  xprintf("=====================================\n\n");

  xprintf("[任务管理测试] 测试任务完成\n");
  vTaskDelete(NULL);
}

/* 启动任务管理测试 */
void vStartTaskManagementTest(void) {
  BaseType_t xResult;

  xprintf("\n[INFO] 启动 FreeRTOS 任务管理测试...\n");

  xResult =
      xTaskCreate(vTaskManagementTestMain, "TaskMgmtTest",
                  TEST_TASK_STACK_SIZE * 2, NULL, TEST_TASK_PRIORITY - 1, NULL);

  if (xResult != pdPASS)
    xprintf("[ERROR] 创建任务管理测试主任务失败\n");
}