#ifndef TEST_SCHED_H
#define TEST_SCHED_H

#include "FreeRTOS.h"
#include "task.h"

/* 5.1.5 调度算法测试 */
void vStartSchedulerPolicyTest(void);

/* 5.1.6 同优先级任务调度测试 */
void vStartSamePriorityTest(void);

#endif /* TEST_SCHED_H */
