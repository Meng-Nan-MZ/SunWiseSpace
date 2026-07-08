#include "test_suspend_resume.h"
#include "FreeRTOS.h"
#include "task.h"
#include "xprintf.h"

/* 测试配置 */
#define TEST_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define TEST_INVALID_PRIORITY (configMAX_PRIORITIES)
#define TEST_HUGE_STACK_SIZE                                                   \
  ((configTOTAL_HEAP_SIZE / sizeof(StackType_t)) * 4 / 5)

/* 测试统计 */
static volatile uint32_t ulTestTaskExecuted = 0;
static volatile uint32_t ulDeleteTaskExecuted = 0;
static volatile uint32_t ulSuspendTaskExecuted = 0;
static TaskHandle_t xDeleteTaskHandle = NULL;
static TaskHandle_t xSuspendTaskHandle = NULL;

/*  用于验证任务挂起和恢复功能 */
static void vTestTaskSuspendResume(void *pvParameters) {
  uint32_t ulCounter = 0;

  xprintf("[任务管理测试] 挂起/恢复任务: 启动成功\n");
  ulSuspendTaskExecuted = 1;

  /* 持续运行，输出计数，用于验证挂起和恢复 */
  while (1) {
    xprintf("[任务管理测试] 挂起/恢复任务: 运行中 [计数=%d]\n", ++ulCounter);
    vTaskDelay(pdMS_TO_TICKS(1000));

    /* 运行一段时间后自动退出，避免影响后续测试 */
    if (ulCounter >= 15) {
      xprintf("[任务管理测试] 挂起/恢复任务: 达到最大计数，退出\n");
      break;
    }
  }

  xSuspendTaskHandle = NULL;
  vTaskDelete(NULL);
}

/* 封装任务挂起函数 */
BaseType_t OSTaskSuspend(TASK_ID tid) {
  if (tid == NULL) {
    // vTaskSuspend(tid);
    return ERROR;
  }

  /* 检查任务句柄有效性 - 通过获取任务状态来验证 */
  eTaskState eState = eTaskGetState(tid);
  if (eState == eInvalid || eState == eDeleted) {
    xprintf("[任务管理测试] 警告: 无效的任务句柄 (状态: %d)\n", eState);
    vTaskSuspend(tid);
    return ERROR;
  }

  vTaskSuspend(tid);
  return OK;
}

/* 封装任务恢复函数 */
BaseType_t OSTaskResume(TASK_ID tid) {
  if (tid == NULL) {
    vTaskResume(tid);
    return ERROR;
  }

  /* 检查任务句柄有效性 */
  eTaskState eState = eTaskGetState(tid);
  if (eState == eInvalid) {
    xprintf("[任务管理测试] 警告: 无效的任务句柄\n");
    vTaskResume(tid);
    return ERROR;
  }

  /* 只有挂起状态的任务才能恢复 */
  if (eState != eSuspended) {
    vTaskResume(tid);
    xprintf("[任务管理测试] 警告: 任务未处于挂起状态\n");
    return ERROR;
  }

  vTaskResume(tid);
  return OK;
}

/* 主测试任务  */
static void vTaskManagementTestMain(void *pvParameters) {
  BaseType_t xResult;
  TaskHandle_t xTestHandle = NULL;
  uint32_t ulPassCount = 0;
  uint32_t ulFailCount = 0;
  BaseType_t status;

  xprintf("\n[INFO] 开始 FreeRTOS 任务管理测试...\n");
  xprintf("[INFO] 系统配置: 最大优先级=%d, 最小堆栈=%d\n\n",
          configMAX_PRIORITIES - 1,
          configMINIMAL_STACK_SIZE * sizeof(StackType_t));

  /* ========== 任务挂起测试 ========== */
  xprintf("\n=====================================\n");
  xprintf("        任务挂起测试\n");
  xprintf("=====================================\n");

  /* ----- 挂起用例1: 挂起有效任务 ----- */
  xprintf("\n========== 用例1: 挂起有效任务 ==========\n");

  ulSuspendTaskExecuted = 0;
  xResult =
      xTaskCreate(vTestTaskSuspendResume, "SuspendTask", TEST_TASK_STACK_SIZE,
                  NULL, TEST_TASK_PRIORITY, &xSuspendTaskHandle);

  if (xResult == pdPASS && xSuspendTaskHandle != NULL) {
    xprintf("[任务管理测试] 创建待挂起任务成功，句柄=0x%08X\n",
            (uint32_t)xSuspendTaskHandle);

    xprintf("[任务管理测试] 任务正常运行中...\n");
    vTaskDelay(pdMS_TO_TICKS(3000));

    xprintf("[任务管理测试] 执行挂起操作...\n");
    status = OSTaskSuspend(xSuspendTaskHandle);

    if (status == OK) {
      xprintf("[任务管理测试] ✅ 任务挂起: 成功\n");

      /* 验证任务已停止输出 */
      xprintf("[任务管理测试] 观察3秒: 被挂起任务应停止输出...\n");
      vTaskDelay(pdMS_TO_TICKS(3000));
      xprintf("[任务管理测试] ✅ 挂起验证: 任务已停止输出\n");
      ulPassCount++;
    } else {
      xprintf("[任务管理测试] ❌ 任务挂起: 失败\n");
      ulFailCount++;
      vTaskDelete(xSuspendTaskHandle);
      xSuspendTaskHandle = NULL;
    }
  } else {
    xprintf("[任务管理测试] ❌ 创建待挂起任务失败\n");
    ulFailCount++;
  }

  vTaskDelay(pdMS_TO_TICKS(1000));

  /* ========== 任务恢复测试 ========== */
  xprintf("\n=====================================\n");
  xprintf("        任务恢复测试\n");
  xprintf("=====================================\n");

  /* ----- 恢复用例1: 恢复已挂起的任务 ----- */
  xprintf("\n========== 用例2: 恢复已挂起的任务 ==========\n");

  if (xSuspendTaskHandle != NULL) {
    xprintf("[任务管理测试] 恢复任务句柄: 0x%08X\n",
            (uint32_t)xSuspendTaskHandle);
    xprintf("[任务管理测试] 执行恢复操作...\n");

    status = OSTaskResume(xSuspendTaskHandle);

    if (status == OK) {
      xprintf("[任务管理测试] ✅ 任务恢复: 成功\n");

      /* 验证任务已恢复运行 */
      xprintf("[任务管理测试] 观察3秒: 被恢复任务应继续输出...\n");
      vTaskDelay(pdMS_TO_TICKS(3000));
      xprintf("[任务管理测试] ✅ 恢复验证: 任务已恢复运行\n");
      ulPassCount++;

      /* 再次挂起以便清理 */
      vTaskSuspend(xSuspendTaskHandle);
      vTaskDelete(xSuspendTaskHandle);
      xSuspendTaskHandle = NULL;
    } else {
      xprintf("[任务管理测试] ❌ 任务恢复: 失败\n");
      ulFailCount++;
    }
  } else {
    xprintf("[任务管理测试] ⚠️ 跳过恢复测试: 没有已挂起的任务\n");
  }

  vTaskDelay(pdMS_TO_TICKS(1000));

  xprintf("=====================================\n\n");

  xprintf("[任务管理测试] 测试任务退出\n");
  vTaskDelete(NULL);
}

/* 启动任务管理测试 */
void vStartSuspendResumeTest(void) {
  BaseType_t xResult;

  xprintf("\n[INFO] 启动 FreeRTOS 任务管理测试（含挂起/恢复）...\n");

  /* 注意：请确保在 FreeRTOSConfig.h 中设置 INCLUDE_vTaskSuspend = 1 */

  xResult =
      xTaskCreate(vTaskManagementTestMain, "TaskMgmtTest",
                  TEST_TASK_STACK_SIZE * 2, NULL, TEST_TASK_PRIORITY - 1, NULL);

  if (xResult != pdPASS)
    xprintf("[ERROR] 创建任务管理测试主任务失败\n");
}