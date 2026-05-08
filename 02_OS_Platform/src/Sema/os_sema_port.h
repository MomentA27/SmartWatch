//
// Created by 35540 on 2026/4/29.
//

#ifndef SMARTWATCH_STM32F4_OS_SEMA_PORT_H
#define SMARTWATCH_STM32F4_OS_SEMA_PORT_H
#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"
#include <stdbool.h>

// 1. 类型抹除：用 void* 隐藏 FreeRTOS 的 SemaphoreHandle_t
typedef void* os_sema_handler_t;


/**
 * @brief 创建信号量
 * @param sema     [out] 信号量句柄的指针（用于传出创建的句柄）
 * @param is_binary [in]  true: 创建二值信号量；false: 创建计数信号量
 * @param max_count [in]  计数信号量的最大计数值（创建二值信号量时传 0 即可）
 * @param init_count[in]  计数信号量的初始计数值（创建二值信号量时传 0 即可）
 * @return SUCCESS 或 ERROR
 */
uint8_t os_sema_create(os_sema_handler_t *sema, bool is_binary, uint32_t max_count, uint32_t init_count);

/**
 * @brief 获取信号量 (Wait/Pend)
 * @param sema    [in] 信号量句柄
 * @param timeout [in] 超时时间（毫秒），传 0xFFFFFFFF 表示永久等待
 * @return SUCCESS 或 ERROR
 */
uint8_t os_sema_take(os_sema_handler_t sema, uint32_t timeout);

/**
 * @brief 释放信号量 (Post/Give)
 * @param sema    [in] 信号量句柄
 * @return SUCCESS 或 ERROR
 */
uint8_t os_sema_give(os_sema_handler_t sema);
#endif //SMARTWATCH_STM32F4_OS_SEMA_PORT_H