#include "FreeRTOS.h"
#include "timers.h"
#include "utils/u_time.h"
#include "xprintf.h"

#define ALARM_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define ALARM_TEST_PRIORITY (tskIDLE_PRIORITY + 2)

static TimerHandle_t xTestTimer = NULL;

//  创建一个Alarm定时器的时间
static void xTimerCreateWithTime(void) {
  for (int i = 0; i < 5; i++) {
    uint64_t cycle_start = get_cpu_cycle();
    xTestTimer =
        xTimerCreate("TestAlarm", pdMS_TO_TICKS(500), pdFALSE, NULL, NULL);
    uint64_t cycle_end = get_cpu_cycle();
    xprintf("[Alarm时间测试] %d: 创建Alarm定时器耗时: %lu cycles\n", i + 1,
            (uint32_t)(cycle_end - cycle_start));
    if (xTestTimer != NULL) {
      xTimerDelete(xTestTimer, 0);
      xTestTimer = NULL;
    }
  }
}

static void vAlarmCreateTimeTestMain(void *pvParameters) {
  xprintf("\n[INFO] 开始 FreeRTOS Alarm定时器创建时间测试...\n\n");
  xprintf("========== 创建Alarm定时器时间 ==========\n");
  xTimerCreateWithTime();
  xprintf("\n[Alarm时间测试] 测试任务完成\n");
  vTaskDelete(NULL);
}

void vStartAlarmCreateTimeTest(void) {
  BaseType_t xResult;
  xprintf("\n[INFO] 启动 FreeRTOS Alarm定时器创建时间测试...\n");
  xResult =
      xTaskCreate(vAlarmCreateTimeTestMain, "AlarmCreateTimeTest",
                  ALARM_TEST_STACK_SIZE * 2, NULL, ALARM_TEST_PRIORITY, NULL);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建Alarm定时器创建时间测试主任务失败\n");
  } else {
    xprintf("[INFO] Alarm定时器创建时间测试启动成功\n");
  }
}
