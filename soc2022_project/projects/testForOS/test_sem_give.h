#ifndef __TEST_SEM_GIVE_H__
#define __TEST_SEM_GIVE_H__

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* 信号量释放时间测试函数声明 */
void vStartSemGiveTimeTest(void);
void vStartSemGiveTest(void);

#endif /* __TEST_SEM_GIVE_H__ */
