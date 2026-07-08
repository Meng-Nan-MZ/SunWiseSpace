#include "test_wdt.h"
#include "XYR3201.h"
#include "xprintf.h"

/* 测试配置 */
#define WDT_TEST_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define WDT_TEST_PRIORITY (tskIDLE_PRIORITY + 2)

/* 任务句柄 */
static TaskHandle_t xWdtTestHandle = NULL;

/* 看门狗配置 */
static DeviceWdtConfig_t xWdtConfig = {
    .mode = eWdtMode_Reset,    // 复位模式
    .timeout = eWdtTimeout_1M, // 1M时钟周期超时（约1秒）
    .user_data = 0,
};

/* ========== 简单看门狗测试 ========== */

/* 看门狗测试任务 */
static void vWdtTestTask(void *pvParameters) {
  xprintf("\n========== 看门狗测试 ==========\n");

  /* 初始化看门狗 */
  xprintf("[看门狗测试] 初始化看门狗...\n");
  if (HW_WDT_InitConfig(WDT0, &xWdtConfig) != 0) {
    xprintf("[看门狗测试] 看门狗初始化失败！\n");
    vTaskDelete(NULL);
    return;
  }

  /* 启动看门狗 */
  xprintf("[看门狗测试] 启动看门狗...\n");
  if (HW_WDT_Start(WDT0) != 0) {
    xprintf("[看门狗测试] 看门狗启动失败！\n");
    vTaskDelete(NULL);
    return;
  }

  xprintf("[看门狗测试] 看门狗已启动，超时时间约1秒\n");

  /* 显示看门狗状态信息 */
  uint32_t wdt_status = HW_WDT_GetStatus(WDT0);
  uint32_t wdt_value = HW_WDT_GetValue(WDT0);
  xprintf("[看门狗测试] 看门狗状态: 0x%08X, 计数值: 0x%08X\n", wdt_status,
          wdt_value);

  xprintf("[看门狗测试] 开始正常清狗测试...\n");

  /* 正常清狗测试 - 持续清狗，应该不会复位 */
  for (int i = 0; i < 5; i++) {
    xprintf("[看门狗测试] 第%d次清狗\n", i + 1);
    HW_WDT_Feed(WDT0);
    wdt_value = HW_WDT_GetValue(WDT0);
    xprintf("[看门狗测试] 清狗后计数值: 0x%08X\n", wdt_value);
    vTaskDelay(pdMS_TO_TICKS(500)); // 延时500ms
  }

  xprintf("[看门狗测试] 正常清狗测试完成，系统未复位\n");
  xprintf("[看门狗测试] 开始不清狗测试（系统应该会复位）...\n");

  /* 不清狗测试 - 停止清狗，系统应该复位 */
  xprintf("[看门狗测试] 停止清狗，等待系统复位...\n");
  /* 禁用NMI中断以防止自动清狗 */
  drv_irq_disable(NMI_EXPn);
  /* 显示当前看门狗状态 */
  wdt_value = HW_WDT_GetValue(WDT0);
  xprintf("[看门狗测试] 停止清狗前计数值: 0x%08X\n", wdt_value);

  /* 延时超过看门狗超时时间 */
  for (int i = 0; i < 10; i++) {
    vTaskDelay(pdMS_TO_TICKS(2000)); // 每500ms检查一次
    wdt_value = HW_WDT_GetValue(WDT0);
    xprintf("[看门狗测试] 等待复位中... 第%d次检查，计数值: 0x%08X\n", i + 1,
            wdt_value);
  }

  xprintf("[看门狗测试] 错误：系统应该已经复位，但仍在运行！\n");

  /* 重新启用NMI中断 */
  drv_irq_enable(NMI_EXPn);
  xprintf("[看门狗测试] NMI中断已重新启用\n");

  vTaskDelete(NULL);
}

/* 启动简单看门狗测试 */
void vStartSimpleWdtTest(void) {
  xprintf("\n[看门狗测试] 创建看门狗测试任务...\n");

  if (xWdtTestHandle != NULL) {
    xprintf("[看门狗测试] 测试任务已存在，删除旧任务\n");
    vTaskDelete(xWdtTestHandle);
    xWdtTestHandle = NULL;
  }

  BaseType_t xResult = xTaskCreate(vWdtTestTask,        // 任务函数
                                   "WdtTest",           // 任务名称
                                   WDT_TEST_STACK_SIZE, // 栈大小
                                   NULL,                // 任务参数
                                   WDT_TEST_PRIORITY,   // 任务优先级
                                   &xWdtTestHandle      // 任务句柄
  );

  if (xResult != pdPASS) {
    xprintf("[看门狗测试] 创建测试任务失败！\n");
  } else {
    xprintf("[看门狗测试] 看门狗测试任务创建成功\n");
  }
}

/* 启动看门狗测试 */
void vStartWdtTest(void) { vStartSimpleWdtTest(); }
