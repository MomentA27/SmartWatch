//
// Created by 35540 on 2026/4/29.
//

#ifndef SMARTWATCH_STM32F4_OS_SYSTEM_PORT_H
#define SMARTWATCH_STM32F4_OS_SYSTEM_PORT_H

#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief 进入临界区（关闭任务调度，在Cortex-M内核下通常会关闭全局中断）
 * @note  必须与 os_exit_critical() 严格配对使用，且不能跨越函数边界！
 */
void os_enter_critical(void);

/**
 * @brief 退出临界区（恢复任务调度和中断状态）
 */
void os_exit_critical(void);

// ================= 2. 时间管理 =================

/**
 * @brief 任务级延时（让出 CPU 给其他同优先级或低优先级任务）
 * @param ms 延时的毫秒数
 */
void os_delay_ms(uint32_t ms);

/**
 * @brief 获取系统从启动到现在运行的毫秒数
 * @return 系统运行时间
 */
uint32_t os_get_tick_ms(void);
#endif //SMARTWATCH_STM32F4_OS_SYSTEM_PORT_H