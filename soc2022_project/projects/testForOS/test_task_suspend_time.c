#include "test_task_suspend_time.h"
#include "utils/u_time.h"
#include "xprintf.h"

#define TIME_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define TIME_TEST_PRIORITY (tskIDLE_PRIORITY + 2)

static TaskHandle_t xTimeTestTaskHandle = NULL;

// 记录挂起一个处于就绪状态任务所需的时间
static void vTaskSuspendWithTime(TaskHandle_t xTaskToSuspend) {
  uint64_t cycle_start = get_cpu_cycle();
  vTaskSuspend(xTaskToSuspend);
  uint64_t cycle_end = get_cpu_cycle();
  xprintf("[任务时间测试] 挂起任务耗时: %lu cycles\n",
          (uint32_t)(cycle_end - cycle_start));
}

static void vTimeTestTask(void *pvParameters) {
  vTaskDelay(pdMS_TO_TICKS(200));
  vTaskDelete(NULL);
}

static void vTaskSuspendTimeTestMain(void *pvParameters) {
  TaskHandle_t xTestHandle = NULL;
  xprintf("\n[INFO] 开始 FreeRTOS 任务挂起时间测试...\n\n");
  xprintf("========== 挂起任务时间 ==========\n");
  for (int i = 0; i < 5; ++i) {
    xTaskCreate(vTimeTestTask, "TimeTestTask", TIME_TEST_STACK_SIZE, NULL,
                TIME_TEST_PRIORITY, &xTestHandle);
    if (xTestHandle != NULL) {
      vTaskSuspendWithTime(xTestHandle);
      vTaskResume(xTestHandle);
      // 等待任务结束
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
  xprintf("\n[任务时间测试] 测试任务完成\n");
  vTaskDelete(NULL);
}

void vStartTaskSuspendTimeTest(void) {
  BaseType_t xResult;
  xprintf("\n[INFO] 启动 FreeRTOS 任务挂起时间测试...\n");
  xResult =
      xTaskCreate(vTaskSuspendTimeTestMain, "TaskSuspendTimeTest", TIME_TEST_STACK_SIZE * 2,
                  NULL, TIME_TEST_PRIORITY - 1, NULL);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建任务挂起时间测试主任务失败\n");
  } else {
    xprintf("[INFO] 任务挂起时间测试启动成功\n");
  }
}
