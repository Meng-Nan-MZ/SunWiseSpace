#include "test_mutex_create_time.h"
#include "xprintf.h"
#include <stdint.h>

/* 测试配置 */
#define MUTEX_CREATE_TIME_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define MUTEX_CREATE_TIME_TEST_PRIORITY (tskIDLE_PRIORITY + 2)
#define TEST_COUNT 5

/* 测试数据 */
static volatile uint32_t ulCreateStartCycles = 0;
static volatile uint32_t ulCreateEndCycles = 0;
static volatile uint32_t ulTestCount = 0;
static volatile uint32_t ulTotalCycles = 0;
static volatile uint32_t ulMaxCycles = 0;
static volatile uint32_t ulMinCycles = 0xFFFFFFFF;

/* 任务句柄 */
static TaskHandle_t xMutexCreateTimeTestHandle = NULL;

/* 获取CPU周期计数器的函数 */
static inline uint32_t get_cpu_cycles(void) {
  uint32_t cycles;
  __asm__ volatile("rdcycle %0" : "=r"(cycles));
  return cycles;
}

/* 互斥量创建耗时测试主函数 */
static void vMutexCreateTimeTestMain(void *pvParameters) {
  xprintf("\n========== 互斥量创建耗时测试 ==========\n");
  xprintf("测试说明: 测量互斥量创建函数的执行时间\n");
  xprintf("\n开始测试...\n");

  for (ulTestCount = 0; ulTestCount < TEST_COUNT; ulTestCount++) {
    /* 记录创建开始时间 - CPU周期计数 */
    ulCreateStartCycles = get_cpu_cycles();

    /* 创建互斥量 */
    SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();

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
    xprintf("第%u次测试: %u CPU cycles", (unsigned)(ulTestCount + 1),
            (unsigned)createCycles);

    if (xMutex != NULL) {
      /* 删除互斥量释放内存 */
      vSemaphoreDelete(xMutex);
    } else {
      xprintf(" - 创建失败");
    }
    xprintf("\n");
  }

  /* 计算统计结果 */
  uint32_t ulAvgCycles = ulTotalCycles / TEST_COUNT;

  xprintf("\n========== 测试结果统计 ==========\n");
  xprintf("测试次数: %u\n", (unsigned)TEST_COUNT);
  xprintf("平均耗时: %u CPU cycles\n", (unsigned)ulAvgCycles);
  xprintf("最大耗时: %u CPU cycles\n", (unsigned)ulMaxCycles);
  xprintf("最小耗时: %u CPU cycles\n", (unsigned)ulMinCycles);

  xprintf("\n========== 测试完成 ==========\n");
  vTaskDelete(NULL);
}

/* 启动互斥量创建耗时测试 */
void vStartMutexCreateTimeTest(void) {
  xprintf("\n[互斥量创建耗时测试] 创建互斥量创建耗时测试任务...\n");

  if (xMutexCreateTimeTestHandle != NULL) {
    xprintf("[互斥量创建耗时测试] 测试任务已存在，删除旧任务\n");
    vTaskDelete(xMutexCreateTimeTestHandle);
    xMutexCreateTimeTestHandle = NULL;
  }

  BaseType_t xResult =
      xTaskCreate(vMutexCreateTimeTestMain,          // 任务函数
                  "MutexCreateTimeTest",             // 任务名称
                  MUTEX_CREATE_TIME_TEST_STACK_SIZE, // 栈大小
                  NULL,                              // 任务参数
                  MUTEX_CREATE_TIME_TEST_PRIORITY,   // 任务优先级
                  &xMutexCreateTimeTestHandle        // 任务句柄
      );

  if (xResult != pdPASS) {
    xprintf("[互斥量创建耗时测试] ❌ 创建测试任务失败！\n");
  }
}
