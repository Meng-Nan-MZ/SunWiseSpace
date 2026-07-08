#ifndef TEST_MUTEX_RELEASE_DELAY_H
#define TEST_MUTEX_RELEASE_DELAY_H

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

/* 互斥量创建函数声明 - 在其他文件中已定义 */
extern MUTEX_ID OSSemMCreate(void);

/* 测试函数声明 */
void vStartMutexReleaseDelayTest(void);

#endif /* TEST_MUTEX_RELEASE_DELAY_H */
