#include "test_mq_create_time.h"
#include "semphr.h"
#include "utils/u_time.h"
#include "xprintf.h"

#define MQ_LENGTH (10)
#define MQ_MSG_SIZE (1)

// 记录创建一个消息队列所需的时间
static QueueHandle_t OSMqCreateWithTime(uint32_t mqleng, uint32_t msgsize) {
  QueueHandle_t mq = NULL;
  for (int i = 0; i < 5; i++) {
    uint64_t cycle_start = get_cpu_cycle();
    mq = xQueueCreate(mqleng, msgsize);
    uint64_t cycle_end = get_cpu_cycle();
    xprintf("[消息队列时间测试] %d: 创建队列耗时: %lu cycles\n", i + 1,
            (uint32_t)(cycle_end - cycle_start));
    if (mq != NULL) {
      vQueueDelete(mq);
    }
  }
  // 最后再创建一个用于后续测试
  mq = xQueueCreate(mqleng, msgsize);
  return mq;
}

static void vMQCreateTimeTestMain(void *pvParameters) {
  xprintf("\n[INFO] 开始 FreeRTOS 消息队列创建时间测试...\n\n");
  xprintf("========== 创建队列时间 ==========\n");
  OSMqCreateWithTime(MQ_LENGTH, MQ_MSG_SIZE);
  xprintf("\n[消息队列时间测试] 测试任务完成\n");
  vTaskDelete(NULL);
}

void vStartMQCreateTimeTest(void) {
  BaseType_t xResult = pdFALSE;
  xprintf("\n[INFO] 启动 FreeRTOS 消息队列创建时间测试...\n");
  xResult =
      xTaskCreate(vMQCreateTimeTestMain, "MQCreateTimeTest", configMINIMAL_STACK_SIZE * 2,
                  NULL, tskIDLE_PRIORITY + 1, NULL);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建消息队列创建时间测试主任务失败\n");
  } else {
    xprintf("[INFO] 消息队列创建时间测试启动成功\n");
  }
}
