#include "FreeRTOS.h"
#include "timers.h"
#include "utils/u_time.h"
#include "xprintf.h"

#define ALARM_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define ALARM_TEST_PRIORITY (tskIDLE_PRIORITY + 2)

static TimerHandle_t xTestTimer = NULL;

//  在任务没有Alarm时，start一个alarm需要的时间
static void xTimerStartWithTime(void) {
  xTestTimer = xTimerCreate("TestAlarm", pdMS_TO_TICKS(500), pdFALSE, NULL, NULL);
  if (xTestTimer == NULL) {
    xprintf("[ERROR] 创建Alarm定时器失败\n");
    return;
  }
  for (int i = 0; i < 5; i++) {
    uint64_t cycle_start = get_cpu_cycle();
    BaseType_t result = xTimerStart(xTestTimer, 0);
    uint64_t cycle_end = get_cpu_cycle();
    xprintf("[Alarm时间测试] %d: 启动Alarm定时器耗时: %lu cycles\n", i + 1,
            (uint32_t)(cycle_end - cycle_start));
    if (result == pdPASS) {
      xTimerStop(xTestTimer, 0);
    }
  }
  xTimerDelete(xTestTimer, 0);
  xTestTimer = NULL;
}

static void vAlarmStartTimeTestMain(void *pvParameters) {
  xprintf("\n[INFO] 开始 FreeRTOS Alarm定时器启动时间测试...\n\n");
  xprintf("========== 启动Alarm定时器时间 ==========\n");
  xTimerStartWithTime();
  xprintf("\n[Alarm时间测试] 测试任务完成\n");
  vTaskDelete(NULL);
}

void vStartAlarmStartTimeTest(void) {
  BaseType_t xResult;
  xprintf("\n[INFO] 启动 FreeRTOS Alarm定时器启动时间测试...\n");
  xResult =
      xTaskCreate(vAlarmStartTimeTestMain, "AlarmStartTimeTest",
                  ALARM_TEST_STACK_SIZE * 2, NULL, ALARM_TEST_PRIORITY, NULL);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建Alarm定时器启动时间测试主任务失败\n");
  } else {
    xprintf("[INFO] Alarm定时器启动时间测试启动成功\n");
  }
}
