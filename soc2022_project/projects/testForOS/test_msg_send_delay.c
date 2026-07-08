#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"
#include "xprintf.h"

// 测试配置
#define MSG_SEND_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define TEST_PRIORITY (tskIDLE_PRIORITY + 2)
#define TEST_COUNT 10
#define MSG_BUFFER_SIZE 2000 // 消息缓冲区大小（增大10倍）
#define MSG_DATA_SIZE 100    // 发送数据大小

// 全局变量
static MessageBufferHandle_t g_msgBuffer = NULL;
static uint32_t g_t1 = 0;
static uint32_t g_t2 = 0;
static uint32_t g_delay_times[TEST_COUNT];
static uint8_t g_test_data[MSG_DATA_SIZE];

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

// 测试任务
static void vMsgSendTestTask(void *pvParameters) {
  xprintf("消息发送延迟时间测试任务启动\r\n");

  // 初始化测试数据
  init_test_data();

  // 创建消息缓冲区
  g_msgBuffer = xMessageBufferCreate(MSG_BUFFER_SIZE);
  if (g_msgBuffer == NULL) {
    xprintf("错误: 无法创建消息缓冲区\r\n");
    vTaskDelete(NULL);
    return;
  }

  xprintf("消息缓冲区创建成功，开始测试\r\n");
  xprintf("测试次数: %d\r\n", TEST_COUNT);
  xprintf("消息大小: %d bytes\r\n", MSG_DATA_SIZE);
  xprintf("缓冲区大小: %d bytes\r\n", MSG_BUFFER_SIZE);

  // 进行多次测试
  for (int i = 0; i < TEST_COUNT; i++) {
    // 记录发送前的时间戳t1
    g_t1 = get_cpu_cycles();

    // 发送消息到缓冲区（非阻塞）
    size_t bytes_sent =
        xMessageBufferSend(g_msgBuffer, g_test_data, MSG_DATA_SIZE,
                           0); // 非阻塞发送

    // 记录发送后的时间戳t2
    g_t2 = get_cpu_cycles();

    if (bytes_sent == MSG_DATA_SIZE) {
      // 计算时间差
      uint32_t delay_cycles = g_t2 - g_t1;
      g_delay_times[i] = delay_cycles;

      xprintf("第%d次发送消息延迟: %lu CPU cycles (发送了 %d bytes)\r\n", i + 1,
              delay_cycles, (int)bytes_sent);
    } else {
      xprintf(
          "警告: 第%d次发送消息部分成功，发送了 %d bytes (缓冲区可能满了)\r\n",
          i + 1, (int)bytes_sent);
      // 即使部分发送也记录时间
      uint32_t delay_cycles = g_t2 - g_t1;
      g_delay_times[i] = delay_cycles;
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

  if (valid_tests > 0) {
    uint32_t avg_cycles = total_cycles / valid_tests;

    xprintf("\r\n========== 统计结果 ==========\r\n");
    xprintf("成功测试次数: %d\r\n", valid_tests);
    xprintf("平均延迟: %lu CPU cycles\r\n", avg_cycles);
    xprintf("最大延迟: %lu CPU cycles\r\n", max_cycles);
    xprintf("最小延迟: %lu CPU cycles\r\n", min_cycles);
    xprintf("==============================\r\n");

    // 时间换算
    uint32_t avg_us = avg_cycles / (CPU_CLOCK_MHZ);
    uint32_t max_us = max_cycles / (CPU_CLOCK_MHZ);
    uint32_t min_us = min_cycles / (CPU_CLOCK_MHZ);

    xprintf("时间换算 (假设 %d MHz CPU):\r\n", CPU_CLOCK_MHZ);
    xprintf("平均延迟: %lu μs\r\n", avg_us);
    xprintf("最大延迟: %lu μs\r\n", max_us);
    xprintf("最小延迟: %lu μs\r\n", min_us);
    xprintf("==============================\r\n");
  } else {
    xprintf("错误: 没有成功的测试\r\n");
  }

  // 清理资源
  vMessageBufferDelete(g_msgBuffer);

  xprintf("消息发送延迟时间测试完成\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 启动测试
void vStartMsgSendDelayTest(void) {
  xprintf("========== 发送消息延迟时间测试 ==========\r\n");
  xprintf("测试说明: 测量任务发送消息到消息缓冲区的时间\r\n");

  // 创建测试任务
  if (xTaskCreate(vMsgSendTestTask, "MsgSendTest", MSG_SEND_STACK_SIZE, NULL,
                  TEST_PRIORITY, NULL) != pdPASS) {
    xprintf("错误: 无法创建测试任务\r\n");
    return;
  }

  xprintf("发送消息延迟时间测试任务创建成功\r\n");
}
