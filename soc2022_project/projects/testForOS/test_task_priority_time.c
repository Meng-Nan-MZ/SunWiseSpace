#include "test_task_priority_time.h"
#include "utils/u_time.h"
#include "xprintf.h"

#define TIME_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define TIME_TEST_PRIORITY (tskIDLE_PRIORITY + 2)

static TaskHandle_t xTimeTestTaskHandle = NULL;

// 记录修改一个处于ready状态任务优先级所需的时间
static void vTaskPrioritySetWithTime(TaskHandle_t xTask,
                                     UBaseType_t uxNewPriority) {
  for (int i = 0; i < 5; i++) {
    uint64_t cycle_start = get_cpu_cycle();
    vTaskPrioritySet(xTask, uxNewPriority);
    uint64_t cycle_end = get_cpu_cycle();
    xprintf("[任务时间测试] %d: 修改优先级耗时: %lu cycles\n", i + 1,
            (uint32_t)(cycle_end - cycle_start));
  }
}

static void vTimeTestTask(void *pvParameters) {
  vTaskDelay(pdMS_TO_TICKS(200));
  vTaskDelete(NULL);
}

static void vTaskPriorityTimeTestMain(void *pvParameters) {
  TaskHandle_t xTestHandle = NULL;
  xprintf("\n[INFO] 开始 FreeRTOS 任务优先级修改时间测试...\n\n");
  xprintf("========== 修改优先级时间 ==========\n");
  xTaskCreate(vTimeTestTask, "TimeTestTask", TIME_TEST_STACK_SIZE, NULL,
              TIME_TEST_PRIORITY, &xTestHandle);
  if (xTestHandle != NULL) {
    vTaskPrioritySetWithTime(xTestHandle, TIME_TEST_PRIORITY + 1);
  }
  xprintf("\n[任务时间测试] 测试任务完成\n");
  vTaskDelete(NULL);
}

void vStartTaskPriorityTimeTest(void) {
  BaseType_t xResult;
  xprintf("\n[INFO] 启动 FreeRTOS 任务优先级修改时间测试...\n");
  xResult =
      xTaskCreate(vTaskPriorityTimeTestMain, "TaskPriorityTimeTest", TIME_TEST_STACK_SIZE * 2,
                  NULL, TIME_TEST_PRIORITY - 1, NULL);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建任务优先级修改时间测试主任务失败\n");
  } else {
    xprintf("[INFO] 任务优先级修改时间测试启动成功\n");
  }
}
