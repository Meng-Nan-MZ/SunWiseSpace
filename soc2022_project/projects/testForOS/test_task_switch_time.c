#include "test_task_switch_time.h"
#include "XYR3201.h"
#include "xprintf.h"

// 测试配置
#define TASK_SWITCH_TEST_COUNT 1
#define TASK_SWITCH_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define HIGH_PRIORITY 3
#define LOW_PRIORITY 2

// 全局变量
static TaskHandle_t g_highTaskHandle = NULL;
static TaskHandle_t g_lowTaskHandle = NULL;
static volatile uint32_t g_switchTimes[TASK_SWITCH_TEST_COUNT];
static volatile uint32_t g_testCount = 0;
static volatile uint32_t g_t1 = 0; // 任务1放弃CPU的时间戳
static volatile uint32_t g_t2 = 0; // 任务2开始执行的时间戳

// CPU周期计数函数
static inline uint32_t get_cpu_cycles(void) {
  uint32_t cycles;
  __asm__ volatile("rdcycle %0" : "=r"(cycles));
  return cycles;
}

// 高优先级任务1
static void vHighPriorityTask1(void *pvParameters) {
  xprintf("========== 任务切换时间测试 ==========\r\n");
  xprintf("测试说明: 测量从任务1放弃执行到任务2开始执行的时间\r\n");
  xprintf("高优先级任务1启动\r\n");

  // 记录任务1放弃CPU前的时间戳t1
  g_t1 = get_cpu_cycles();

  // 放弃CPU，让低优先级任务2执行
  vTaskDelay(pdMS_TO_TICKS(100));
  // vTaskSuspend(NULL);
  //  显示测试结果
  uint32_t switch_time = g_switchTimes[0];

  xprintf("\r\n========== 任务切换时间结果 ==========\r\n");
  xprintf("任务切换时间: %lu CPU cycles\r\n", switch_time);

#ifdef CPU_CLOCK_HZ
  xprintf("任务切换时间: %.3f 微秒\r\n",
          (float)switch_time * 1000000.0f / CPU_CLOCK_HZ);
#endif

  xprintf("========== 测试完成 ==========\r\n\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 低优先级任务2
static void vLowPriorityTask2(void *pvParameters) {
  // 记录任务2开始执行的时间戳t2
  g_t2 = get_cpu_cycles();
  xprintf("低优先级任务2启动\r\n");
  // 计算切换时间
  uint32_t switch_time = g_t2 - g_t1;
  g_switchTimes[0] = switch_time;

  xprintf("任务切换时间: %lu CPU cycles\r\n", switch_time);

  // 更新测试计数
  g_testCount = 1;

  xprintf("低优先级任务2完成\r\n");
  // vTaskResume(g_highTaskHandle);
  //  删除任务
  vTaskDelete(NULL);
}

// 启动测试
void vStartTaskSwitchTimeTest(void) {
  // 重置测试数据
  g_testCount = 0;
  g_switchTimes[0] = 0;
  // 创建高优先级任务1
  if (xTaskCreate(vHighPriorityTask1, "HighTask1", TASK_SWITCH_STACK_SIZE, NULL,
                  HIGH_PRIORITY, &g_highTaskHandle) != pdPASS) {
    xprintf("错误: 无法创建高优先级任务1\r\n");
    return;
  }

  // 创建低优先级任务2
  if (xTaskCreate(vLowPriorityTask2, "LowTask2", TASK_SWITCH_STACK_SIZE, NULL,
                  LOW_PRIORITY, &g_lowTaskHandle) != pdPASS) {
    xprintf("错误: 无法创建低优先级任务2\r\n");
    return;
  }
  xprintf("任务切换时间测试任务创建成功\r\n");
}
