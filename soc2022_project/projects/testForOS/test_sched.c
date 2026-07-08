/*
 * 优先级调度测试
 * 创建5个任务，优先级依次升高，观察执行顺序
 * 通过修改优先级来改变调度顺序
 */

#include "test_sched.h"
#include "FreeRTOS.h"
#include "task.h"
#include "xprintf.h"
#include <stdio.h>

#define SCHED_STACK_SIZE (configMINIMAL_STACK_SIZE)


#define PRIO_T1   (tskIDLE_PRIORITY + 1)  // 最低优先级
#define PRIO_T2   (tskIDLE_PRIORITY + 2)
#define PRIO_T3   (tskIDLE_PRIORITY + 3)
#define PRIO_T4   (tskIDLE_PRIORITY + 4)
#define PRIO_T5   (tskIDLE_PRIORITY + 5)  // 最高优先级


#define SAME_PRIO_T1 (tskIDLE_PRIORITY + 2)
#define SAME_PRIO_T2 (tskIDLE_PRIORITY + 2)
#define SAME_PRIO_T3 (tskIDLE_PRIORITY + 2)
#define SAME_PRIO_T4 (tskIDLE_PRIORITY + 2)
#define SAME_PRIO_T5 (tskIDLE_PRIORITY + 2)

static TaskHandle_t task_handles[5] = {0};
static volatile uint32_t execution_count[5] = {0};

// 工作任务函数
static void vWorkerTask(void *pvParameters) {
  const uint32_t task_id = (uint32_t)(uintptr_t)pvParameters;
  UBaseType_t current_priority;
  uint32_t step = 0;

  xprintf("[调度测试] 任务%d启动，初始优先级: %d\n", task_id,
          task_id + tskIDLE_PRIORITY);

  while (1) {
    // 获取当前优先级
    current_priority = uxTaskPriorityGet(NULL);

    // 输出执行信息
    xprintf("[调度测试] 任务%d执行 (步骤:%d, 优先级:%d)\n", task_id, ++step,
            current_priority);

    // 增加执行计数
    execution_count[task_id - 1]++;

    // 让出CPU
    taskYIELD();

    // 短暂延迟
    vTaskDelay(pdMS_TO_TICKS(6));
  }
}

// 优先级修改任务
static void vPriorityModifierTask(void *pvParameters) {
  xprintf("\n========== 优先级调度测试开始 ==========\n");
  xprintf("初始优先级顺序: T1(%d) < T2(%d) < T3(%d) < T4(%d) < T5(%d)\n",
          PRIO_T1, PRIO_T2, PRIO_T3, PRIO_T4, PRIO_T5);
  xprintf("预期执行顺序: T5 > T4 > T3 > T2 > T1\n\n");

  // 等待一段时间观察初始执行顺序
  xprintf("=== 阶段1: 观察初始优先级执行顺序 ===\n");
  vTaskDelay(pdMS_TO_TICKS(500));

  // 修改优先级：将T1提升为最高，T5降为最低
  xprintf("\n=== 阶段2: 修改优先级 ===\n");
  xprintf("将任务1提升为最高优先级，任务5降为最低优先级\n");

  if (task_handles[0] != NULL) {
    vTaskPrioritySet(task_handles[0], PRIO_T5); // T1 -> 最高优先级
    xprintf("任务1优先级已修改为: %d\n", PRIO_T5);
  }

  if (task_handles[4] != NULL) {
    vTaskPrioritySet(task_handles[4], PRIO_T1); // T5 -> 最低优先级
    xprintf("任务5优先级已修改为: %d\n", PRIO_T1);
  }

  xprintf("新的优先级顺序: T5(%d) < T2(%d) < T3(%d) < T4(%d) < T1(%d)\n",
          PRIO_T1, PRIO_T2, PRIO_T3, PRIO_T4, PRIO_T5);
  xprintf("预期新执行顺序: T1 > T4 > T3 > T2 > T5\n\n");

  // 再等待一段时间观察修改后的执行顺序
  xprintf("=== 阶段3: 观察修改后的执行顺序 ===\n");
  vTaskDelay(pdMS_TO_TICKS(500));

  // 再次修改优先级：恢复原始顺序
  xprintf("\n=== 阶段4: 恢复原始优先级 ===\n");
  if (task_handles[0] != NULL) {
    vTaskPrioritySet(task_handles[0], PRIO_T1);
    xprintf("任务1优先级已恢复为: %d\n", PRIO_T1);
  }

  if (task_handles[4] != NULL) {
    vTaskPrioritySet(task_handles[4], PRIO_T5);
    xprintf("任务5优先级已恢复为: %d\n", PRIO_T5);
  }

  // 输出执行统计
  xprintf("\n=== 执行统计 ===\n");
  for (int i = 0; i < 5; i++) {
    xprintf("任务%d执行次数: %d\n", i + 1, execution_count[i]);
  }

  xprintf("\n========== 优先级调度测试完成 ==========\n");

  // 删除所有任务
  for (int i = 0; i < 5; i++) {
    if (task_handles[i] != NULL) {
      vTaskDelete(task_handles[i]);
      task_handles[i] = NULL;
    }
  }

  vTaskDelete(NULL);
}

// 主测试函数
void vStartSchedulerPolicyTest(void) {
  BaseType_t ret;

  xprintf("\n========== 创建优先级调度测试任务 ==========\n");

  // 创建优先级修改控制任务
  ret = xTaskCreate(vPriorityModifierTask, "PriorityModifier",
                    SCHED_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 6, NULL);

  if (ret != pdPASS) {
    xprintf("[调度测试] ❌ 创建优先级修改任务失败\n");
    return;
  }

  // 创建5个工作任务
  ret = xTaskCreate(vWorkerTask, "Task1", SCHED_STACK_SIZE,
                    (void *)(uintptr_t)1, PRIO_T1, &task_handles[0]);
  ret &= xTaskCreate(vWorkerTask, "Task2", SCHED_STACK_SIZE,
                     (void *)(uintptr_t)2, PRIO_T2, &task_handles[1]);
  ret &= xTaskCreate(vWorkerTask, "Task3", SCHED_STACK_SIZE,
                     (void *)(uintptr_t)3, PRIO_T3, &task_handles[2]);
  ret &= xTaskCreate(vWorkerTask, "Task4", SCHED_STACK_SIZE,
                     (void *)(uintptr_t)4, PRIO_T4, &task_handles[3]);
  ret &= xTaskCreate(vWorkerTask, "Task5", SCHED_STACK_SIZE,
                     (void *)(uintptr_t)5, PRIO_T5, &task_handles[4]);

  if (ret != pdPASS) {
    xprintf("[调度测试] ❌ 创建工作任务失败\n");
    return;
  }

  xprintf("[调度测试] ✅ 所有任务创建成功\n");
}

// 同优先级调度控制任务
static void vSamePriorityModifierTask(void *pvParameters) {
  xprintf("\n========== 同优先级调度测试开始 ==========\n");
  xprintf("所有任务优先级相同: T1(%d) = T2(%d) = T3(%d) = T4(%d) = T5(%d)\n",
          PRIO_T1, PRIO_T2, PRIO_T3, PRIO_T4, PRIO_T5);
  xprintf("预期执行顺序: 按时间片轮转调度 (Round Robin)\n\n");

  // 重置执行计数
  for (int i = 0; i < 5; i++) {
    execution_count[i] = 0;
  }

  // 观察同优先级时间片轮转调度
  xprintf("=== 观察同优先级时间片轮转调度 ===\n");
  vTaskDelay(pdMS_TO_TICKS(500));

  // 输出执行统计
  xprintf("\n=== 执行统计 ===\n");
  for (int i = 0; i < 5; i++) {
    xprintf("任务%d执行次数: %d\n", i + 1, execution_count[i]);
  }

  xprintf("\n========== 同优先级调度测试完成 ==========\n");

  // 删除所有任务
  for (int i = 0; i < 5; i++) {
    if (task_handles[i] != NULL) {
      vTaskDelete(task_handles[i]);
      task_handles[i] = NULL;
    }
  }

  vTaskDelete(NULL);
}

// 同优先级调度测试函数
void vStartSamePriorityTest(void) {
  BaseType_t ret;

  xprintf("\n========== 创建同优先级调度测试任务 ==========\n");

  // 创建同优先级控制任务
  ret = xTaskCreate(vSamePriorityModifierTask, "SamePriorityModifier",
                    SCHED_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 6, NULL);

  if (ret != pdPASS) {
    xprintf("[同优先级测试] ❌ 创建控制任务失败\n");
    return;
  }

  // 创建5个同优先级工作任务
  ret = xTaskCreate(vWorkerTask, "SameTask1", SCHED_STACK_SIZE,
                    (void *)(uintptr_t)1, SAME_PRIO_T1, &task_handles[0]);
  ret &= xTaskCreate(vWorkerTask, "SameTask2", SCHED_STACK_SIZE,
                     (void *)(uintptr_t)2, SAME_PRIO_T2, &task_handles[1]);
  ret &= xTaskCreate(vWorkerTask, "SameTask3", SCHED_STACK_SIZE,
                     (void *)(uintptr_t)3, SAME_PRIO_T3, &task_handles[2]);
  ret &= xTaskCreate(vWorkerTask, "SameTask4", SCHED_STACK_SIZE,
                     (void *)(uintptr_t)4, SAME_PRIO_T4, &task_handles[3]);
  ret &= xTaskCreate(vWorkerTask, "SameTask5", SCHED_STACK_SIZE,
                     (void *)(uintptr_t)5, SAME_PRIO_T5, &task_handles[4]);

  if (ret != pdPASS) {
    xprintf("[同优先级测试] ❌ 创建工作任务失败\n");
    return;
  }

  xprintf("[同优先级测试] ✅ 所有任务创建成功\n");
}