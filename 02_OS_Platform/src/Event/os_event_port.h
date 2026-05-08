//
// Created by 35540 on 2026/4/29.
//

#ifndef SMARTWATCH_STM32F4_OS_EVENT_PORT_H
#define SMARTWATCH_STM32F4_OS_EVENT_PORT_H
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "FreeRTOS.h"
#include "event_groups.h"

typedef void* os_event_hanlder_t;

/**
 * @brief 等待事件位
 * @param event        [in]  事件组句柄
 * @param wait_bits    [in]  要等待的事件位掩码（如 0x03 表示等待 bit0 和 bit1）
 * @param wait_all     [in]  true: 等待 wait_bits 全部置位；false: 等待任意一个置位
 * @param auto_clear   [in]  true: 退出等待前自动清除置位的事件；false: 保持置位状态不自动清除
 * @param timeout      [in]  超时时间（毫秒），传 0xFFFFFFFF 表示永久等待
 * @param out_active_bits [out] 返回实际被置位的事件位（用于判断具体是哪个事件触发的）
 * @return SUCCESS 或 ERROR
 */
uint8_t os_event_wait(os_event_hanlder_t event, uint32_t wait_bits, bool wait_all, bool auto_clear, uint32_t timeout, uint32_t *out_active_bits);

/**
 * @brief 设置事件位（唤醒等待的任务）
 * @param event       [in] 事件组句柄
 * @param set_bits    [in] 要设置的事件位掩码
 * @return SUCCESS 或 ERROR
 */
uint8_t os_event_set(os_event_hanlder_t event, uint32_t set_bits);

/**
 * @brief 清除事件位
 * @param event       [in] 事件组句柄
 * @param clear_bits  [in] 要清除的事件位掩码
 * @return SUCCESS 或 ERROR
 */
uint8_t os_event_clear(os_event_hanlder_t event, uint32_t clear_bits);

/**
 * @brief 删除事件组
 * @param event       [in] 事件组句柄
 * @return SUCCESS 或 ERROR
 */
uint8_t os_event_delete(os_event_hanlder_t event);
#endif //SMARTWATCH_STM32F4_OS_EVENT_PORT_H