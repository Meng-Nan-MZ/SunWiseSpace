/*
 * Copyright (c) 2020-2023, TaoTe Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TEST_TASK_PRIORITY_H
#define TEST_TASK_PRIORITY_H

#include "FreeRTOS.h"
#include "task.h"

/* 映射统一返回码 */
#ifndef OK
#define OK      pdPASS
#endif
#ifndef ERROR
#define ERROR   pdFAIL
#endif

typedef TaskHandle_t TASK_ID;

/* 启动优先级测试 */
void vStartTaskPriorityTest(void);

/* 封装的优先级设置API（带安全检查） */
BaseType_t OSTaskPrioritySetSafe(TASK_ID tid, UBaseType_t newPriority);

#endif /* TEST_TASK_PRIORITY_H */
