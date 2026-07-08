#ifndef TEST_MUTEX_CREATE_TIME_H
#define TEST_MUTEX_CREATE_TIME_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* 统一返回码定义 */
#ifndef OK
#define OK      pdPASS
#endif
#ifndef ERROR
#define ERROR   pdFAIL
#endif

/* 互斥量类型定义 */
typedef SemaphoreHandle_t MUTEX_ID;



/* 测试函数声明 */
void vStartMutexCreateTimeTest(void);

#endif /* TEST_MUTEX_CREATE_TIME_H */
