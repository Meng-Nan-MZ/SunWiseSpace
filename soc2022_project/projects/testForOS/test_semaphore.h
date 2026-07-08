#ifndef TEST_SEMAPHORE_H
#define TEST_SEMAPHORE_H

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

/* 信号量类型定义 */
typedef SemaphoreHandle_t SEM_ID;

/* 二值信号量初始状态枚举 */
typedef enum {
    SEM_B_FREE = 0,    /* 二值信号量初始状态：空闲 */
    SEM_B_TAKEN = 1    /* 二值信号量初始状态：已占用 */
} SEM_B_STATE;

/* 信号量创建函数声明 */
SEM_ID OSSemBCreate(SEM_B_STATE init);
SEM_ID OSSemCCreate(U32 initCount);

/* 测试函数声明 */
void vStartSemaphoreTest(void);

/* 信号量获取测试函数声明 */
void vStartSemaphoreAcquireTest(void);

#endif /* TEST_SEMAPHORE_H */