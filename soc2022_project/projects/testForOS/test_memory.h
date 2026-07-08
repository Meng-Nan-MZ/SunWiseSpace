#ifndef TEST_MEMORY_H
#define TEST_MEMORY_H

#include "FreeRTOS.h"
#include "task.h"

/* 统一返回码定义 */
#ifndef OK
#define OK      pdPASS
#endif
#ifndef ERROR
#define ERROR   pdFAIL
#endif

/* 内存分配函数封装 */
#define OSMemAlloc(size)        pvPortMalloc(size)
#define OSMemFree(ptr)          vPortFree(ptr)

/* 测试函数声明 */
void vStartMemoryTest(void);

/* 简单内存测试 */
void vStartSimpleMemoryTest(void);

#endif /* TEST_MEMORY_H */
