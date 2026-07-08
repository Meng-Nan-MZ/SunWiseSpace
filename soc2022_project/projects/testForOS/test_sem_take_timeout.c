#include "test_sem_take_timeout.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "xprintf.h"
/*
static void vSemTakeTimeoutTestTask(void *pvParameters) {
  xprintf("========== 限时获取信号量 ==========\n");
  SemaphoreHandle_t sem = xSemaphoreCreateBinary();
  xSemaphoreGive(sem); // 预先释放信号量，确保第一次获取成功

  for (int i = 0; i < 5; ++i) {
    if (sem) {
      BaseType_t ret = xSemaphoreTake(sem, 100);
      if (ret == pdTRUE) {
        xprintf("[限时测试] 在超时时间内成功获取信号量\n");
      } else {
        xprintf("[限时测试] 超时，信号量不可用\n");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  
  vTaskDelete(NULL);
}

void vStartSemTakeTimeoutTest(void) {
  xTaskCreate(vSemTakeTimeoutTestTask, "SemTakeTimeoutTest", 256, NULL,
              tskIDLE_PRIORITY + 1, NULL);
}
*/

static SemaphoreHandle_t sem1 = NULL;
static SemaphoreHandle_t sem2 = NULL;

// TaskA: 尝试阻塞获取信号量
static void vSemTakeWaitTaskA(void *pvParameters) {
    xprintf("[TaskA] 尝试以阻塞方式获取信号量...\n");
    TickType_t tick_before = xTaskGetTickCount();
   
    BaseType_t ret = xSemaphoreTake(sem1, 100);
    TickType_t tick_after = xTaskGetTickCount();
    if (ret == pdTRUE) {
        xprintf("[TaskA] 阻塞获取成功，阻塞时长: %lu ticks\n", (unsigned long)(tick_after - tick_before));
    } else {
        xprintf("[TaskA] 阻塞获取失败。\n");
    }
    TickType_t tick_before2 = xTaskGetTickCount();
    ret = xSemaphoreTake(sem2, 100);
    TickType_t tick_after2 = xTaskGetTickCount();
    if (ret == pdTRUE) {
        xprintf("[TaskA] 阻塞获取成功，阻塞时长: %lu ticks\n", (unsigned long)(tick_after2 - tick_before2));
    } else {
        xprintf("[TaskA] 阻塞获取失败。\n");
    }
    vTaskDelete(NULL);
}

// TaskB: 延迟后释放信号量
static void vSemTakeWaitTaskB(void *pvParameters) {
    vTaskDelay(50); // 延迟100 ticks，确保TaskA已阻塞
    xprintf("[TaskB] 释放信号量1\n");
    xSemaphoreGive(sem1);

    vTaskDelay(200);
    xprintf("[TaskB] 释放信号量2\n");
    xSemaphoreGive(sem2);
    vTaskDelete(NULL);
}

void vStartSemTakeTimeoutTest(void) {
    xprintf("========== 阻塞方式获取信号量 ==========");
    sem1 = xSemaphoreCreateBinary();
    sem2 = xSemaphoreCreateBinary();
    if (sem1 && sem2) {
        // 不提前give，确保TaskA会阻塞
        xTaskCreate(vSemTakeWaitTaskA, "SemTakeWaitA", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
        xTaskCreate(vSemTakeWaitTaskB, "SemTakeWaitB", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
        // 任务结束后由任务自身删除信号量
    } else {
        xprintf("[测试失败] 无法创建信号量。\n");
    }
}
