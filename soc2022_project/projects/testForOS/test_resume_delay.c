#include "FreeRTOS.h"
#include "task.h"
#include "xprintf.h"

// 测试配置
#define RESUME_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define HIGH_PRIORITY 3
#define LOW_PRIORITY 2

// 全局变量
static TaskHandle_t g_lowTaskHandle = NULL;
static uint32_t g_t1 = 0;
static uint32_t g_t2 = 0;

// 获取CPU周期数（32位）
static inline uint32_t get_cpu_cycles(void) {
  uint32_t cycle;
  __asm__ volatile("rdcycle %0" : "=r"(cycle));
  return cycle;
}

// 高优先级任务1
static void vHighPriorityTask1(void *pvParameters) {
  xprintf("高优先级任务1启动\r\n");

  // 短暂延迟确保低优先级任务2先运行
  vTaskDelay(pdMS_TO_TICKS(10));

  xprintf("高优先级任务1挂起低优先级任务2\r\n");

  // 挂起低优先级任务2
  vTaskSuspend(g_lowTaskHandle);

  // 短暂延迟
  vTaskDelay(pdMS_TO_TICKS(100));

  xprintf("高优先级任务1准备解挂低优先级任务2\r\n");

  // 记录解挂前的时间戳t1
  g_t1 = get_cpu_cycles();

  // 解挂低优先级任务2
  vTaskResume(g_lowTaskHandle);
  
  g_t2 = get_cpu_cycles();
  // 计算并打印时间差
  uint32_t delay_cycles = g_t2 - g_t1;
  xprintf("解挂任务延迟时间: %lu CPU cycles\r\n", delay_cycles);

  // 等待低优先级任务2完成
  vTaskDelay(pdMS_TO_TICKS(100));

  xprintf("高优先级任务1完成\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 低优先级任务2
static void vLowPriorityTask2(void *pvParameters) {
  xprintf("低优先级任务2启动\r\n");
  xprintf("低优先级任务2将被挂起\r\n");
  vTaskDelay(pdMS_TO_TICKS(10));

  xprintf("低优先级任务2被解挂，继续执行\r\n");

  vTaskDelay(pdMS_TO_TICKS(50));

  xprintf("低优先级任务2完成\r\n");

  // 删除任务
  vTaskDelete(NULL);
}

// 启动测试
void vStartResumeDelayTest(void) {
  xprintf("========== 解挂任务延迟时间测试 ==========\r\n");
  xprintf("测试说明: 测量高优先级任务中唤醒低优先级任务的时间\r\n");

  if (xTaskCreate(vLowPriorityTask2, "LowTask2", RESUME_STACK_SIZE, NULL,
                  LOW_PRIORITY, &g_lowTaskHandle) != pdPASS) {
    xprintf("错误: 无法创建低优先级任务2\r\n");
    return;
  }
  vTaskDelay(pdMS_TO_TICKS(10));
  // 再创建高优先级任务1
  if (xTaskCreate(vHighPriorityTask1, "HighTask1", RESUME_STACK_SIZE, NULL,
                  HIGH_PRIORITY, NULL) != pdPASS) {
    xprintf("错误: 无法创建高优先级任务1\r\n");
    vTaskDelete(g_lowTaskHandle);
    return;
  }

  xprintf("解挂任务延迟时间测试任务创建成功\r\n");
}
