#ifndef PRECISION_TIMER_H
#define PRECISION_TIMER_H

#include <stdint.h>
#include <stdio.h>

/**
 * ARM64 System Counter 高精度计时器 (FreeRTOS 版本)
 * 
 * 使用 ARM Generic Timer 的系统寄存器直接读取硬件计数器
 * 避免系统调用开销,提供纳秒级精度
 * 
 * 相关寄存器：
 * - CNTFRQ_EL0: 计数器频率寄存器（飞腾派为 50MHz）
 * - CNTVCT_EL0: 虚拟计数器寄存器（单调递增的 64 位计数值）
 * 
 * 注意：需要 Hypervisor 允许 Guest 访问这些 EL0 系统寄存器
 */

/**
 * 读取 ARM Generic Timer 频率
 * 
 * 从 CNTFRQ_EL0 系统寄存器读取计数器频率
 * 飞腾派开发板：50MHz (0x2faf080 = 50,000,000 Hz)
 * 
 * @return 计数器频率 (Hz)
 */
static inline uint64_t get_cntfrq(void) {
    uint64_t freq;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r" (freq));
    return freq;
}

/**
 * 读取 ARM Generic Timer 当前计数值
 * 
 * 读取虚拟计数器 CNTVCT_EL0
 * 虚拟计数器在虚拟化环境中通常更容易访问
 * 
 * @return 当前计数值 (ticks)
 */
static inline uint64_t get_cntpct(void) {
    uint64_t count;
    __asm__ volatile(
        "isb\n\t"                    // 指令同步屏障
        "mrs %0, cntvct_el0"         // 读取虚拟计数器
        : "=r" (count)
        :
        : "memory"
    );
    return count;
}

/**
 * 读取 ARM Generic Timer 当前计数值 (32位版本)
 * 
 * 只返回低 32 位，用于节省空间
 * 注意：50MHz 时每 85.9 秒溢出一次
 * 
 * @return 当前计数值低 32 位 (ticks)
 */
static inline uint32_t get_cntpct_32(void) {
    return (uint32_t)get_cntpct();
}

/**
 * 将 ticks 差值转换为微秒 (μs) - 支持 u32 计数器溢出
 * 
 * 计算公式：μs = (ticks * 1,000,000) / freq
 * 
 * 自动处理 u32 计数器溢出：
 * - 如果 end >= start：正常计算
 * - 如果 end < start：假设溢出一次
 * 
 * @param ticks_start 起始计数值 (u32)
 * @param ticks_end   结束计数值 (u32)
 * @param freq        计数器频率 (Hz)
 * @return 时间差（微秒）
 */
static inline uint64_t ticks_to_us_32(uint32_t ticks_start, uint32_t ticks_end, uint64_t freq) {
    uint64_t ticks_diff;
    
    if (ticks_end >= ticks_start) {
        /* 正常情况：未溢出 */
        ticks_diff = ticks_end - ticks_start;
    } else {
        /* 溢出情况：假设溢出一次 (2^32 = 4294967296) */
        ticks_diff = (0x100000000ULL - ticks_start) + ticks_end;
    }
    
    return (ticks_diff * 1000000ULL) / freq;
}

/**
 * 将 ticks 差值转换为纳秒 (ns) - 支持 u32 计数器溢出
 * 
 * @param ticks_start 起始计数值 (u32)
 * @param ticks_end   结束计数值 (u32)
 * @param freq        计数器频率 (Hz)
 * @return 时间差（纳秒）
 */
static inline uint64_t ticks_to_ns_32(uint32_t ticks_start, uint32_t ticks_end, uint64_t freq) {
    uint64_t ticks_diff;
    
    if (ticks_end >= ticks_start) {
        ticks_diff = ticks_end - ticks_start;
    } else {
        ticks_diff = (0x100000000ULL - ticks_start) + ticks_end;
    }
    
    return (ticks_diff * 1000000000ULL) / freq;
}

/**
 * 将 ticks 差值转换为微秒 (μs) - u64 版本（原始）
 * 
 * 计算公式：μs = (ticks * 1,000,000) / freq
 * 
 * 示例（飞腾派 50MHz）：
 * - 1 tick = 20 ns
 * - 50 ticks = 1 μs
 * - 50,000 ticks = 1 ms
 * 
 * @param ticks_start 起始计数值
 * @param ticks_end   结束计数值
 * @param freq        计数器频率 (Hz)
 * @return 时间差（微秒）
 */
static inline uint64_t ticks_to_us(uint64_t ticks_start, uint64_t ticks_end, uint64_t freq) {
    uint64_t ticks_diff = ticks_end - ticks_start;
    // 使用 64 位运算避免溢出
    // (ticks * 1000000) / freq
    return (ticks_diff * 1000000ULL) / freq;
}

/**
 * 将 ticks 差值转换为纳秒 (ns)
 * 
 * 计算公式：ns = (ticks * 1,000,000,000) / freq
 * 
 * @param ticks_start 起始计数值
 * @param ticks_end   结束计数值
 * @param freq        计数器频率 (Hz)
 * @return 时间差（纳秒）
 */
static inline uint64_t ticks_to_ns(uint64_t ticks_start, uint64_t ticks_end, uint64_t freq) {
    uint64_t ticks_diff = ticks_end - ticks_start;
    return (ticks_diff * 1000000000ULL) / freq;
}

/**
 * 将 ticks 差值转换为毫秒 (ms)
 * 
 * 计算公式：ms = (ticks * 1,000) / freq
 * 
 * @param ticks_start 起始计数值
 * @param ticks_end   结束计数值
 * @param freq        计数器频率 (Hz)
 * @return 时间差（毫秒）
 */
static inline uint64_t ticks_to_ms(uint64_t ticks_start, uint64_t ticks_end, uint64_t freq) {
    uint64_t ticks_diff = ticks_end - ticks_start;
    return (ticks_diff * 1000ULL) / freq;
}

/**
 * 将 ticks 差值转换为秒 (s)
 * 
 * 计算公式：s = ticks / freq
 * 
 * @param ticks_start 起始计数值
 * @param ticks_end   结束计数值
 * @param freq        计数器频率 (Hz)
 * @return 时间差（秒）
 */
static inline double ticks_to_seconds(uint64_t ticks_start, uint64_t ticks_end, uint64_t freq) {
    uint64_t ticks_diff = ticks_end - ticks_start;
    return (double)ticks_diff / (double)freq;
}

/**
 * 获取计数器精度信息
 * 
 * @param freq 计数器频率
 * @return 每个 tick 对应的纳秒数
 */
static inline double get_timer_precision_ns(uint64_t freq) {
    return 1000000000.0 / (double)freq;
}

/**
 * 打印计时器信息（调试用）
 */
static inline void print_timer_info(void) {
    uint64_t freq = get_cntfrq();
    double precision_ns = get_timer_precision_ns(freq);
    
    printf("=== ARM64 System Counter Information ===\r\n");
    printf("Frequency (CNTFRQ_EL0): %lu Hz (%.2f MHz)\r\n", 
           (unsigned long)freq, freq / 1000000.0);
    printf("Timer Precision: %.2f ns per tick\r\n", precision_ns);
    printf("Expected Frequency (Phytium Pi): 50,000,000 Hz (50 MHz)\r\n");
    printf("========================================\r\n");
}

#endif // PRECISION_TIMER_H
