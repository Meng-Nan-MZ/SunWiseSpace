#include "test_priority.h"
#include "FreeRTOS.h"
#include "task.h"
#include "xprintf.h"

/* 配置 */
#define PRIO_TEST_STACK (configMINIMAL_STACK_SIZE)
#define PRIO_BASE (tskIDLE_PRIORITY + 2)
#define PRIO_HIGHER (tskIDLE_PRIORITY + 3)

static TaskHandle_t xWorkerHandle = NULL;

static void vWorkerTask(void *arg) {
  UBaseType_t prio = uxTaskPriorityGet(NULL);
  for (;;) {
    xprintf("[优先级测试] worker 运行中, 当前优先级=%u\n", (unsigned)prio);
    vTaskDelay(pdMS_TO_TICKS(500));
    prio = uxTaskPriorityGet(NULL);
  }
}

BaseType_t OSTaskPrioritySetSafe(TASK_ID tid, UBaseType_t newPriority) {
  if (newPriority >= configMAX_PRIORITIES) {
    xprintf("[优先级测试] 错误: 新优先级%u 超出范围(最大=%u)\n",
            (unsigned)newPriority, (unsigned)(configMAX_PRIORITIES - 1));
    vTaskPrioritySet(tid, newPriority);
    return ERROR;
  }

  /* 校验句柄有效性：通过获取状态，避免非法句柄导致异常 */
  eTaskState s = eTaskGetState(tid);
  if (s == eInvalid || s == eDeleted) {
    xprintf("[优先级测试] 错误: 无效的任务句柄\n");
    vTaskPrioritySet(tid, newPriority);
    return ERROR;
  }

  vTaskPrioritySet(tid, newPriority);
  return OK;
}

static void vTaskPriorityTestMain(void *arg) {
  BaseType_t ret;
  UBaseType_t prio;

  xprintf("\n========== 任务优先级设置测试==========%n");

  /* 用例1：有效任务与优先级 */
  xprintf("\n[用例1] 设置有效任务的优先级\n");
  ret = xTaskCreate(vWorkerTask, "prioWorker", PRIO_TEST_STACK, NULL, PRIO_BASE,
                    &xWorkerHandle);
  if (ret != pdPASS || xWorkerHandle == NULL) {
    xprintf("[优先级测试] ❌ 创建worker任务失败\n");
  }

  vTaskDelay(pdMS_TO_TICKS(1000));

  xprintf("[优先级测试] 尝试将worker优先级设置为%u\n", (unsigned)PRIO_HIGHER);
  ret = OSTaskPrioritySetSafe(xWorkerHandle, PRIO_HIGHER);
  if (ret == OK) {
    prio = uxTaskPriorityGet(xWorkerHandle);
    xprintf("[优先级测试] ✅ 设置成功，当前优先级=%u\n", (unsigned)prio);
  } else {
    xprintf("[优先级测试] ❌ 设置失败(不应失败)\n");
  }

  vTaskDelay(pdMS_TO_TICKS(1000));

  if (xWorkerHandle) {
    vTaskDelete(xWorkerHandle);
    xWorkerHandle = NULL;
  }
  vTaskDelete(NULL);
}

void vStartTaskPriorityTest(void) {
  if (xTaskCreate(vTaskPriorityTestMain, "prioTest", PRIO_TEST_STACK * 2, NULL,
                  PRIO_BASE, NULL) != pdPASS)
    xprintf("[优先级测试] 创建测试主任务失败\n");
}