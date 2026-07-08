/*
 * Task state transition test (5.1.4)
 */
#include "test_state.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "xprintf.h"

#define STATE_STACK (configMINIMAL_STACK_SIZE)
#define PRIO_T1 (tskIDLE_PRIORITY + 4)
#define PRIO_T2 (tskIDLE_PRIORITY + 2)
#define PRIO_T3 (tskIDLE_PRIORITY + 2)
#define PRIO_T4 (tskIDLE_PRIORITY + 1)
#define PRIO_T5 (tskIDLE_PRIORITY + 1)

static TaskHandle_t t1_handle = NULL;
static TaskHandle_t t2_handle = NULL;
static TaskHandle_t t3_handle = NULL;
static TaskHandle_t t4_handle = NULL;
static TaskHandle_t t5_handle = NULL;
static QueueHandle_t q_msg = NULL;

static void vTask1_SelfSuspend(void *arg) {
  xprintf("[状态测试] 任务1: 运行 -> 即将自挂起\n");
  xprintf("[状态测试] 任务1: 调用 vTaskSuspend(NULL) 进入挂起态\n");
  vTaskSuspend(NULL);
  /* 若被外部恢复，这里会继续运行 */
  xprintf("[状态测试] 任务1: 被恢复，继续运行然后退出\n");
  vTaskDelete(NULL);
}

static void vTask2_Sender(void *arg) {
  int cnt = 0;
  vTaskDelay(pdMS_TO_TICKS(1500)); /* 先让接收者阻塞，再发送 */
  for (int i = 0; i < 3; i++) {
    cnt = i + 1;
    xprintf("[状态测试] 任务2: 发送消息 %d -> 队列\n", cnt);
    xQueueSend(q_msg, &cnt, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(800));
  }
  xprintf("[状态测试] 任务2: 发送完成\n");
  vTaskDelete(NULL);
}

static void vTask3_Receiver(void *arg) {
  int val;
  xprintf("[状态测试] 任务3: 准备接收，若队列空则进入阻塞态\n");
  for (int i = 0; i < 3; i++) {
    if (xQueueReceive(q_msg, &val, pdMS_TO_TICKS(5000)) == pdTRUE) {
      eTaskState s3 = (t3_handle ? eTaskGetState(t3_handle) : eDeleted);
      xprintf("[状态测试] 任务3当前状态t3=%d\n", (int)s3);
      xprintf("[状态测试] 任务3: 从阻塞/就绪 -> 运行态，收到消息=%d\n", val);
    } else {
      xprintf("[状态测试] 任务3: 超时未收到，仍阻塞/就绪等待\n");
    }
  }
  xprintf("[状态测试] 任务3: 接收完成\n");
  vTaskDelete(NULL);
}

static void vTask4_Monitor(void *arg) {
  for (int i = 0; i < 6; i++) {
    eTaskState s1 = (t1_handle ? eTaskGetState(t1_handle) : eDeleted);
    eTaskState s3 = (t3_handle ? eTaskGetState(t3_handle) : eDeleted);
    xprintf("[状态测试] 任务4(监控): t1状态=%d, t3状态=%d\n", (int)s1, (int)s3);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  vTaskDelete(NULL);
}

static void vTask5_Resumer(void *arg) {
  /* 等任务1挂起后，恢复它，展示 SUSPENDED->READY->RUNNING */
  vTaskDelay(pdMS_TO_TICKS(4000));
  if (t1_handle) {
    xprintf("[状态测试] 任务5: 恢复任务1\n");
    vTaskResume(t1_handle);
  }
  vTaskDelete(NULL);
}

static void vTaskStateTestMain(void *arg) {
  BaseType_t ret;

  xprintf("\n========== 任务状态转换测试 ==========\n");

  q_msg = xQueueCreate(2, sizeof(int));
  if (q_msg == NULL) {
    xprintf("[状态测试] ❌ 创建队列失败\n");
    vTaskDelete(NULL);
    return;
  }

  ret = xTaskCreate(vTask1_SelfSuspend, "stateT1", STATE_STACK, NULL, PRIO_T1,
                    &t1_handle);
  ret |= xTaskCreate(vTask2_Sender, "stateT2", STATE_STACK, NULL, PRIO_T2,
                     &t2_handle);
  ret |= xTaskCreate(vTask3_Receiver, "stateT3", STATE_STACK, NULL, PRIO_T3,
                     &t3_handle);
  ret |= xTaskCreate(vTask4_Monitor, "stateT4", STATE_STACK, NULL, PRIO_T4,
                     &t4_handle);
  ret |= xTaskCreate(vTask5_Resumer, "stateT5", STATE_STACK, NULL, PRIO_T5,
                     &t5_handle);

  if (ret != pdPASS) {
    xprintf("[状态测试] ❌ 创建任务失败\n");
    vTaskDelete(NULL);
    return;
  }

  /* 主控任务退出 */
  vTaskDelete(NULL);
}

void vStartTaskStateTest(void) {
  if (xTaskCreate(vTaskStateTestMain, "stateTest", STATE_STACK * 2, NULL,
                  PRIO_T1, NULL) != pdPASS) {
    xprintf("[状态测试] 创建测试主任务失败\n");
  } else {
    xprintf("[状态测试] 测试任务已启动\n");
  }
}