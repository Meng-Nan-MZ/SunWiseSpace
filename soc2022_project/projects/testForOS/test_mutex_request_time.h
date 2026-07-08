#ifndef TEST_MUTEX_REQUEST_TIME_H
#define TEST_MUTEX_REQUEST_TIME_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// 测试函数声明
void vStartMutexRequestTimeTest(void);

// 互斥量ID类型定义
typedef SemaphoreHandle_t MUTEX_ID;

// 互斥量创建函数声明
extern MUTEX_ID OSSemMCreate(void);

#endif // TEST_MUTEX_REQUEST_TIME_H
