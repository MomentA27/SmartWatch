//
// Created by 35540 on 2026/5/14.
//
#include "DWT_delay.h"



/**
 * @brief 初始化 DWT 延时功能
 */
void DWT_Delay_Init(void) {
  /* 1. 使能调试跟踪 (DEMCR.TRCENA = 1) */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // ✅ 这些宏和结构体现在由 CMSIS 提供

  /* 2. 清零周期计数器 */
  DWT->CYCCNT = 0U;

  /* 3. 启动周期计数器 (DWT_CTRL.CYCCNTENA = 1) */
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief 微秒级阻塞延时
 */
void DWT_Delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * CPU_FREQ_MHZ;   /* 需要等待的 CPU 周期数 */
  uint32_t target = start + ticks;

  if (target > start) {
    /* 无溢出：直接等待达到目标值 */
    while (DWT->CYCCNT < target);
  } else {
    /* 溢出情况：先等待计数器翻转，再等待达到目标值 */
    while (DWT->CYCCNT >= start);
    while (DWT->CYCCNT < target);
  }
}

/**
 * @brief 毫秒级阻塞延时
 */
void DWT_Delay_ms(uint32_t ms)
{
  DWT_Delay_us(ms * 1000UL);
}
