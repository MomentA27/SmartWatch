//
// Created by 35540 on 2026/4/28.
//

#ifndef SMARTWATCH_STM32F4_OS_QUEUE_PORT_H
#define SMARTWATCH_STM32F4_OS_QUEUE_PORT_H
#include <stdint.h>
#include <stdbool.h>
#include "main.h"

// 1. 类型抹除：用 void* 隐藏底层 RTOS 类型
typedef void* os_queue_handler_t;

// 2. 行为抽象：统一的全局 API
bool os_queue_create(os_queue_handler_t *queue, uint32_t msg_size, uint32_t queue_len);
bool os_queue_put(os_queue_handler_t queue, void *msg, uint32_t timeout);
bool os_queue_get(os_queue_handler_t queue, void *msg, uint32_t timeout);
bool os_queue_delete(os_queue_handler_t queue);
bool os_queue_put_from_isr(os_queue_handler_t queue, void *msg);

#endif //SMARTWATCH_STM32F4_OS_QUEUE_PORT_H