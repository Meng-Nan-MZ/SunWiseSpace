#include "test_sem_give.h"
#include "xprintf.h"

/* 为避免与 test_sem_give_time.c 中的 vStartSemGiveTimeTest 重复定义，
 * 本文件仅保留统一对外接口 vStartSemGiveTest，并直接调用时间测试实现。 */

void vStartSemGiveTest(void)
{
    xprintf("[信号量释放测试] 进入统一包装接口 vStartSemGiveTest\n");
    vStartSemGiveTimeTest();
}


