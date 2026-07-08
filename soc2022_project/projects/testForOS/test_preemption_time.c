#include "test_preemption_time.h"
#include "XYR3201.h"
#include "xprintf.h"

// 测试配置
#define PREEMPTION_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define HIGH_PRIORITY 3
#define LOW_PRIORITY 2

// 全局变量
static volatile uint32_t g_t1 = 0; // 任务2释放信号量前的时间戳
static volatile uint32_t g_t2 = 0; // 任务1获取信号量后的时间戳
static SemaphoreHandle_t g_semaphore = NULL;
static TaskHandle_t g_lowTaskHandle = NULL;

// CPU周期计数函数
static inline uint32_t get_cpu_cycles(void) {
  uint32_t cycles;
  __asm__ volatile("rdcycle %0" : "=r"(cycles));
  return cycles;
}

// 高优先级任务1
static void vHighPriorityTask1(void *pvParameters) {
  xprintf("高优先级任务1启动\r\n");
  // 尝试获取信号量（会被阻塞）
  xprintf("高优先级任务1尝试获取信号量（将被阻塞）\r\n");

  // 获取信号量（会阻塞直到低优先级任务释放）
  if (xSemaphoreTake(g_semaphore, portMAX_DELAY) == pdTRUE) {
    // 记录获取信号量后的时间戳t2
    g_t2 = get_cpu_cycles();

    // 计算抢占时间
    uint32_t preemption_time = g_t2 - g_t1;

    xprintf("抢占时间: %lu CPU cycles\r\n", preemption_time);

    // 短暂延迟后释放信号量
    vTaskDelay(pdMS_TO_TICKS(10));
    xSemaphoreGive(g_semaphore);
  }

  xprintf("高优先级任务1完成\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 低优先级任务2
static void vLowPriorityTask2(void *pvParameters) {
  xprintf("低优先级任务2启动\r\n");

  // 获取信号量
  if (xSemaphoreTake(g_semaphore, portMAX_DELAY) == pdTRUE) {
    xprintf("低优先级任务2获取信号量\r\n");

    // 在临界区工作一段时间
    vTaskDelay(pdMS_TO_TICKS(50));

    xprintf("低优先级任务2释放信号量\r\n");

    // 记录释放信号量的时间戳t1
    g_t1 = get_cpu_cycles();

    // 释放信号量（高优先级任务1会立即抢占）
    xSemaphoreGive(g_semaphore);
  }

  xprintf("低优先级任务2完成\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 启动测试
void vStartPreemptionTimeTest(void) {
  xprintf("========== 抢占时间测试 ==========\r\n");
  xprintf("测试说明: 测量高优先级任务从变为就绪态到开始执行的时间\r\n");

  // 创建信号量
  g_semaphore = xSemaphoreCreateBinary();
  if (g_semaphore == NULL) {
    xprintf("错误: 无法创建信号量\r\n");
    return;
  }
  // 先让信号量可用，这样低优先级任务可以获取
  xSemaphoreGive(g_semaphore);

  // 再创建高优先级任务1
  if (xTaskCreate(vHighPriorityTask1, "HighTask1", PREEMPTION_STACK_SIZE, NULL,
                  HIGH_PRIORITY, NULL) != pdPASS) {
    xprintf("错误: 无法创建高优先级任务1\r\n");
    vTaskDelete(g_lowTaskHandle);
    vSemaphoreDelete(g_semaphore);
    return;
  }
  // 短暂延迟让低优先级任务先运行并获取信号量
  vTaskDelay(pdMS_TO_TICKS(10));
  // 先创建低优先级任务2
  if (xTaskCreate(vLowPriorityTask2, "LowTask2", PREEMPTION_STACK_SIZE, NULL,
                  LOW_PRIORITY, &g_lowTaskHandle) != pdPASS) {
    xprintf("错误: 无法创建低优先级任务2\r\n");
    vSemaphoreDelete(g_semaphore);
    return;
  }

  xprintf("抢占时间测试任务创建成功\r\n");
}