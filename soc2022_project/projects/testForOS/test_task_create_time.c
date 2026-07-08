#include "test_task_create_time.h"
#include "utils/u_time.h"
#include "xprintf.h"

#define TIME_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define TIME_TEST_PRIORITY (tskIDLE_PRIORITY + 2)

static TaskHandle_t xTimeTestTaskHandle = NULL;

// 记录创建并激活一个任务所需的时间
static void xTaskCreateWithTime(TaskFunction_t pvTaskCode,
                                const char *const pcName,
                                const configSTACK_DEPTH_TYPE uxStackDepth,
                                void *pvParameters, UBaseType_t uxPriority,
                                TaskHandle_t *pxCreatedTask) {
  for (int i = 0; i < 3; i++) {
    uint64_t cycle_start = get_cpu_cycle();
    BaseType_t result = xTaskCreate(pvTaskCode, pcName, uxStackDepth,
                                    pvParameters, uxPriority, pxCreatedTask);
    uint64_t cycle_end = get_cpu_cycle();
    xprintf("[任务时间测试] %d: 创建任务耗时: %lu cycles\n", i + 1,
            (uint32_t)(cycle_end - cycle_start));
  }
}

static void vTimeTestTask(void *pvParameters) {
  for (int i = 0; i < 3; i++) {
    vTaskDelay(pdMS_TO_TICKS(200));
  }
  vTaskDelete(NULL);
}

static void vTaskCreateTimeTestMain(void *pvParameters) {
  TaskHandle_t xTestHandle = NULL;
  xprintf("\n[INFO] 开始 FreeRTOS 任务创建时间测试...\n\n");
  xprintf("========== 创建任务时间 ==========\n");
  xTaskCreateWithTime(vTimeTestTask, "TimeTestTask", TIME_TEST_STACK_SIZE, NULL,
                      TIME_TEST_PRIORITY, &xTestHandle);
  xprintf("\n[任务时间测试] 测试任务完成\n");
  vTaskDelete(NULL);
}

void vStartTaskCreateTimeTest(void) {
  BaseType_t xResult;
  xprintf("\n[INFO] 启动 FreeRTOS 任务创建时间测试...\n");
  xResult =
      xTaskCreate(vTaskCreateTimeTestMain, "TaskCreateTimeTest", TIME_TEST_STACK_SIZE * 2,
                  NULL, TIME_TEST_PRIORITY - 1, NULL);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建任务创建时间测试主任务失败\n");
  } else {
    xprintf("[INFO] 任务创建时间测试启动成功\n");
  }
}
