#include "test_sem_give_time.h"
#include "XYR3201.h"
#include "xprintf.h"
#include <stdint.h>

/* 测试配置 */
#define SEM_GIVE_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define SEM_GIVE_TEST_PRIORITY_HIGH (tskIDLE_PRIORITY + 3)
#define SEM_GIVE_TEST_PRIORITY_LOW (tskIDLE_PRIORITY + 2)
#define TEST_COUNT 5

/* 测试数据 */
static volatile uint32_t ulGiveStartCycles = 0;
static volatile uint32_t ulTakeEndCycles = 0;

/* 信号量句柄 */
static SemaphoreHandle_t xSemaphore = NULL;

/* 任务句柄 */
static TaskHandle_t xHighTaskHandle = NULL;
static TaskHandle_t xLowTaskHandle = NULL;
static TaskHandle_t xSemGiveTestHandle = NULL;

/* 获取CPU周期计数器的函数 */
static inline uint32_t get_cpu_cycles(void) {
  uint32_t cycles;
  __asm__ volatile("rdcycle %0" : "=r"(cycles));
  return cycles;
}

/* 高优先级任务1 - 获取信号量 */
static void vHighPriorityTask(void *pvParameters) {
  xprintf("高优先级任务1启动\n");

  /* 获取信号量（阻塞等待） */
  xSemaphoreTake(xSemaphore, portMAX_DELAY);

  /* 记录获取完成时的CPU周期计数 */
  ulTakeEndCycles = get_cpu_cycles();

  xprintf("任务1获取信号量时，cycles: %u\n", (unsigned)ulTakeEndCycles);
  xprintf("任务2准备释放信号量时，cycles: %u\n", (unsigned)ulGiveStartCycles);
  /* 计算从释放到获取的CPU周期数 */
  uint32_t switchCycles = ulTakeEndCycles - ulGiveStartCycles;

  xprintf("释放到获取时间: %u CPU cycles\n", (unsigned)switchCycles);

  xprintf("高优先级任务1完成\n");
  vTaskDelete(NULL);
}

/* 低优先级任务2 - 释放信号量 */
static void vLowPriorityTask(void *pvParameters) {
  ulGiveStartCycles = get_cpu_cycles();
  /* 释放信号量 */
  xSemaphoreGive(xSemaphore);

  xprintf("低优先级任务2完成\n");
  vTaskDelete(NULL);
}

/* 信号量释放时间测试主函数 */
static void vSemGiveTimeTestMain(void *pvParameters) {
  xprintf("\n========== 信号量释放时间测试 ==========\n");
  xprintf("测试说明: 测量信号量释放到被获取的延时\n");

  /* 创建信号量 */
  xSemaphore = xSemaphoreCreateBinary();
  if (xSemaphore == NULL) {
    xprintf("❌ 创建信号量失败\n");
    vTaskDelete(NULL);
    return;
  }

  /* 创建高优先级任务 */
  xTaskCreate(vHighPriorityTask, "HighTask", SEM_GIVE_TEST_STACK_SIZE, NULL,
              SEM_GIVE_TEST_PRIORITY_HIGH, &xHighTaskHandle);

  /* 创建低优先级任务 */
  xTaskCreate(vLowPriorityTask, "LowTask", SEM_GIVE_TEST_STACK_SIZE, NULL,
              SEM_GIVE_TEST_PRIORITY_LOW, &xLowTaskHandle);

  /* 等待测试完成 */
  vTaskDelay(pdMS_TO_TICKS(8000));

  /* 删除信号量 */
  vSemaphoreDelete(xSemaphore);

  xprintf("\n========== 测试完成 ==========\n");
  vTaskDelete(NULL);
}

/* 启动信号量释放时间测试 */
void vStartSemGiveTimeTest(void) {
  xprintf("\n[信号量释放测试] 创建信号量释放时间测试任务...\n");

  if (xSemGiveTestHandle != NULL) {
    xprintf("[信号量释放测试] 测试任务已存在，删除旧任务\n");
    vTaskDelete(xSemGiveTestHandle);
    xSemGiveTestHandle = NULL;
  }

  BaseType_t xResult = xTaskCreate(vSemGiveTimeTestMain,         // 任务函数
                                   "SemGiveTest",                // 任务名称
                                   SEM_GIVE_TEST_STACK_SIZE * 2, // 栈大小
                                   NULL,                         // 任务参数
                                   SEM_GIVE_TEST_PRIORITY_HIGH, // 任务优先级
                                   &xSemGiveTestHandle          // 任务句柄
  );

  if (xResult != pdPASS) {
    xprintf("[信号量释放测试] ❌ 创建测试任务失败！\n");
  } else {
    xprintf("[信号量释放测试] ✅ 信号量释放时间测试任务创建成功\n");
  }
}
