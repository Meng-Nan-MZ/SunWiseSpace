#include "test_mq_send.h"
#include "xprintf.h"

/* 测试配置 */
#define MQ_SEND_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define MQ_SEND_TEST_PRIORITY (tskIDLE_PRIORITY + 2)
#define MQ_SEND_MSG_SIZE (sizeof(uint32_t))
#define MQ_SEND_QUEUE_LENGTH (3) /* 小队列长度便于测试阻塞 */

/* 消息队列句柄 */
static QueueHandle_t xSendTestQueue = NULL;

/* 测试统计 */
static uint32_t ulBlockingSendCount = 0;
static uint32_t ulNonBlockingSendCount = 0;
static uint32_t ulReceiveCount = 0;

/* 任务句柄 */
static TaskHandle_t xBlockingSenderHandle = NULL;
static TaskHandle_t xNonBlockingSenderHandle = NULL;
static TaskHandle_t xReceiverHandle = NULL;
static TaskHandle_t xTestMainHandle = NULL;

/* 阻塞发送任务 */
static void vBlockingSenderTask(void *pvParameters) {
  uint32_t ulMessage = 0;
  BaseType_t xResult;
  uint32_t ulSendAttempts = 0;

  xprintf("[阻塞发送测试] 任务启动，将尝试发送10条消息\n");

  for (uint32_t i = 1; i <= 10; i++) {
    ulMessage = i;
    ulSendAttempts++;

    xprintf("[阻塞发送测试] 尝试发送消息 #%d (队列长度: %d)\n", i,
            (int)uxQueueMessagesWaiting(xSendTestQueue));

    /* 阻塞发送 - WAIT_FOREVER */
    xResult = xQueueSend(xSendTestQueue, &ulMessage, portMAX_DELAY);

    if (xResult == pdPASS) {
      ulBlockingSendCount++;
      xprintf("[阻塞发送测试] ✅ 消息 #%d 发送成功 (总发送: %d)\n", i,
              (int)ulBlockingSendCount);
    } else {
      xprintf("[阻塞发送测试] ❌ 消息 #%d 发送失败\n", i);
    }

    /* 短暂延迟，让接收任务有机会处理 */
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  xprintf("[阻塞发送测试] 任务完成，共尝试发送 %d 次，成功 %d 次\n",
          (int)ulSendAttempts, (int)ulBlockingSendCount);

  vTaskDelete(NULL);
}

/* ========== 非阻塞方式发送消息队列测试 ========== */

/* 非阻塞发送任务 */
static void vNonBlockingSenderTask(void *pvParameters) {
  uint32_t ulMessage = 0;
  BaseType_t xResult;
  uint32_t ulSendAttempts = 0;
  uint32_t ulSuccessCount = 0;
  uint32_t ulFailCount = 0;

  xprintf("[非阻塞发送测试] 任务启动，将尝试发送10条消息\n");

  for (uint32_t i = 1; i <= 10; i++) {
    ulMessage = i + 100; /* 使用不同的消息ID区分 */
    ulSendAttempts++;

    xprintf("[非阻塞发送测试] 尝试发送消息 #%d (队列长度: %d)\n", i,
            (int)uxQueueMessagesWaiting(xSendTestQueue));

    /* 非阻塞发送 - NO_WAIT */
    xResult = xQueueSend(xSendTestQueue, &ulMessage, 0);

    if (xResult == pdPASS) {
      ulSuccessCount++;
      ulNonBlockingSendCount++;
      xprintf("[非阻塞发送测试] ✅ 消息 #%d 发送成功 (总成功: %d)\n", i,
              (int)ulSuccessCount);
    } else {
      ulFailCount++;
      xprintf("[非阻塞发送测试] ❌ 消息 #%d 发送失败 - 队列满 (总失败: %d)\n",
              i, (int)ulFailCount);
    }

    /* 短暂延迟 */
    vTaskDelay(pdMS_TO_TICKS(50));
  }

  xprintf("[非阻塞发送测试] 任务完成，尝试 %d 次，成功 %d 次，失败 %d 次\n",
          (int)ulSendAttempts, (int)ulSuccessCount, (int)ulFailCount);

  vTaskDelete(NULL);
}

/* 接收任务 - 用于处理消息队列中的消息 */
static void vReceiverTask(void *pvParameters) {
  uint32_t ulMessage = 0;
  BaseType_t xResult;

  xprintf("[接收测试] 接收任务启动\n");

  while (1) {
    /* 接收消息，阻塞等待 */
    xResult = xQueueReceive(xSendTestQueue, &ulMessage, portMAX_DELAY);

    if (xResult == pdPASS) {
      ulReceiveCount++;
      xprintf("[接收测试] 接收到消息: %d (总接收: %d)\n", (int)ulMessage,
              (int)ulReceiveCount);

      /* 模拟消息处理时间 */
      vTaskDelay(pdMS_TO_TICKS(50));
    } else {
      xprintf("[接收测试] 接收消息失败\n");
    }
  }
}

/* 阻塞发送测试主函数 */
static void vBlockingSendTestMain(void *pvParameters) {
  xprintf("\n========== 阻塞方式发送消息队列测试 ==========\n");
  xprintf("[阻塞发送测试] 创建消息队列，长度: %d\n", MQ_SEND_QUEUE_LENGTH);

  /* 创建消息队列 */
  xSendTestQueue = xQueueCreate(MQ_SEND_QUEUE_LENGTH, MQ_SEND_MSG_SIZE);
  if (xSendTestQueue == NULL) {
    xprintf("[阻塞发送测试] ❌ 创建消息队列失败\n");
    vTaskDelete(NULL);
    return;
  }

  xprintf("[阻塞发送测试] ✅ 消息队列创建成功\n");

  /* 创建阻塞发送任务 */
  if (xTaskCreate(vBlockingSenderTask, "mqBlockSender", MQ_SEND_TEST_STACK_SIZE,
                  NULL, MQ_SEND_TEST_PRIORITY + 1,
                  &xBlockingSenderHandle) != pdPASS) {
    xprintf("[阻塞发送测试] ❌ 创建阻塞发送任务失败\n");
    vTaskDelete(xReceiverHandle);
    vQueueDelete(xSendTestQueue);
    vTaskDelete(NULL);
    return;
  }

  xprintf("[阻塞发送测试] 测试开始，观察阻塞行为...\n");

  vTaskDelay(pdMS_TO_TICKS(5000)); /* 等待接收任务启动 */
  /* 创建接收任务 */
  if (xTaskCreate(vReceiverTask, "mqReceiver", MQ_SEND_TEST_STACK_SIZE, NULL,
                  MQ_SEND_TEST_PRIORITY, &xReceiverHandle) != pdPASS) {
    xprintf("[阻塞发送测试] ❌ 创建接收任务失败\n");
    vQueueDelete(xSendTestQueue);
    vTaskDelete(NULL);
    return;
  }

  /* 等待测试完成 */
  vTaskDelay(pdMS_TO_TICKS(2000));

  /* 清理资源 */
  if (xReceiverHandle)
    vTaskDelete(xReceiverHandle);
  if (xBlockingSenderHandle)
    vTaskDelete(xBlockingSenderHandle);
  if (xSendTestQueue)
    vQueueDelete(xSendTestQueue);

  xprintf("[阻塞发送测试] 测试完成\n");
  vTaskDelete(NULL);
}

/* 非阻塞发送测试主函数 */
static void vNonBlockingSendTestMain(void *pvParameters) {
  xprintf("\n========== 非阻塞方式发送消息队列测试 ==========\n");
  xprintf("[非阻塞发送测试] 创建消息队列，长度: %d\n", MQ_SEND_QUEUE_LENGTH);

  /* 创建消息队列 */
  xSendTestQueue = xQueueCreate(MQ_SEND_QUEUE_LENGTH, MQ_SEND_MSG_SIZE);
  if (xSendTestQueue == NULL) {
    xprintf("[非阻塞发送测试] ❌ 创建消息队列失败\n");
    vTaskDelete(NULL);
    return;
  }

  xprintf("[非阻塞发送测试] ✅ 消息队列创建成功\n");

  xprintf("[非阻塞发送测试] 测试开始，观察非阻塞行为...\n");
  /* 创建非阻塞发送任务 */
  if (xTaskCreate(vNonBlockingSenderTask, "mqNonBlockSender",
                  MQ_SEND_TEST_STACK_SIZE, NULL, MQ_SEND_TEST_PRIORITY + 1,
                  &xNonBlockingSenderHandle) != pdPASS) {
    xprintf("[非阻塞发送测试] ❌ 创建非阻塞发送任务失败\n");
    vTaskDelete(xReceiverHandle);
    vQueueDelete(xSendTestQueue);
    vTaskDelete(NULL);
    return;
  }

  vTaskDelay(pdMS_TO_TICKS(500)); /* 等待接收任务启动 */
  /* 创建接收任务 */
  if (xTaskCreate(vReceiverTask, "mqReceiver2", MQ_SEND_TEST_STACK_SIZE, NULL,
                  MQ_SEND_TEST_PRIORITY, &xReceiverHandle) != pdPASS) {
    xprintf("[非阻塞发送测试] ❌ 创建接收任务失败\n");
    vQueueDelete(xSendTestQueue);
    vTaskDelete(NULL);
    return;
  }

  /* 等待测试完成 */
  vTaskDelay(pdMS_TO_TICKS(2000));

  /* 清理资源 */
  if (xReceiverHandle)
    vTaskDelete(xReceiverHandle);
  if (xNonBlockingSenderHandle)
    vTaskDelete(xNonBlockingSenderHandle);
  if (xSendTestQueue)
    vQueueDelete(xSendTestQueue);

  xprintf("[非阻塞发送测试] 测试完成\n");
  vTaskDelete(NULL);
}

/* 综合测试主函数 */
static void vMqSendTestMain(void *pvParameters) {
  xprintf("\n========== 消息队列发送功能综合测试 ==========\n");

  /* 先测试阻塞发送 */
  vBlockingSendTestMain(NULL);

  vTaskDelay(pdMS_TO_TICKS(2000));

  /* 重置统计计数 */
  ulBlockingSendCount = 0;
  ulNonBlockingSendCount = 0;
  ulReceiveCount = 0;

  /* 再测试非阻塞发送 */
  vNonBlockingSendTestMain(NULL);

  xprintf("\n========== 消息队列发送功能测试完成 ==========\n");
  vTaskDelete(NULL);
}

/* ========== 公共接口函数 ========== */

/* 启动阻塞发送测试 */
void vStartMqBlockingSendTest(void) {
  if (xTaskCreate(vBlockingSendTestMain, "mqBlockTest",
                  MQ_SEND_TEST_STACK_SIZE * 2, NULL, MQ_SEND_TEST_PRIORITY,
                  &xTestMainHandle) != pdPASS) {
    xprintf("[阻塞发送测试] ❌ 创建测试主任务失败\n");
  } else {
    xprintf("[阻塞发送测试] ✅ 测试任务已启动\n");
  }
}

/* 启动非阻塞发送测试 */
void vStartMqNonBlockingSendTest(void) {
  if (xTaskCreate(vNonBlockingSendTestMain, "mqNonBlockTest",
                  MQ_SEND_TEST_STACK_SIZE * 2, NULL, MQ_SEND_TEST_PRIORITY,
                  &xTestMainHandle) != pdPASS) {
    xprintf("[非阻塞发送测试] ❌ 创建测试主任务失败\n");
  } else {
    xprintf("[非阻塞发送测试] ✅ 测试任务已启动\n");
  }
}

/* 启动综合发送测试 */
void vStartMqSendTest(void) {
  if (xTaskCreate(vMqSendTestMain, "mqSendTest", MQ_SEND_TEST_STACK_SIZE * 2,
                  NULL, MQ_SEND_TEST_PRIORITY, &xTestMainHandle) != pdPASS) {
    xprintf("[消息队列发送测试] ❌ 创建测试主任务失败\n");
  } else {
    xprintf("[消息队列发送测试] ✅ 综合测试任务已启动\n");
  }
}
