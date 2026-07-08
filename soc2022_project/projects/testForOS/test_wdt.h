#ifndef TEST_WDT_H
#define TEST_WDT_H

#include "FreeRTOS.h"
#include "task.h"
#include "drv_wdt.h"

/* 统一返回码定义 */
#ifndef OK
#define OK      pdPASS
#endif
#ifndef ERROR
#define ERROR   pdFAIL
#endif

/* 测试函数声明 */
void vStartWdtTest(void);

/* 简单看门狗测试 */
void vStartSimpleWdtTest(void);

#endif /* TEST_WDT_H */
