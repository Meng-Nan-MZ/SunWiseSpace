/* 用于获取当前时间 */

#ifndef U_TIME_H
#define U_TIME_H

#include <stdint.h>

#define FREQUENCY 500000000 // CPU 频率 500MHz

/* 获取当前 cpu 周期 */
static uint64_t get_cpu_cycle() {
    uint32_t cycleh;
    uint32_t cycle;
    __asm__ volatile (
        "csrr    %1, cycleh\n"
        "csrr    %0, cycle\n"
        "csrr    %1, cycleh\n"
        : "=r" (cycle), "=r" (cycleh)
    );
    return ((uint64_t)cycleh << 32) | cycle;
}

/* 转换 cpu 周期到微秒 */
inline static uint64_t cycle_to_us(uint64_t cycle) {
    return cycle / (FREQUENCY / 1000000);
}

/* 转换 cpu 周期到纳秒 */
inline static uint64_t cycle_to_ns(uint64_t cycle) {
    return cycle * (1000000000 / FREQUENCY);   // 500MHz 时钟周期为 2ns
}

#endif
