#ifndef TEST_TASK_SWITCH_H
#define TEST_TASK_SWITCH_H

#include "FreeRTOS.h"
#include "task.h"

/* 统一返回码定义 */
#ifndef OK
#define OK      pdPASS
#endif
#ifndef ERROR
#define ERROR   pdFAIL
#endif

/* 测试配置 */
#define TASK_SWITCH_TEST_STACK_SIZE     (configMINIMAL_STACK_SIZE)
#define TASK_SWITCH_TEST_PRIORITY_HIGH  (tskIDLE_PRIORITY + 3)
#define TASK_SWITCH_TEST_PRIORITY_LOW   (tskIDLE_PRIORITY + 2)

/* 测试宏定义开关 */
#define TEST_TASK_SWITCH_TIME

/* 测试函数声明 */
void vStartTaskSwitchTest(void);

/* 5.2.1 任务切换时间测试 */
void vStartTaskSwitchTimeTest(void);

#endif /* TEST_TASK_SWITCH_H */
