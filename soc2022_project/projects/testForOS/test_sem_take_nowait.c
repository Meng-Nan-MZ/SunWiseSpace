#include "test_sem_take_nowait.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "xprintf.h"

static void vSemTakeNoWaitTestTask(void *pvParameters) {
  SemaphoreHandle_t sem = xSemaphoreCreateBinary();
  xprintf("========== 非阻塞获取信号量 ==========\n");
  
  if (sem) {
    // 场景1：信号量可用，非阻塞获取应立即成功
    xSemaphoreGive(sem);
    BaseType_t ret1 = xSemaphoreTake(sem, 0);
    if (ret1 == pdTRUE) {
      xprintf("[非阻塞测试] 信号量可用，立即获取成功。\n");
    } else {
      xprintf("[非阻塞测试] 信号量可用，但获取失败。\n");
    }

    // 场景2：信号量不可用，非阻塞获取应立即失败且不阻塞
    BaseType_t ret2 = xSemaphoreTake(sem, 0);
    if (ret2 == pdTRUE) {
      xprintf("[非阻塞测试] 信号量不可用，但意外获取成功。\n");
    } else {
      xprintf("[非阻塞测试] 信号量不可用，未阻塞，立即返回失败。\n");
    }

    vSemaphoreDelete(sem);
  } else {
    xprintf("[测试失败] 无法创建信号量。\n");
  }
  vTaskDelete(NULL);
}

void vStartSemTakeNoWaitTest(void) {
  xTaskCreate(vSemTakeNoWaitTestTask, "SemTakeNoWaitTest", 256, NULL,
              tskIDLE_PRIORITY + 1, NULL);
}
