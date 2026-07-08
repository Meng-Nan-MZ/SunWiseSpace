#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "xprintf.h"

// 测试配置
#define SEM_TAKE_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define TEST_PRIORITY (tskIDLE_PRIORITY + 2)
#define TEST_COUNT 10

// 全局变量
static SemaphoreHandle_t g_semaphore = NULL;
static uint32_t g_t1 = 0;
static uint32_t g_t2 = 0;
static uint32_t g_delay_times[TEST_COUNT];
static uint32_t g_give_delay_times[TEST_COUNT];
static uint32_t g_test_count = 0;

// 获取CPU周期数（32位）
static inline uint32_t get_cpu_cycles(void) {
  uint32_t cycle;
  __asm__ volatile("rdcycle %0" : "=r"(cycle));
  return cycle;
}

// 测试任务
static void vSemTakeTestTask(void *pvParameters) {
  xprintf("信号量申请延迟时间测试任务启动\r\n");

  // 创建二进制信号量
  g_semaphore = xSemaphoreCreateBinary();
  if (g_semaphore == NULL) {
    xprintf("错误: 无法创建信号量\r\n");
    vTaskDelete(NULL);
    return;
  }

  xprintf("信号量创建成功，开始测试\r\n");
  xprintf("测试次数: %d\r\n", TEST_COUNT);

  // 先让信号量可用
  xSemaphoreGive(g_semaphore);

  // 进行多次测试
  for (int i = 0; i < TEST_COUNT; i++) {
    // 记录申请前的时间戳t1
    g_t1 = get_cpu_cycles();

    // 申请信号量（应该立即成功）
    if (xSemaphoreTake(g_semaphore, portMAX_DELAY) == pdTRUE) {
      // 记录申请后的时间戳t2
      g_t2 = get_cpu_cycles();

      // 计算时间差
      uint32_t delay_cycles = g_t2 - g_t1;
      g_delay_times[i] = delay_cycles;

      xprintf("第%d次申请信号量延迟: %lu CPU cycles\r\n", i + 1, delay_cycles);

      // 测试释放信号量延迟
      g_t1 = get_cpu_cycles();
      xSemaphoreGive(g_semaphore);
      g_t2 = get_cpu_cycles();

      uint32_t give_delay_cycles = g_t2 - g_t1;
      g_give_delay_times[i] = give_delay_cycles;
      xprintf("第%d次释放信号量延迟: %lu CPU cycles\r\n", i + 1,
              give_delay_cycles);

      // 短暂延迟
      vTaskDelay(pdMS_TO_TICKS(1));
    } else {
      xprintf("错误: 第%d次申请信号量失败\r\n", i + 1);
      break;
    }
  }

  // 计算申请信号量统计信息
  uint32_t total_cycles = 0;
  uint32_t max_cycles = 0;
  uint32_t min_cycles = 0xFFFFFFFF;

  for (int i = 0; i < TEST_COUNT; i++) {
    total_cycles += g_delay_times[i];
    if (g_delay_times[i] > max_cycles)
      max_cycles = g_delay_times[i];
    if (g_delay_times[i] < min_cycles)
      min_cycles = g_delay_times[i];
  }

  uint32_t avg_cycles = total_cycles / TEST_COUNT;

  xprintf("\r\n========== 申请信号量统计结果 ==========\r\n");
  xprintf("测试次数: %d\r\n", TEST_COUNT);
  xprintf("平均延迟: %lu CPU cycles\r\n", avg_cycles);
  xprintf("最大延迟: %lu CPU cycles\r\n", max_cycles);
  xprintf("最小延迟: %lu CPU cycles\r\n", min_cycles);
  xprintf("========================================\r\n");

  // 计算释放信号量统计信息
  uint32_t give_total_cycles = 0;
  uint32_t give_max_cycles = 0;
  uint32_t give_min_cycles = 0xFFFFFFFF;

  for (int i = 0; i < TEST_COUNT; i++) {
    give_total_cycles += g_give_delay_times[i];
    if (g_give_delay_times[i] > give_max_cycles)
      give_max_cycles = g_give_delay_times[i];
    if (g_give_delay_times[i] < give_min_cycles)
      give_min_cycles = g_give_delay_times[i];
  }

  uint32_t give_avg_cycles = give_total_cycles / TEST_COUNT;

  xprintf("\r\n========== 释放信号量统计结果 ==========\r\n");
  xprintf("测试次数: %d\r\n", TEST_COUNT);
  xprintf("平均延迟: %lu CPU cycles\r\n", give_avg_cycles);
  xprintf("最大延迟: %lu CPU cycles\r\n", give_max_cycles);
  xprintf("最小延迟: %lu CPU cycles\r\n", give_min_cycles);
  xprintf("========================================\r\n");

  // 清理资源
  vSemaphoreDelete(g_semaphore);

  xprintf("信号量申请和释放延迟时间测试完成\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 释放信号量延迟时间测试（独立测试）
void vStartSemGiveDelayTest(void) {
  xprintf("========== 释放信号量延迟时间测试 ==========\r\n");
  xprintf("测试说明: 测量任务成功释放信号量的时间\r\n");

  // 创建测试任务
  if (xTaskCreate(vSemTakeTestTask, "SemGiveTest", SEM_TAKE_STACK_SIZE, NULL,
                  TEST_PRIORITY, NULL) != pdPASS) {
    xprintf("错误: 无法创建测试任务\r\n");
    return;
  }

  xprintf("释放信号量延迟时间测试任务创建成功\r\n");
}

// 启动测试
void vStartSemTakeDelayTest(void) {
  xprintf("========== 申请信号量延迟时间测试 ==========\r\n");
  xprintf("测试说明: 测量任务成功获取信号量的时间\r\n");

  // 创建测试任务
  if (xTaskCreate(vSemTakeTestTask, "SemTakeTest", SEM_TAKE_STACK_SIZE, NULL,
                  TEST_PRIORITY, NULL) != pdPASS) {
    xprintf("错误: 无法创建测试任务\r\n");
    return;
  }

  xprintf("申请信号量延迟时间测试任务创建成功\r\n");
}
