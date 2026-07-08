#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"
#include "xprintf.h"

// 测试配置
#define MSG_RECV_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define HIGH_PRIORITY (tskIDLE_PRIORITY + 1)
#define LOW_PRIORITY (tskIDLE_PRIORITY + 2)
#define TEST_COUNT 10
#define MSG_BUFFER_SIZE 2000 // 消息缓冲区大小
#define MSG_DATA_SIZE 100    // 发送数据大小

// 全局变量
static MessageBufferHandle_t g_msgBuffer = NULL;
static TaskHandle_t g_lowTaskHandle = NULL;
static uint32_t g_t1 = 0;
static uint32_t g_t2 = 0;
static uint32_t g_delay_times[TEST_COUNT];
static uint8_t g_test_data[MSG_DATA_SIZE];
static uint8_t g_receive_buffer[MSG_DATA_SIZE];
static uint32_t g_test_count = 0;

// 获取CPU周期数（32位）
static inline uint32_t get_cpu_cycles(void) {
  uint32_t cycle;
  __asm__ volatile("rdcycle %0" : "=r"(cycle));
  return cycle;
}

// 初始化测试数据
static void init_test_data(void) {
  for (int i = 0; i < MSG_DATA_SIZE; i++) {
    g_test_data[i] = (uint8_t)(i & 0xFF);
  }
}

// 高优先级任务1（发送任务）
static void vHighPriorityTask1(void *pvParameters) {
  xprintf("高优先级任务1（发送）启动\r\n");

  // 等待低优先级任务2启动
  vTaskDelay(pdMS_TO_TICKS(10));

  xprintf("开始发送消息测试\r\n");

  // 进行多次测试
  for (int i = 0; i < TEST_COUNT; i++) {
    // 发送消息到缓冲区
    size_t bytes_sent =
        xMessageBufferSend(g_msgBuffer, g_test_data, MSG_DATA_SIZE,
                           0); // 非阻塞发送

    if (bytes_sent == MSG_DATA_SIZE) {
      xprintf("第%d次发送消息成功 (%d bytes)\r\n", i + 1, (int)bytes_sent);
    } else {
      xprintf("第%d次发送消息失败，只发送了 %d bytes\r\n", i + 1,
              (int)bytes_sent);
    }

    // 短暂延迟让低优先级任务接收
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  xprintf("高优先级任务1（发送）完成\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 低优先级任务2（接收任务）
static void vLowPriorityTask2(void *pvParameters) {
  xprintf("低优先级任务2（接收）启动\r\n");

  // 等待高优先级任务1发送消息
  vTaskDelay(pdMS_TO_TICKS(100));

  xprintf("开始接收消息测试\r\n");

  // 进行多次测试
  for (int i = 0; i < TEST_COUNT; i++) {
    // 记录接收前的时间戳t1
    g_t1 = get_cpu_cycles();

    // 接收消息
    size_t bytes_received = xMessageBufferReceive(g_msgBuffer, g_receive_buffer,
                                                  MSG_DATA_SIZE, portMAX_DELAY);

    // 记录接收后的时间戳t2
    g_t2 = get_cpu_cycles();

    if (bytes_received == MSG_DATA_SIZE) {
      // 计算时间差
      uint32_t delay_cycles = g_t2 - g_t1;
      g_delay_times[i] = delay_cycles;

      xprintf("第%d次接收消息延迟: %lu CPU cycles (接收了 %d bytes)\r\n", i + 1,
              delay_cycles, (int)bytes_received);
    } else {
      xprintf("错误: 第%d次接收消息失败，只接收了 %d bytes\r\n", i + 1,
              (int)bytes_received);
      g_delay_times[i] = 0;
    }

    // 短暂延迟
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  // 计算统计信息
  uint32_t total_cycles = 0;
  uint32_t max_cycles = 0;
  uint32_t min_cycles = 0xFFFFFFFF;
  uint32_t valid_tests = 0;

  for (int i = 0; i < TEST_COUNT; i++) {
    if (g_delay_times[i] > 0) // 只统计成功的测试
    {
      total_cycles += g_delay_times[i];
      if (g_delay_times[i] > max_cycles)
        max_cycles = g_delay_times[i];
      if (g_delay_times[i] < min_cycles)
        min_cycles = g_delay_times[i];
      valid_tests++;
    }
  }

  xprintf("低优先级任务2（接收）完成\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 启动测试
void vStartMsgRecvDelayTest(void) {
  xprintf("========== 接收消息延迟时间测试 ==========\r\n");
  xprintf("测试说明: 测量任务接收消息的时间\r\n");

  // 初始化测试数据
  init_test_data();

  // 创建消息缓冲区
  g_msgBuffer = xMessageBufferCreate(MSG_BUFFER_SIZE);
  if (g_msgBuffer == NULL) {
    xprintf("错误: 无法创建消息缓冲区\r\n");
    return;
  }

  xprintf("消息缓冲区创建成功\r\n");
  xprintf("测试次数: %d\r\n", TEST_COUNT);
  xprintf("消息大小: %d bytes\r\n", MSG_DATA_SIZE);
  xprintf("缓冲区大小: %d bytes\r\n", MSG_BUFFER_SIZE);

  // 先创建低优先级任务2（接收任务）
  if (xTaskCreate(vLowPriorityTask2, "LowTask2", MSG_RECV_STACK_SIZE, NULL,
                  LOW_PRIORITY, &g_lowTaskHandle) != pdPASS) {
    xprintf("错误: 无法创建低优先级任务2\r\n");
    vMessageBufferDelete(g_msgBuffer);
    return;
  }

  // 再创建高优先级任务1（发送任务）
  if (xTaskCreate(vHighPriorityTask1, "HighTask1", MSG_RECV_STACK_SIZE, NULL,
                  HIGH_PRIORITY, NULL) != pdPASS) {
    xprintf("错误: 无法创建高优先级任务1\r\n");
    vTaskDelete(g_lowTaskHandle);
    vMessageBufferDelete(g_msgBuffer);
    return;
  }

  xprintf("接收消息延迟时间测试任务创建成功\r\n");
}
