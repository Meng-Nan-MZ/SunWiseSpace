#include "test_sem_delete.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "xprintf.h"

static void vSemDeleteTestTask(void *pvParameters) {
  xprintf("========== 删除信号量 ==========\n");

  xprintf("[用例一] 删除有效信号量\n");
  SemaphoreHandle_t sem = xSemaphoreCreateBinary();
  if (sem) {
      vSemaphoreDelete(sem);
    xprintf("[删除测试] 有效信号量删除成功。\n");
  }

  xprintf("[用例二] 删除无效信号量\n");
  SemaphoreHandle_t invalidSem = NULL;
  vSemaphoreDelete(invalidSem); // 断言测试

  xprintf("[删除测试] 无效信号量删除应触发断言（断言失败）\n");
  vTaskDelete(NULL);
}

void vStartSemDeleteTest(void) {
  xTaskCreate(vSemDeleteTestTask, "SemDeleteTest", 256, NULL,
              tskIDLE_PRIORITY + 1, NULL);
}
