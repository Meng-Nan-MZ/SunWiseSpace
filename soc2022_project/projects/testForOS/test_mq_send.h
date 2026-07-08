#ifndef TEST_MQ_SEND_H
#define TEST_MQ_SEND_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* 阻塞方式发送消息队列测试 */
void vStartMqBlockingSendTest(void);

/* 非阻塞方式发送消息队列测试 */
void vStartMqNonBlockingSendTest(void);

/* 消息队列发送功能综合测试 */
void vStartMqSendTest(void);

#endif /* TEST_MQ_SEND_H */
