#ifndef TEST_INTERRUPT_DELAY_H
#define TEST_INTERRUPT_DELAY_H

#include "FreeRTOS.h"
#include "task.h"

// 测试函数声明
void vStartInterruptDelayTest(void);

// 中断处理函数声明
void TestTimer_IRQHandler(void);

#endif // TEST_INTERRUPT_DELAY_H
