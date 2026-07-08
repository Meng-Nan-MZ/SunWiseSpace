#ifndef TEST_TICK_H
#define TEST_TICK_H

#include "FreeRTOS.h"
#include "task.h"

/* 统一返回码定义 */
#ifndef OK
#define OK      pdPASS
#endif
#ifndef ERROR
#define ERROR   pdFAIL
#endif

/* 测试函数声明 */
void vStartTickTest(void);

/* 节拍获取测试 */
void vStartTickGetTest(void);

/* 节拍延时准确度测试 */
void vStartTickDelayTest(void);

#endif /* TEST_TICK_H */
