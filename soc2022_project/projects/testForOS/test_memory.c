#include "test_memory.h"
#include "xprintf.h"
#include <stdint.h>

/* 测试配置 */
#define MEMORY_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define MEMORY_TEST_PRIORITY (tskIDLE_PRIORITY + 2)

/* 任务句柄 */
static TaskHandle_t xMemoryTestHandle = NULL;

/* ========== 简单内存测试 ========== */

/* 内存测试任务 */
static void vMemoryTestTask(void *pvParameters) {
  xprintf("\n========== 内存分配回收测试 ==========\n");

  /* 显示初始堆状态 */
  size_t xFreeHeapSize = xPortGetFreeHeapSize();
  xprintf("[内存测试] 初始堆大小: %u bytes\n", (unsigned)xFreeHeapSize);

  /* 分配100字节 */
  void *pmem1 = OSMemAlloc(100);
  if (pmem1 == NULL) {
    xprintf("[内存测试] 分配100字节失败！\n");
    vTaskDelete(NULL);
    return;
  }
  xprintf("[内存测试] 分配100字节成功，地址: 0x%08X\n",
          (unsigned)(uintptr_t)pmem1);

  /* 分配50字节 */
  void *pmem2 = OSMemAlloc(50);
  if (pmem2 == NULL) {
    xprintf("[内存测试] 分配50字节失败！\n");
    OSMemFree(pmem1);
    vTaskDelete(NULL);
    return;
  }
  xprintf("[内存测试] 分配50字节成功，地址: 0x%08X\n",
          (unsigned)(uintptr_t)pmem2);

  /* 回收50字节 */
  OSMemFree(pmem2);
  xprintf("[内存测试] 回收50字节成功，地址: 0x%08X\n",
          (unsigned)(uintptr_t)pmem2);

  /* 分配200字节 */
  void *pmem3 = OSMemAlloc(200);
  if (pmem3 == NULL) {
    xprintf("[内存测试] 分配200字节失败！\n");
    OSMemFree(pmem1);
    vTaskDelete(NULL);
    return;
  }
  xprintf("[内存测试] 分配200字节成功，地址: 0x%08X\n",
          (unsigned)(uintptr_t)pmem3);

  /* 回收所有内存 */
  OSMemFree(pmem1);
  xprintf("[内存测试] 回收100字节成功，地址: 0x%08X\n",
          (unsigned)(uintptr_t)pmem1);

  OSMemFree(pmem3);
  xprintf("[内存测试] 回收200字节成功，地址: 0x%08X\n",
          (unsigned)(uintptr_t)pmem3);

  /* 显示最终堆状态 */
  size_t xFinalFreeHeap = xPortGetFreeHeapSize();
  xprintf("[内存测试] 最终剩余堆大小: %u bytes\n", (unsigned)xFinalFreeHeap);
  xprintf("[内存测试] 内存分配回收测试完成\n");

  vTaskDelete(NULL);
}

/* 启动简单内存测试 */
void vStartSimpleMemoryTest(void) {
  xprintf("\n[内存测试] 创建内存测试任务...\n");

  if (xMemoryTestHandle != NULL) {
    xprintf("[内存测试] 测试任务已存在，删除旧任务\n");
    vTaskDelete(xMemoryTestHandle);
    xMemoryTestHandle = NULL;
  }

  BaseType_t xResult = xTaskCreate(vMemoryTestTask,        // 任务函数
                                   "MemoryTest",           // 任务名称
                                   MEMORY_TEST_STACK_SIZE, // 栈大小
                                   NULL,                   // 任务参数
                                   MEMORY_TEST_PRIORITY,   // 任务优先级
                                   &xMemoryTestHandle      // 任务句柄
  );

  if (xResult != pdPASS) {
    xprintf("[内存测试] 创建测试任务失败！\n");
  } else {
    xprintf("[内存测试] 内存测试任务创建成功\n");
  }
}

/* 启动内存测试 */
void vStartMemoryTest(void) { vStartSimpleMemoryTest(); }
