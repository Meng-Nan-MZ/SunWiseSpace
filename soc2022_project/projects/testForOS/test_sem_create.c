#include "test_sem_create.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "xprintf.h"
#include <stdint.h>

static void vSemCreateTestTask(void *pvParameters) {
  xprintf("========== 创建信号量和计数信号量 ==========\n");

  xprintf("[用例一] 创建二值信号量\n");
  SemaphoreHandle_t binSem[5]; 
  for (int i = 0; i < 5; i++) {
    binSem[i] = xSemaphoreCreateBinary();
    if (binSem[i] != NULL) {
      xprintf("[创建测试] %d: 二值信号量创建成功: 0x%08X\n", i, (uint32_t)binSem[i]);
    } else {
      xprintf("[创建测试] %d: 二值信号量创建失败!\n", i);
    }
  }

  for (int i = 0; i < 5; i++) {
    if (binSem[i]) vSemaphoreDelete(binSem[i]);
  }

  xprintf("[用例二] 创建计数信号量\n");


  SemaphoreHandle_t countSem[5];
  for (int i = 0; i < 5; i++) {
    countSem[i] = xSemaphoreCreateCounting(5, 2);
    if (countSem[i] != NULL) {
      xprintf("[创建测试] %d: 计数信号量创建成功: 0x%08X\n", i, (uint32_t)countSem[i]);
    } else {
      xprintf("[创建测试] %d: 计数信号量创建失败!\n", i);
    }
  }

  for (int i = 0; i < 5; i++) {
    if (countSem[i]) vSemaphoreDelete(countSem[i]);
  }

  vTaskDelete(NULL);
}

void vStartSemCreateTest(void) {
  xTaskCreate(vSemCreateTestTask, "SemCreateTest", 256, NULL,
              tskIDLE_PRIORITY + 1, NULL);
}
