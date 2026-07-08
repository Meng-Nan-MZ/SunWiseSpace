#include "test_tick.h"
#include "xprintf.h"

/* 测试配置 */
#define TICK_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define TICK_TEST_PRIORITY (tskIDLE_PRIORITY + 2)

/* 任务句柄 */
static TaskHandle_t xTickTestHandle = NULL;

/* ========== 节拍获取测试 ========== */

/* 节拍获取测试任务 */
static void vTickGetTestTask(void *pvParameters) {
  xprintf("\n========== 节拍获取测试 ==========\n");

  /* 连续获取节拍，观察变化 */
  xprintf("[节拍测试] 连续获取节拍（观察变化）:\n");
  for (int i = 0; i < 10; i++) {
    TickType_t xTick = xTaskGetTickCount();
    xprintf("  第%d次: %u\n", i + 1, (unsigned)xTick);
    vTaskDelay(pdMS_TO_TICKS(100)); /* 延时100ms */
  }

  xprintf("[节拍测试] 节拍获取测试完成\n");
  vTaskDelete(NULL);
}

/* ========== 节拍延时准确度测试 ========== */

/* 节拍延时准确度测试任务 */
static void vTickDelayTestTask(void *pvParameters) {
  xprintf("\n========== 节拍延时准确度测试 ==========\n");

  /* 测试不同节拍数的延时准确度 */
  uint32_t test_delays[] = {1, 5, 10, 50, 100, 500, 1000}; /* 毫秒 */
  int test_count = sizeof(test_delays) / sizeof(test_delays[0]);

  for (int i = 0; i < test_count; i++) {
    TickType_t xStartTick = xTaskGetTickCount();
    vTaskDelay(pdMS_TO_TICKS(test_delays[i]));
    TickType_t xEndTick = xTaskGetTickCount();
    TickType_t xActualDelay = xEndTick - xStartTick;

    /* 计算期望的节拍数 */
    TickType_t xExpectedTicks = pdMS_TO_TICKS(test_delays[i]);

    xprintf("  期望延时: %u ticks, 实际延时: %u ticks\n",
            (unsigned)xExpectedTicks, (unsigned)xActualDelay);
  }

  xprintf("[节拍测试] 节拍延时准确度测试完成\n");
  vTaskDelete(NULL);
}

/* ========== 简化测试主函数 ========== */

/* 简化节拍测试主函数 */
static void vTickTestMain(void *pvParameters) {
  /* 执行节拍获取测试 */
  vStartTickGetTest();
  vTaskDelay(pdMS_TO_TICKS(2000));

  /* 执行节拍延时准确度测试 */
  vStartTickDelayTest();

  vTaskDelete(NULL);
}

/* ========== 公共接口函数 ========== */

/* 启动节拍获取测试 */
void vStartTickGetTest(void) {
  if (xTaskCreate(vTickGetTestTask, "tickGetTest", TICK_TEST_STACK_SIZE, NULL,
                  TICK_TEST_PRIORITY, &xTickTestHandle) != pdPASS) {
    xprintf("[节拍测试] ❌ 创建节拍获取测试任务失败\n");
  }
}

/* 启动节拍延时准确度测试 */
void vStartTickDelayTest(void) {
  if (xTaskCreate(vTickDelayTestTask, "tickDelayTest", TICK_TEST_STACK_SIZE,
                  NULL, TICK_TEST_PRIORITY, &xTickTestHandle) != pdPASS) {
    xprintf("[节拍测试] ❌ 创建节拍延时准确度测试任务失败\n");
  }
}

/* 启动简化节拍测试 */
void vStartTickTest(void) {
  if (xTaskCreate(vTickTestMain, "tickTest", TICK_TEST_STACK_SIZE * 2, NULL,
                  TICK_TEST_PRIORITY, &xTickTestHandle) != pdPASS) {
    xprintf("[节拍测试] ❌ 创建节拍测试任务失败\n");
  }
}
