/*
 * Copyright (c) 2020-2023, TaoTe Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author          Notes
 * 2025-01-XX   YourName        Add suspend/resume test functions
 */

#ifndef TEST_SUSPEND_RESUME_H
#define TEST_SUSPEND_RESUME_H

#include "FreeRTOS.h"
#include "task.h"

/* 说明：本测试依赖 INCLUDE_vTaskSuspend=1，请在 FreeRTOSConfig.h 中开启。*/

/* 类型与返回值定义 */
typedef TaskHandle_t TASK_ID;

#ifndef OK
#define OK      pdPASS
#endif
#ifndef ERROR
#define ERROR   pdFAIL
#endif

/* 测试主函数 */
void vStartSuspendResumeTest(void);

/* 封装的任务管理API（返回FreeRTOS风格状态码） */
BaseType_t OSTaskSuspend(TASK_ID tid);
BaseType_t OSTaskResume(TASK_ID tid);

/* 测试结果统计结构 */
typedef struct {
    uint32_t ulTotalTests;
    uint32_t ulPassedTests;
    uint32_t ulFailedTests;
    uint32_t ulSkippedTests;
} TestStatistics_t;

/* 测试用例ID枚举 */
typedef enum {
    TEST_TASK_CREATE_VALID = 0,
    TEST_TASK_CREATE_INVALID_PRIO,
    TEST_TASK_CREATE_HUGE_STACK,
    TEST_TASK_DELETE_VALID,
    TEST_TASK_DELETE_INVALID,
    TEST_TASK_DELETE_IDLE,
    TEST_TASK_SUSPEND_VALID,
    TEST_TASK_SUSPEND_INVALID,
    TEST_TASK_SUSPEND_IDLE,
    TEST_TASK_RESUME_VALID,
    TEST_TASK_RESUME_INVALID,
    TEST_TASK_MAX
} TestCaseId_e;

/* 测试状态枚举 */
typedef enum {
    TEST_STATUS_NOT_RUN = 0,
    TEST_STATUS_RUNNING,
    TEST_STATUS_PASSED,
    TEST_STATUS_FAILED,
    TEST_STATUS_SKIPPED
} TestStatus_e;

#endif /* TEST_SUSPEND_RESUME_H */