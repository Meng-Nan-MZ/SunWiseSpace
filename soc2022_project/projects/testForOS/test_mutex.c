
#include "test_mutex.h"
#include "xprintf.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

typedef void* MUTEX_ID;
MUTEX_ID OSMutexCreate(void) { return xSemaphoreCreateMutex(); }

static void vMutexCreateDeleteTestMain(void *pvParameters) {
  MUTEX_ID xTestMutex = NULL;

  xprintf("\n========== 互斥量创建和删除测试 ==========\n");
  xprintf("[INFO] 开始 FreeRTOS 互斥量创建和删除功能测试...\n\n");

  /* ========== 用例1: 创建互斥量 ========== */
  xprintf("========== 用例1: 创建互斥量 ==========\n");

  xTestMutex = OSMutexCreate();
  if (xTestMutex != NULL) {
    xprintf("[互斥量测试] ✅ 互斥量创建成功，句柄=0x%08X\n",
            (uint32_t)xTestMutex);
  } else {
    xprintf("[互斥量测试] ❌ 互斥量创建失败\n");
  }

  vTaskDelay(pdMS_TO_TICKS(100));

  /* ========== 用例2: 删除互斥量 ========== */
  xprintf("\n========== 用例2: 删除互斥量 ==========\n");

  if (xTestMutex != NULL) {
    vSemaphoreDelete(xTestMutex);
    xprintf("[互斥量测试] ✅ 互斥量删除成功\n");
    xTestMutex = NULL;
  }

  /* ========== 测试结果总结 ========== */
  vTaskDelay(pdMS_TO_TICKS(100));
  xprintf("=====================================\n");
  xprintf("[互斥量创建删除测试] 测试任务完成\n");
  vTaskDelete(NULL);
}


void vStartMutexTest(void) {
    xTaskCreate(vMutexCreateDeleteTestMain, "MutexTest", 256, NULL,
                tskIDLE_PRIORITY + 1, NULL);
}