#include "test_mutex_request_time.h"
#include "XYR3201.h"
#include "xprintf.h"

// 全局变量
static MUTEX_ID g_mutex = NULL;
static TaskHandle_t g_highTaskHandle = NULL;
static TaskHandle_t g_lowTaskHandle = NULL;

// CPU周期计数函数
static uint32_t get_cpu_cycles(void) {
  uint32_t cycles_low, cycles_high;
  __asm__ volatile("rdcycle %0" : "=r"(cycles_low));
  __asm__ volatile("rdcycleh %0" : "=r"(cycles_high));
  return cycles_low;
}

// 高优先级任务A
static void vHighPriorityTaskA(void *pvParameters) {
  uint32_t request_start_cycles, request_end_cycles;
  uint32_t request_time;

  xprintf("========== 5.2.10 互斥量请求操作耗时测试 ==========\r\n");
  xprintf("测试说明: 测量互斥量不可用时请求操作的CPU开销\r\n");
  xprintf("高优先级任务A启动\r\n");

  // 等待500ms确保低优先级任务先获取互斥量
  vTaskDelay(pdMS_TO_TICKS(500));

  xprintf("高优先级任务A尝试获取互斥量（将被阻塞）\r\n");

  // 记录请求开始时间
  request_start_cycles = get_cpu_cycles();

  // 尝试获取互斥量（会阻塞）
  if (xSemaphoreTake(g_mutex, portMAX_DELAY) == pdTRUE) {
    // 记录请求结束时间（实际是获取成功的时间）
    request_end_cycles = get_cpu_cycles();
    request_time = request_end_cycles - request_start_cycles;

    xprintf("高优先级任务A获取互斥量成功\r\n");
    xprintf("请求操作耗时: %lu CPU cycles\r\n", request_time);

    // 在临界区工作
    xprintf("高优先级任务A在临界区中工作\r\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 释放互斥量
    xprintf("高优先级任务A释放互斥量\r\n");
    xSemaphoreGive(g_mutex);
  }

  xprintf("高优先级任务A完成\r\n");
  xprintf("========== 测试完成 ==========\r\n\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 低优先级任务B
static void vLowPriorityTaskB(void *pvParameters) {
  xprintf("低优先级任务B启动\r\n");

  // 获取互斥量
  if (xSemaphoreTake(g_mutex, portMAX_DELAY) == pdTRUE) {
    xprintf("低优先级任务B获取互斥量\r\n");
    xprintf("低优先级任务B在临界区中工作（忙等待）\r\n");

    // 忙等待一段时间，让高优先级任务尝试获取

    xprintf("低优先级任务B释放互斥量\r\n");
    xSemaphoreGive(g_mutex);
  }

  xprintf("低优先级任务B完成\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 启动测试
void vStartMutexRequestTimeTest(void) {
  // 创建互斥量
  g_mutex = xSemaphoreCreateMutex();
  if (g_mutex == NULL) {
    xprintf("错误: 无法创建互斥量\r\n");
    return;
  }

  // 创建高优先级任务A (优先级1)
  if (xTaskCreate(vHighPriorityTaskA, "HighTaskA", 256, NULL, 3,
                  &g_highTaskHandle) != pdPASS) {
    xprintf("错误: 无法创建高优先级任务A\r\n");
    return;
  }

  // 创建低优先级任务B (优先级2)
  if (xTaskCreate(vLowPriorityTaskB, "LowTaskB", 256, NULL, 2,
                  &g_lowTaskHandle) != pdPASS) {
    xprintf("错误: 无法创建低优先级任务B\r\n");
    return;
  }

  xprintf("互斥量请求操作耗时测试任务创建成功\r\n");
}
