#include "XYR3201.h"
#include "drv_common.h"
#include "sysclock.h"
#include "xprintf.h"
#include "xshell.h"
#include "utils/u_macro.h"

/**========宏开关定义区间========**/
#define USE_OS_FREERTOS // 启用 FreeRTOS 操作系统

#define TEST_TASK                0         // 7.1.1 任务管理功能测试
#define TEST_SUSPEND_RESUME      0         // 7.1.2 任务挂起和恢复功能测试
#define TEST_PRIORITY            0         // 7.1.3 任务优先级功能测试
#define TEST_STATE               0         // 7.1.4 任务状态管理测试
#define TEST_SCHED               0         // 7.1.5 调度器策略测试
#define TEST_SAME_PRIORITY       0         // 7.1.6 同优先级任务调度测试
#define TEST_SEM_CREATE          0         // 7.1.7 信号量创建测试
#define TEST_SEM_DELETE          0         // 7.1.8 信号量删除测试
#define TEST_SEM_TAKE_NOWAIT     0         // 7.1.9 信号量非阻塞获取测试
#define TEST_SEM_TAKE_WAIT       0         // 7.1.10 信号量阻塞获取测试
#define TEST_SEM_TASK_TIMEOUT    0         // 7.1.11 信号量任务超时测试
#define TEST_MUTEX               0         // 7.1.12 - 7.1.15 互斥锁功能测试（大部分并入信号量测试中）
#define TEST_MQ_SYNC             0         // 7.1.16 消息队列同步通信测试
#define TEST_MQ_ASYNC            0         // 7.1.17 消息队列异步通信测试
#define TEST_MQ_CREATE_DELETE    0         // 7.1.18 消息队列创建和删除测试
#define TEST_MQ_SEND_BLOCKING    0         // 7.1.19 阻塞方式发送消息队列测试
#define TEST_MQ_SEND_NONBLOCKING 0         // 7.1.20 非阻塞方式发送消息队列测试
#define TEST_TICK                0         // 7.1.23/24 系统时钟节拍测试
#define TEST_WDT                 0         // 7.1.25 看门狗功能测试
#define TEST_MEMORY              0         // 7.1.26 内存管理功能测试

/**======== time test ========**/

#define TEST_TASK_SWITCH         0         // 7.2.1/2 任务切换时间测试
#define TEST_SEM_CREATE_TIME     0         // 7.2.3 信号量创建时间测试
#define TEST_SEM_GIVE_TIME       0         // 7.2.4 信号量释放时间测试
#define TEST_MUTEX_CREATE_TIME   0         // 7.2.5 互斥锁创建时间测试
#define TEST_MUTEX_RELEASE_DELAY 0         // 7.2.6 互斥锁释放延迟测试

#define TEST_MQ_CREATE_TIME      0         // 7.2.8 消息队列创建时间测试
#define TEST_MQ_RECV_TIME        0         // 7.2.9 消息队列接收时间测试
#define TEST_MQ_SEND_TIME        0         // 7.2.10 消息队列发送时间测试
#define TEST_ALARM_CREATE_TIME   0         // 7.2.11 alarm创建时间测试
#define TEST_ALARM_START_TIME    0         // 7.2.12 alarm启动时间测试
#define TEST_TASK_CREATE_TIME    0         // 7.2.13 任务创建等相关时间测试
#define TEST_TASK_SUSPEND_TIME   0         // 7.2.14 任务挂起等相关时间测试
#define TEST_TASK_PRIORITY_TIME  0         // 7.2.15 任务优先级等相关时间测试

#define TEST_INTERRUPT_DELAY     0         // 7.3.2 中断延迟时间
#define TEST_PREEMPTION_TIME     0         // 7.3.3 抢占时间，高优先级任务从变为就绪态到开始执行的时间
#define TEST_STABILITY           1         // 7.3.4任务切换时间稳定性测试

#define TEST_RESUME_DELAY        0         // 7.4.2 解挂任务延迟时间
#define TEST_DELETE_DELAY        0         // 7.4.3 删除任务延迟时间
#define TEST_SEM_TAKE_DELAY      0         // 7.4.5 申请信号量延迟时间
#define TEST_SEM_GIVE_DELAY      0         // 7.4.6 释放信号量延迟时间
#define TEST_MSG_SEND_DELAY      0         // 7.4.7 发送消息延迟时间
#define TEST_MSG_RECV_DELAY      0         // 7.4.8 接收消息延迟时间


#ifdef USE_OS_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif // #ifdef USE_OS_FREERTOS

/* lwIP core includes */
#include "lwip/opt.h"

#include "lwip/err.h"
#include "lwip/netif.h"

#include "lwip/init.h"
#include "lwip/ip4_addr.h"

#include "lwip/sys.h"
#include "lwip/tcpip.h"

/* ----- ----- ----- ----- USER TASK DEFINE START ----- ----- ----- ----- */

#ifdef USE_OS_FREERTOS

#ifndef OS_WAIT_FOREVER
#define OS_WAIT_FOREVER ((TickType_t)(~0))
#endif

#include "testForOS/test_task.h"
#include "testForOS/test_suspend_resume.h"
#include "testForOS/test_priority.h"
#include "testForOS/test_state.h"
#include "testForOS/test_sched.h"
// #include "testForOS/test_same_priority.h"
#include "testForOS/test_sem_create.h"
#include "testForOS/test_sem_delete.h"
#include "testForOS/test_sem_take_nowait.h"
#include "testForOS/test_sem_take_wait.h"
#include "testForOS/test_sem_take_timeout.h"
#include "testForOS/test_mutex.h"
#include "testForOS/test_mq.h"
#include "testForOS/test_mq_send.h"
#include "testForOS/test_tick.h"
#include "testForOS/test_wdt.h"
#include "testForOS/test_memory.h"

#include "testForOS/test_task_switch.h"
#include "testForOS/test_stability.h"
#include "testForOS/test_sem_create_time.h"
#include "testForOS/test_sem_give_time.h"
#include "testForOS/test_mutex_create_time.h"
#include "testForOS/test_mutex_release_delay.h"
#include "testForOS/test_mutex_request_time.h"
#include "testForOS/test_mq_create_time.h"
#include "testForOS/test_mq_recv_time.h"
#include "testForOS/test_mq_send_time.h"
#include "testForOS/test_alarm_create_time.h"
#include "testForOS/test_alarm_start_time.h"
#include "testForOS/test_task_create_time.h"
#include "testForOS/test_task_suspend_time.h"
#include "testForOS/test_task_priority_time.h"

#include "testForOS/test_interrupt_delay.h"
#include "testForOS/test_preemption_time.h"

#include "testForOS/test_resume_delay.h"
#include "testForOS/test_delete_delay.h"
#include "testForOS/test_sem_take_delay.h"
// #include "testForOS/test_sem_give_delay.h"
#include "testForOS/test_msg_send_delay.h"
#include "testForOS/test_msg_recv_delay.h"

void uTaskCreate(void) {
  IFDEF(TEST_TASK, vStartTaskManagementTest();)
  IFDEF(TEST_SUSPEND_RESUME, vStartSuspendResumeTest();)
  IFDEF(TEST_PRIORITY, vStartTaskPriorityTest();)
  IFDEF(TEST_STATE, vStartTaskStateTest();)
  IFDEF(TEST_SCHED, vStartSchedulerPolicyTest();)
  IFDEF(TEST_SAME_PRIORITY, vStartSamePriorityTest();)
  IFDEF(TEST_SEM_CREATE, vStartSemCreateTest();)
  IFDEF(TEST_SEM_DELETE, vStartSemDeleteTest();)
  IFDEF(TEST_SEM_TAKE_NOWAIT, vStartSemTakeNoWaitTest();)
  IFDEF(TEST_SEM_TAKE_WAIT, vStartSemTakeWaitTest();)
  IFDEF(TEST_SEM_TASK_TIMEOUT, vStartSemTakeTimeoutTest();)
  IFDEF(TEST_MUTEX, vStartMutexTest();)

  IFDEF(TEST_MQ, vStartMqAsyncTest();)
  IFDEF(TEST_MQ_SYNC, vStartMqSyncTest();)
  IFDEF(TEST_MQ_ASYNC, vStartMqAsyncTest();)
  IFDEF(TEST_MQ_CREATE_DELETE, vStartMqCreateDeleteTest();)
  IFDEF(TEST_MQ_SEND_BLOCKING, vStartMqBlockingSendTest();)
  IFDEF(TEST_MQ_SEND_NONBLOCKING, vStartMqNonBlockingSendTest();)
  IFDEF(TEST_TICK, vStartTickTest();)
  IFDEF(TEST_WDT, vStartWdtTest();)
  IFDEF(TEST_MEMORY, vStartMemoryTest();)

  IFDEF(TEST_TASK_SWITCH, vStartTaskSwitchTest();)
  IFDEF(TEST_STABILITY, vStartStabilityTest();)
  IFDEF(TEST_SEM_CREATE_TIME, vStartSemCreateTimeTest();)
  IFDEF(TEST_SEM_GIVE_TIME, vStartSemGiveTimeTest();)
  IFDEF(TEST_MUTEX_CREATE_TIME, vStartMutexCreateTimeTest();)
  IFDEF(TEST_MUTEX_RELEASE_DELAY, vStartMutexReleaseDelayTest();)
  IFDEF(TEST_MUTEX_REQUEST_TIME, vStartMutexRequestTimeTest();)
  IFDEF(TEST_MQ_CREATE_TIME, vStartMQCreateTimeTest();)
  IFDEF(TEST_MQ_RECV_TIME, vStartMQRecvTimeTest();)
  IFDEF(TEST_MQ_SEND_TIME, vStartMQSendTimeTest();)
  IFDEF(TEST_ALARM_CREATE_TIME, vStartAlarmCreateTimeTest();)
  IFDEF(TEST_ALARM_START_TIME, vStartAlarmStartTimeTest();)
  IFDEF(TEST_TASK_CREATE_TIME, vStartTaskCreateTimeTest();)
  IFDEF(TEST_TASK_SUSPEND_TIME, vStartTaskSuspendTimeTest();)
  IFDEF(TEST_TASK_PRIORITY_TIME, vStartTaskPriorityTimeTest();)

  IFDEF(TEST_INTERRUPT_DELAY, vStartInterruptDelayTest();)
  IFDEF(TEST_PREEMPTION_TIME, vStartPreemptionTimeTest();)

  IFDEF(TEST_RESUME_DELAY, vStartResumeDelayTest();)
  IFDEF(TEST_DELETE_DELAY, vStartDeleteDelayTest();)
  IFDEF(TEST_SEM_TAKE_DELAY, vStartSemTakeDelayTest();)
  IFDEF(TEST_SEM_GIVE_DELAY, vStartSemGiveDelayTest();)
  IFDEF(TEST_MSG_SEND_DELAY, vStartMsgSendDelayTest();)
  IFDEF(TEST_MSG_RECV_DELAY, vStartMsgRecvDelayTest();)
}

mTaskDefine(xshell_task) {
  for (;;) {
    char aszCommand[128];
    uint32_t ui32CommandLengh = Shell_GetCommand(aszCommand, ARRAY_SIZE(aszCommand));
    aszCommand[ui32CommandLengh] = 0;
    Shell_HandleCommand(aszCommand, ui32CommandLengh);
  }
}

mTaskDefine(iperf_tcp_task) {
  extern void iperf_tcp_server(void *pvParameters);
  iperf_tcp_server(NULL);
}

mTaskDefine(iperf_udp_task) {
  extern void iperf_udp_server(void *pvParameters);
  iperf_udp_server(NULL);
}

#endif // #ifdef USE_OS_FREERTOS

/* ----- ----- ----- ----- USER TASK DEFINE END   ----- ----- ----- ----- */

/* ----- ----- ----- ----- LwIP INIT DEFINE START ----- ----- ----- ----- */

static struct netif g_netif;

static err_t _ethernetif_init(struct netif *netif) {
  extern err_t ethernetif_init(struct netif * netif);
  return ethernetif_init(netif);
}

static err_t _tcpip_input(struct pbuf *p, struct netif *netif) {
  extern err_t tcpip_input(struct pbuf * p, struct netif * inp);
  return tcpip_input(p, netif);
}

void LwIP_Init(void) {
  ip4_addr_t ipaddr;
  ip4_addr_t netmask;
  ip4_addr_t gw;

  IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
  IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2,
           NETMASK_ADDR3);
  IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);

  tcpip_init(NULL, NULL);

  /* add the network interface (IPv4/IPv6) without RTOS */
  netif_add(&g_netif, &ipaddr, &netmask, &gw, NULL, (void *)_ethernetif_init,
            (void *)_tcpip_input);

  /* Registers the default network interface */
  netif_set_default(&g_netif);

  if (netif_is_link_up(&g_netif)) {
    netif_set_up(&g_netif);
    netif_set_link_up(&g_netif);
  } else {
    netif_set_down(&g_netif);
    netif_set_link_down(&g_netif);
  }

  if (!ip4_addr_isany(netif_ip4_addr((&g_netif)))) {
    xprintf("Static assigned IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&g_netif)));
  }
}

/* ----- ----- ----- ----- LwIP INIT DEFINE END   ----- ----- ----- ----- */

int main(void) {

  xprintf("Hello, world.\n");
  xprintf("build %s %s.\n", __DATE__, __TIME__);
  xprintf("\n");

  xprintf("apbg_clk_gate = 0x%x\n", REG_READ_32(0xd000004c));
  xprintf("apbg_resetn   = 0x%x\n", REG_READ_32(0xd0000050));

#ifdef USE_OS_FREERTOS
  portDISABLE_INTERRUPTS();

  // LwIP_Init();
  // 开启网络，若仅仅关注OS可以不开启

  uTaskCreate();
  vTaskStartScheduler();
#endif // #ifdef USE_OS_FREERTOS

  // 进入死循环
  for (;;) mdelay(100);
  return 0;
}
