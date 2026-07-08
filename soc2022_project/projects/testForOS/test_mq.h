#ifndef TEST_MQ_H
#define TEST_MQ_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* 消息队列同步通信测试 */
void vStartMqSyncTest(void);

/* 消息队列异步通信测试 */
void vStartMqAsyncTest(void);

/* 消息队列创建和删除测试 */
void vStartMqCreateDeleteTest(void);

#endif /* TEST_MQ_H */
