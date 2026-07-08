#include "test_sem_take_nowait.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "xprintf.h"

static SemaphoreHandle_t sem = NULL;

// TaskA: 尝试阻塞获取信号量
static void vSemTakeWaitTaskA(void *pvParameters) {
    xprintf("[TaskA] 尝试以阻塞方式获取信号量...\n");
    TickType_t tick_before = xTaskGetTickCount();
    BaseType_t ret = xSemaphoreTake(sem, portMAX_DELAY);
    TickType_t tick_after = xTaskGetTickCount();
    if (ret == pdTRUE) {
        xprintf("[TaskA] 阻塞获取成功，阻塞时长: %lu ticks\n", (unsigned long)(tick_after - tick_before));
    } else {
        xprintf("[TaskA] 阻塞获取失败。\n");
    }
    vTaskDelete(NULL);
}

// TaskB: 延迟后释放信号量
static void vSemTakeWaitTaskB(void *pvParameters) {
    vTaskDelay(100); // 延迟100 ticks，确保TaskA已阻塞
    xprintf("[TaskB] 释放信号量。\n");
    xSemaphoreGive(sem);
    vTaskDelete(NULL);
}

void vStartSemTakeWaitTest(void) {
    xprintf("========== 阻塞方式获取信号量 ==========");
    sem = xSemaphoreCreateBinary();
    if (sem) {
        // 不提前give，确保TaskA会阻塞
        xTaskCreate(vSemTakeWaitTaskA, "SemTakeWaitA", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
        xTaskCreate(vSemTakeWaitTaskB, "SemTakeWaitB", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
        // 任务结束后由任务自身删除信号量
    } else {
        xprintf("[测试失败] 无法创建信号量。\n");
    }
}
