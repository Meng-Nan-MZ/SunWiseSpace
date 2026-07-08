/*
 * Task state transition test (5.1.4)
 */

#ifndef TEST_TASK_STATE_H
#define TEST_TASK_STATE_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#ifndef OK
#define OK      pdPASS
#endif
#ifndef ERROR
#define ERROR   pdFAIL
#endif

typedef TaskHandle_t TASK_ID;

void vStartTaskStateTest(void);

#endif /* TEST_TASK_STATE_H */
