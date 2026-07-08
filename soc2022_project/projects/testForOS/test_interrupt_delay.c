#include "test_interrupt_delay.h"
#include "xprintf.h"
#include "XYR3201.h"
#include "drv_timer.h"

// 测试配置
#define INTERRUPT_DELAY_TEST_COUNT    10
#define INTERRUPT_DELAY_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2)
#define TEST_PRIORITY                 (tskIDLE_PRIORITY + 2)

// 全局变量
static volatile uint32_t g_interruptDelays[INTERRUPT_DELAY_TEST_COUNT];
static volatile uint32_t g_currentTest = 0;
static volatile uint32_t g_testComplete = 0;
static volatile uint32_t g_t1 = 0;  // 触发中断前的时间戳
static volatile uint32_t g_t2 = 0;  // 中断处理函数开始的时间戳

// CPU周期计数函数
static inline uint32_t get_cpu_cycles(void)
{
    uint32_t cycles;
    __asm__ volatile ("rdcycle %0" : "=r" (cycles));
    return cycles;
}

// 自定义定时器中断处理函数
void TestTimer_IRQHandler(void)
{
    // 记录中断处理函数开始的时间戳t2
    g_t2 = get_cpu_cycles();
    
    // 计算中断延迟时间（减去定时器间隔100us）
    uint32_t raw_delay = g_t2 - g_t1;
    uint32_t timer_interval_cycles = 50000;  // 100us = 50,000 cycles (500MHz)
    uint32_t interrupt_delay = raw_delay - timer_interval_cycles;
    
    g_interruptDelays[g_currentTest] = interrupt_delay;
    
    xprintf("第%lu次中断延迟: %lu CPU cycles (原始: %lu)\r\n", 
            g_currentTest + 1, interrupt_delay, raw_delay);
    
    // 清除定时器中断
    HW_TIMER_ClearInterrupt(TIMER0, eTimerChannel_0);
    
    // 标记当前测试完成
    g_testComplete = 1;
}

// 测试任务
static void vInterruptDelayTestTask(void *pvParameters)
{
    xprintf("========== 中断延迟时间测试 ==========\r\n");
    xprintf("测试说明: 测量从中断到来到处理函数开始执行的时间\r\n");
    xprintf("测试次数: %d\r\n", INTERRUPT_DELAY_TEST_COUNT);
    
    // 配置定时器
    DeviceTimerConfig_t timerConfig = {
        .mode = eTimerMode_UserDefined,
        .tick1us = 1000000,      // 1MHz
        .interval = 100,          // 100us间隔（更短，减少延迟）
        .int_enable = 1,          // 使能中断
        .user_data = 0
    };
    
    // 初始化定时器
    if (HW_TIMER_InitConfig(TIMER0, eTimerChannel_0, &timerConfig) != 0)
    {
        xprintf("错误: 定时器初始化失败\r\n");
        vTaskDelete(NULL);
        return;
    }
    
    // 注册自定义中断处理函数
    drv_irq_register(TIMER0_IRQn, TestTimer_IRQHandler);
    
    // 使能定时器中断
    drv_irq_enable(TIMER0_IRQn);
    
    // 执行10次测试
    for (uint32_t test = 0; test < INTERRUPT_DELAY_TEST_COUNT; test++)
    {
        g_currentTest = test;
        g_testComplete = 0;
        
        g_t1 = get_cpu_cycles();//放位置1还是位置2？
        // 启动定时器（100us后触发中断）
        HW_TIMER_Start(TIMER0, eTimerChannel_0);
        
        //g_t1 = get_cpu_cycles();//位置2
        // 立即记录启动时间作为t1
        
        
        // 等待中断处理完成
        while (g_testComplete == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        
        // 停止定时器
        HW_TIMER_Stop(TIMER0, eTimerChannel_0);
        
        // 短暂延迟
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // 禁用定时器中断
    drv_irq_disable(TIMER0_IRQn);
    
    // 计算统计结果
    uint32_t total_cycles = 0;
    uint32_t max_cycles = 0;
    uint32_t min_cycles = 0xFFFFFFFF;
    
    for (uint32_t i = 0; i < INTERRUPT_DELAY_TEST_COUNT; i++)
    {
        total_cycles += g_interruptDelays[i];
        if (g_interruptDelays[i] > max_cycles) max_cycles = g_interruptDelays[i];
        if (g_interruptDelays[i] < min_cycles) min_cycles = g_interruptDelays[i];
    }
    
    uint32_t avg_cycles = total_cycles / INTERRUPT_DELAY_TEST_COUNT;
    
    xprintf("\r\n========== 中断延迟时间统计 ==========\r\n");
    xprintf("测试次数: %lu\r\n", INTERRUPT_DELAY_TEST_COUNT);
    xprintf("平均中断延迟: %lu CPU cycles\r\n", avg_cycles);
    xprintf("最大中断延迟: %lu CPU cycles\r\n", max_cycles);
    xprintf("最小中断延迟: %lu CPU cycles\r\n", min_cycles);
    
    xprintf("========== 测试完成 ==========\r\n\r\n");
    
    // 删除任务
    vTaskDelete(NULL);
}

// 启动测试
void vStartInterruptDelayTest(void)
{
    // 重置测试数据
    g_currentTest = 0;
    g_testComplete = 0;
    for (uint32_t i = 0; i < INTERRUPT_DELAY_TEST_COUNT; i++)
    {
        g_interruptDelays[i] = 0;
    }
    
    // 创建测试任务
    if (xTaskCreate(vInterruptDelayTestTask, "InterruptDelayTest", INTERRUPT_DELAY_STACK_SIZE, NULL, TEST_PRIORITY, NULL) != pdPASS)
    {
        xprintf("错误: 无法创建中断延迟测试任务\r\n");
        return;
    }
    
    xprintf("中断延迟时间测试任务创建成功\r\n");
}
