#include "test_mutex_release_delay.h"
#include "xprintf.h"
#include <stdint.h>

/* 测试配置 */
#define MUTEX_RELEASE_DELAY_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define MUTEX_RELEASE_DELAY_TEST_PRIORITY_HIGH (tskIDLE_PRIORITY + 3)
#define MUTEX_RELEASE_DELAY_TEST_PRIORITY_LOW (tskIDLE_PRIORITY + 2)

/* 测试数据 */
static volatile uint32_t ulReleaseStartCycles = 0;
static volatile uint32_t ulAcquireEndCycles = 0;

/* 互斥量句柄 */
static SemaphoreHandle_t xMutex = NULL;

/* 任务句柄 */
static TaskHandle_t xHighTaskHandle = NULL;
static TaskHandle_t xLowTaskHandle = NULL;
static TaskHandle_t xMutexReleaseDelayTestHandle = NULL;

/* 获取CPU周期计数器的函数 */
static inline uint32_t get_cpu_cycles(void) {
  uint32_t cycles;
  __asm__ volatile("rdcycle %0" : "=r"(cycles));
  return cycles;
}

/* 高优先级任务1 - 获取互斥量 */
static void vHighPriorityTask(void *pvParameters) {
  xprintf("高优先级任务1启动\n");

  vTaskDelay(pdMS_TO_TICKS(100));
  xprintf("高优先级任务1尝试获取互斥量\n");
  /* 获取互斥量（阻塞等待） */
  xSemaphoreTake(xMutex, portMAX_DELAY);

  /* 记录获取完成时间 */
  ulAcquireEndCycles = get_cpu_cycles();

  xprintf("低优先级任务2释放互斥量时，cycles: %u\n",
          (unsigned)ulReleaseStartCycles);
  xprintf("高优先级任务1获取互斥量完成，cycles: %u\n",
          (unsigned)ulAcquireEndCycles);
  /* 计算从释放到获取的时间 */
  uint32_t releaseDelay = ulAcquireEndCycles - ulReleaseStartCycles;

  xprintf("互斥量释放延迟: %u CPU cycles\n", (unsigned)releaseDelay);

  /* 在临界区中工作 */
  // vTaskDelay(pdMS_TO_TICKS(1000));
  xprintf("高优先级任务1在临界区中工作\n");

  /* 释放互斥量 */
  xSemaphoreGive(xMutex);
  xprintf("高优先级任务1释放互斥量\n");

  xprintf("高优先级任务1完成\n");
  vTaskDelete(NULL);
}

/* 低优先级任务2 - 释放互斥量 */
static void vLowPriorityTask(void *pvParameters) {
  /* 获取互斥量 */
  xSemaphoreTake(xMutex, portMAX_DELAY);
  xprintf("低优先级任务2获取互斥量\n");

  /* 在临界区中工作1000ms */
  vTaskDelay(pdMS_TO_TICKS(100));

  /* 记录释放开始时间 */
  ulReleaseStartCycles = get_cpu_cycles();

  /* 释放互斥量 */
  xSemaphoreGive(xMutex);

  xprintf("低优先级任务2完成\n");
  vTaskDelete(NULL);
}

/* 互斥量释放延迟测试主函数 */
static void vMutexReleaseDelayTestMain(void *pvParameters) {
  xprintf("\n========== 互斥量释放延迟测试 ==========\n");
  xprintf("测试说明: 测量互斥量释放到高优先级任务获取的延迟时间\n");

  /* 创建互斥量 */
  xMutex = xSemaphoreCreateMutex();
  if (xMutex == NULL) {
    xprintf("❌ 创建互斥量失败\n");
    vTaskDelete(NULL);
    return;
  }
  /* 创建高优先级任务 */
  xTaskCreate(vHighPriorityTask, "HighTask",
              MUTEX_RELEASE_DELAY_TEST_STACK_SIZE, NULL,
              MUTEX_RELEASE_DELAY_TEST_PRIORITY_HIGH, &xHighTaskHandle);

  /* 创建低优先级任务 */
  xTaskCreate(vLowPriorityTask, "LowTask", MUTEX_RELEASE_DELAY_TEST_STACK_SIZE,
              NULL, MUTEX_RELEASE_DELAY_TEST_PRIORITY_LOW, &xLowTaskHandle);

  /* 等待测试完成 */
  vTaskDelay(pdMS_TO_TICKS(5000));

  /* 删除互斥量 */
  vSemaphoreDelete(xMutex);

  xprintf("\n========== 测试完成 ==========\n");
  vTaskDelete(NULL);
}

/* 启动互斥量释放延迟测试 */
void vStartMutexReleaseDelayTest(void) {
  xprintf("\n[互斥量释放延迟测试] 创建互斥量释放延迟测试任务...\n");

  if (xMutexReleaseDelayTestHandle != NULL) {
    xprintf("[互斥量释放延迟测试] 测试任务已存在，删除旧任务\n");
    vTaskDelete(xMutexReleaseDelayTestHandle);
    xMutexReleaseDelayTestHandle = NULL;
  }

  BaseType_t xResult =
      xTaskCreate(vMutexReleaseDelayTestMain,              // 任务函数
                  "MutexReleaseDelayTest",                 // 任务名称
                  MUTEX_RELEASE_DELAY_TEST_STACK_SIZE * 2, // 栈大小
                  NULL,                                    // 任务参数
                  MUTEX_RELEASE_DELAY_TEST_PRIORITY_HIGH,  // 任务优先级
                  &xMutexReleaseDelayTestHandle            // 任务句柄
      );

  if (xResult != pdPASS) {
    xprintf("[互斥量释放延迟测试] ❌ 创建测试任务失败！\n");
  }
}
