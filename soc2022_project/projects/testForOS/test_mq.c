#include "test_mq.h"
#include "xprintf.h"

/* 测试配置 */
#define MQ_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define MQ_TEST_PRIORITY (tskIDLE_PRIORITY + 1)
#define MQ_MSG_SIZE (sizeof(uint32_t))
#define MQ_QUEUE_LENGTH (5)

/* 消息队列句柄 */
static QueueHandle_t xTestQueue = NULL;

/* 异步测试任务句柄 */
static TaskHandle_t xAsyncReceiverHandle = NULL;

/* ========== 消息队列同步通信测试 ========== */

static void vMqSyncSenderTask(void *pvParameters) {
  uint32_t ulMessage = 0;

  xprintf("[消息队列同步测试] 发送任务启动\n");

  for (uint32_t i = 1; i <= 3; i++) {
    ulMessage = i;

    xprintf("[消息队列同步测试] 发送任务: 准备发送消息 #%d\n", i);

    if (xQueueSend(xTestQueue, &ulMessage, portMAX_DELAY) == pdPASS) {
      xprintf("[消息队列同步测试] 发送任务: 发送消息 #%d 成功\n", i);
    } else {
      xprintf("[消息队列同步测试] 发送任务: 发送失败 #%d\n", i);
    }

    // 发送间隔，给接收任务时间处理消息
    vTaskDelay(pdMS_TO_TICKS(2000));
  }

  xprintf("[消息队列同步测试] 发送任务完成\n");
  vTaskDelete(NULL);
}

static void vMqSyncReceiverTask(void *pvParameters) {
  uint32_t ulReceivedMessage = 0;
  uint32_t ulReceivedCount = 0;

  xprintf("[消息队列同步测试] 接收任务启动（阻塞模式）\n");
  xprintf("[消息队列同步测试] 接收任务: 开始等待消息（阻塞状态）...\n");

  // 等待发送任务启动并发送第一条消息
  vTaskDelay(pdMS_TO_TICKS(500));

  while (ulReceivedCount < 3) {
    // 在阻塞等待前打印一次状态
    if (ulReceivedCount == 0) {
      xprintf("[消息队列同步测试] 接收任务: 正在阻塞等待第1条消息...\n");
    } else {
      xprintf("[消息队列同步测试] 接收任务: 继续等待第%d条消息...\n",
              ulReceivedCount + 1);
    }

    if (xQueueReceive(xTestQueue, &ulReceivedMessage, portMAX_DELAY) ==
        pdPASS) {
      ulReceivedCount++;
      xprintf("[消息队列同步测试] 接收任务: 收到消息 #%d "
              "(值=%d)，从阻塞状态恢复！\n",
              ulReceivedCount, ulReceivedMessage);
    }
  }

  xprintf("[消息队列同步测试] 接收任务: 收到所有消息，测试完成\n");
  vTaskDelete(NULL);
}

void vStartMqSyncTest(void) {
  BaseType_t xResult;

  xprintf("\n========== 消息队列同步通信测试 ==========\n");
  xprintf("[INFO] 创建消息队列用于同步通信测试\n");

  /* 创建消息队列 */
  xTestQueue = xQueueCreate(MQ_QUEUE_LENGTH, MQ_MSG_SIZE);
  if (xTestQueue == NULL) {
    xprintf("[ERROR] 创建消息队列失败\n");
    return;
  }

  xprintf("[INFO] 消息队列创建成功，长度=%d，消息大小=%d字节\n",
          MQ_QUEUE_LENGTH, MQ_MSG_SIZE);

  /* 先创建发送任务（高优先级，先运行） */
  xResult = xTaskCreate(vMqSyncSenderTask, "MqSyncSender", MQ_TEST_STACK_SIZE,
                        NULL, MQ_TEST_PRIORITY + 2, NULL);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建发送任务失败\n");
    vQueueDelete(xTestQueue);
    return;
  }

  /* 再创建接收任务（低优先级，后运行） */
  xResult = xTaskCreate(vMqSyncReceiverTask, "MqSyncReceiver",
                        MQ_TEST_STACK_SIZE, NULL, MQ_TEST_PRIORITY + 1, NULL);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建接收任务失败\n");
    vQueueDelete(xTestQueue);
    return;
  }

  xprintf("[INFO] 同步通信测试任务创建完成\n");
  xprintf("[INFO] 发送任务将先启动并发送消息\n");
  xprintf("[INFO] 接收任务稍后启动并接收消息\n");
  xprintf("[INFO] 测试将运行约7秒...\n\n");
}

/* ========== 消息队列异步通信测试 ========== */

static void vMqAsyncSenderTask(void *pvParameters) {
  uint32_t ulMessage = 0;
  uint32_t ulBatchCount = 0;

  xprintf("[消息队列异步测试] 发送任务启动\n");

  for (uint32_t i = 1; i <= 2; i++) {
    xprintf("[消息队列异步测试] 发送任务: 准备发送第%d批消息（2条）\n", i);

    // 一次发送两个消息
    ulBatchCount = 0;
    for (uint32_t j = 0; j < 2; j++) {
      ulMessage = i * 10 + j + 1; // 消息值：11,12, 21,22, 31,32

      xprintf("[消息队列异步测试] 发送任务: 发送消息 #%d (值=%d)\n",
              (i - 1) * 2 + j + 1, ulMessage);

      if (xQueueSend(xTestQueue, &ulMessage, 0) == pdPASS) {
        ulBatchCount++;
        xprintf("[消息队列异步测试] 发送任务: 发送消息 #%d 成功\n",
                (i - 1) * 2 + j + 1);
      } else {
        xprintf("[消息队列异步测试] 发送任务: 发送消息 #%d 失败\n",
                (i - 1) * 2 + j + 1);
      }
    }

    xprintf("[消息队列异步测试] 发送任务: 第%d批发送完成，成功发送%d条消息\n",
            i, ulBatchCount);

    // 唤醒接收任务
    if (xAsyncReceiverHandle != NULL) {
      xprintf("[消息队列异步测试] 发送任务: 唤醒接收任务\n");
      vTaskResume(xAsyncReceiverHandle);
    }

    // 发送间隔
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  xprintf("[消息队列异步测试] 发送任务完成\n");
  vTaskResume(xAsyncReceiverHandle);
  vTaskDelete(NULL);
}

static void vMqAsyncReceiverTask(void *pvParameters) {
  uint32_t ulReceivedMessage = 0;
  uint32_t ulReceivedCount = 0;
  uint32_t ulNoMessageCount = 0;
  uint32_t ulLoopCount = 0;

  xprintf("[消息队列异步测试] 接收任务启动（非阻塞模式）\n");

  while (ulLoopCount < 3) {
    ulLoopCount++;
    xprintf("[消息队列异步测试] 接收任务: 第%d次检查消息...\n", ulLoopCount);

    uint32_t ulBatchCount = 0;
    while (xQueueReceive(xTestQueue, &ulReceivedMessage, 0) == pdPASS) {
      ulReceivedCount++;
      ulBatchCount++;
      xprintf("[消息队列异步测试] 接收任务: 收到消息 #%d (值=%d)\n",
              ulReceivedCount, ulReceivedMessage);
    }

    if (ulBatchCount > 0) {
      xprintf("[消息队列异步测试] 接收任务: 本次批量接收了%d条消息\n",
              ulBatchCount);
    } else {
      ulNoMessageCount++;
      xprintf("[消息队列异步测试] 接收任务: 队列为空，无消息可接收\n");
    }

    xprintf("[消息队列异步测试] 接收任务: 主动挂起，让发送任务运行\n");
    vTaskSuspend(NULL);

    xprintf("[消息队列异步测试] 接收任务: 被唤醒，继续检查\n");
  }

  xprintf("[消息队列异步测试] 接收任务: 测试完成\n");
  xprintf("[消息队列异步测试] 统计: 收到%d条消息，无消息%d次，总循环%d次\n",
          ulReceivedCount, ulNoMessageCount, ulLoopCount);
  vTaskDelete(NULL);
}

void vStartMqAsyncTest(void) {
  BaseType_t xResult;

  xprintf("\n========== 消息队列异步通信测试 ==========\n");
  xprintf("[INFO] 创建消息队列用于异步通信测试\n");

  /* 创建消息队列 */
  xTestQueue = xQueueCreate(MQ_QUEUE_LENGTH, MQ_MSG_SIZE);
  if (xTestQueue == NULL) {
    xprintf("[ERROR] 创建消息队列失败\n");
    return;
  }

  xprintf("[INFO] 消息队列创建成功，长度=%d，消息大小=%d字节\n",
          MQ_QUEUE_LENGTH, MQ_MSG_SIZE);

  xResult =
      xTaskCreate(vMqAsyncReceiverTask, "MqAsyncReceiver", MQ_TEST_STACK_SIZE,
                  NULL, MQ_TEST_PRIORITY + 2, &xAsyncReceiverHandle);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建接收任务失败\n");
    vQueueDelete(xTestQueue);
    return;
  }

  xResult = xTaskCreate(vMqAsyncSenderTask, "MqAsyncSender", MQ_TEST_STACK_SIZE,
                        NULL, MQ_TEST_PRIORITY + 1, NULL);
  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建发送任务失败\n");
    vQueueDelete(xTestQueue);
    return;
  }

  xprintf("[INFO] 异步通信测试任务创建完成\n");
  xprintf("[INFO] 发送任务将先启动并发送消息\n");
  xprintf("[INFO] 接收任务稍后启动并接收消息（非阻塞模式）\n");
  xprintf("[INFO] 测试将运行约6秒...\n\n");
}

/* ========== 消息队列创建和删除测试 ========== */

static void vMqCreateDeleteTestMain(void *pvParameters) {
  QueueHandle_t xTestMq = NULL;
  BaseType_t xResult;
  uint32_t ulPassCount = 0;
  uint32_t ulFailCount = 0;

  xprintf("\n========== 消息队列创建和删除测试 ==========\n");

  /* 用例1: 创建消息队列 - 有效参数 */
  xprintf("\n[用例1] 创建消息队列（有效参数）\n");
  xTestMq = xQueueCreate(MQ_QUEUE_LENGTH, MQ_MSG_SIZE);
  if (xTestMq != NULL) {
    xprintf("[消息队列创建测试] ✅ 创建成功，句柄=0x%08X\n", (uint32_t)xTestMq);
    xprintf("[消息队列创建测试] 队列长度=%d，消息大小=%d字节\n",
            MQ_QUEUE_LENGTH, MQ_MSG_SIZE);
    ulPassCount++;

    /* 用例2: 删除消息队列 - 有效ID */
    xprintf("\n[用例2] 删除消息队列（有效ID）\n");
    vQueueDelete(xTestMq);
    xprintf("[消息队列删除测试] ✅ 删除成功\n");
    ulPassCount++;
  } else {
    xprintf("[消息队列创建测试] ❌ 创建失败（不应失败）\n");
    ulFailCount++;
  }

  vTaskDelete(NULL);
}

void vStartMqCreateDeleteTest(void) {
  BaseType_t xResult;

  xprintf("\n[INFO] 启动消息队列创建和删除测试...\n");

  xResult = xTaskCreate(vMqCreateDeleteTestMain, "MqCreateDeleteTest",
                        MQ_TEST_STACK_SIZE * 2, NULL, MQ_TEST_PRIORITY, NULL);

  if (xResult != pdPASS) {
    xprintf("[ERROR] 创建消息队列创建删除测试任务失败\n");
  } else {
    xprintf("[INFO] 消息队列创建删除测试启动成功\n");
  }
}