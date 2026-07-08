#include "test_mq_send_time.h"
#include "semphr.h"
#include "utils/u_time.h"
#include "xprintf.h"

#define MQ_LENGTH (10)
#define MQ_MSG_SIZE (1)

// 记录没有任务等待数据时发送1Byte数据所需的时间
static BaseType_t OSMqSendWithTime(QueueHandle_t mq, uint8_t *buffer,
                                   uint32_t nbyte, int32_t timeout,
                                   uint32_t prio) {
  for (int i = 0; i < 5; i++) {
    uint64_t cycle_start = get_cpu_cycle();
    BaseType_t result = xQueueSend(mq, buffer, timeout);
    uint64_t cycle_end = get_cpu_cycle();
    xprintf("[消息队列时间测试] %d: 发送数据耗时: %lu cycles\n", i + 1,
            (uint32_t)(cycle_end - cycle_start));
  }
  return pdPASS;
}

static void vMQSendTimeTestMain(void *pvParameters) {
  xprintf("\n[INFO] 开始 FreeRTOS 消息队列非阻塞发送时间测试...\n\n");
  xprintf("========== 非阻塞发送时间 ==========\n");
  QueueHandle_t mq = xQueueCreate(MQ_LENGTH, MQ_MSG_SIZE);
  uint8_t send_buf = 0xA5;
  if (mq != NULL) {
    OSMqSendWithTime(mq, &send_buf, 1, 0, 0);
    vQueueDelete(mq);
  }
  xprintf("\n[消息队列时间测试] 测试任务完成\n");
  vTaskDelete(NULL);
}

void vStartMQSendTimeTest(void) {
  BaseType_t xResult = pdFALSE;
  xprintf("\n[INFO] 启动 FreeRTOS 消息队列非阻塞发送时间测试...\n");
  xResult =
      xTaskCreate(vMQSendTimeTestMain, "MQSendTimeTest", configMINIMAL_STACK_SIZE * 2,
                  NULL, tskIDLE_PRIORITY + 1, NULL);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建消息队列非阻塞发送时间测试主任务失败\n");
  } else {
    xprintf("[INFO] 消息队列非阻塞发送时间测试启动成功\n");
  }
}
