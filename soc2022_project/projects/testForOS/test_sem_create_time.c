#include "test_sem_create_time.h"
#include "xprintf.h"
#include <stdint.h>

/* 测试配置 */
#define SEM_CREATE_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define SEM_CREATE_TEST_PRIORITY (tskIDLE_PRIORITY + 2)
#define TEST_COUNT 10

/* 测试数据 */
static volatile uint32_t ulCreateStartTime = 0;
static volatile uint32_t ulCreateEndTime = 0;
static volatile uint32_t ulTestCount = 0;
static volatile uint32_t ulTotalTime = 0;
static volatile uint32_t ulMaxTime = 0;
static volatile uint32_t ulMinTime = 0xFFFFFFFF;

/* 高精度时间测量数据 */
static volatile uint32_t ulCreateStartCycles = 0;
static volatile uint32_t ulCreateEndCycles = 0;
static volatile uint32_t ulTotalCycles = 0;
static volatile uint32_t ulMaxCycles = 0;
static volatile uint32_t ulMinCycles = 0xFFFFFFFF;

/* 获取CPU周期计数器的函数 */
static inline uint32_t get_cpu_cycles(void) {
  uint32_t cycles;
  __asm__ volatile("rdcycle %0" : "=r"(cycles));
  return cycles;
}

/* 任务句柄 */
static TaskHandle_t xSemCreateTestHandle = NULL;

/* 信号量创建耗时测试任务 */
static void vSemCreateTimeTestTask(void *pvParameters) {
  xprintf("\n========== 信号量创建耗时测试 ==========\n");
  xprintf("测试配置:\n");
  xprintf("  - 测试次数: %d\n", TEST_COUNT);
  xprintf("\n开始测试...\n");

  for (ulTestCount = 0; ulTestCount < TEST_COUNT; ulTestCount++) {
    /* 记录创建开始时间 - CPU周期计数 */
    ulCreateStartCycles = get_cpu_cycles();

    /* 创建信号量 */
    SemaphoreHandle_t xSemaphore = xSemaphoreCreateBinary();

    /* 记录创建结束时间 - CPU周期计数 */
    ulCreateEndCycles = get_cpu_cycles();

    /* 计算创建耗时 - CPU周期计数 */
    uint32_t createCycles = ulCreateEndCycles - ulCreateStartCycles;
    ulTotalCycles += createCycles;

    if (createCycles > ulMaxCycles)
      ulMaxCycles = createCycles;
    if (createCycles < ulMinCycles)
      ulMinCycles = createCycles;

    /* 打印测试结果 */
    xprintf("第%u次测试: %u CPU cycles\n",
            (unsigned)(ulTestCount + 1), (unsigned)createCycles);

    /* 删除信号量 */
    if (xSemaphore != NULL) {
      vSemaphoreDelete(xSemaphore);
    }

    /* 短暂延时 */
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  /* 计算并显示统计结果 */
  if (ulTestCount > 0) {
    uint32_t avgTime = ulTotalTime / ulTestCount;
    uint32_t avgCycles = ulTotalCycles / ulTestCount;

    xprintf("\n========== 测试结果 ==========\n");
    xprintf("测试次数: %u", (unsigned)ulTestCount);
    xprintf("\n--- CPU周期统计 ---\n");
    xprintf("最小耗时: %u CPU cycles\n", (unsigned)ulMinCycles);
    xprintf("最大耗时: %u CPU cycles\n", (unsigned)ulMaxCycles);
    xprintf("平均耗时: %u CPU cycles\n", (unsigned)avgCycles);
    xprintf("总耗时: %u CPU cycles\n", (unsigned)ulTotalCycles);
  }

  xprintf("\n========== 信号量创建耗时测试完成 ==========\n");
  vTaskDelete(NULL);
}

/* 启动信号量创建耗时测试 */
void vStartSemCreateTimeTest(void) {
  xprintf("\n[信号量创建测试] 创建信号量创建耗时测试任务...\n");

  if (xSemCreateTestHandle != NULL) {
    xprintf("[信号量创建测试] 测试任务已存在，删除旧任务\n");
    vTaskDelete(xSemCreateTestHandle);
    xSemCreateTestHandle = NULL;
  }

  BaseType_t xResult = xTaskCreate(vSemCreateTimeTestTask, // 任务函数
                                   "SemCreateTest",        // 任务名称
                                   SEM_CREATE_TEST_STACK_SIZE * 2, // 栈大小
                                   NULL,                     // 任务参数
                                   SEM_CREATE_TEST_PRIORITY, // 任务优先级
                                   &xSemCreateTestHandle     // 任务句柄
  );

  if (xResult != pdPASS) {
    xprintf("[信号量创建测试] ❌ 创建测试任务失败！\n");
  } else {
    xprintf("[信号量创建测试] ✅ 信号量创建耗时测试任务创建成功\n");
  }
}
