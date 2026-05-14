//
// Created by 35540 on 2026/4/28.
//

#ifndef SMARTWATCH_STM32F4_OS_MUTEX_PORT_H
#define SMARTWATCH_STM32F4_OS_MUTEX_PORT_H
#include <stdbool.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"
typedef void* os_mutex_handler_t;

bool os_mutex_create(os_mutex_handler_t *mutex);
bool os_mutex_take(os_mutex_handler_t mutex, uint32_t timeout);
bool os_mutex_give(os_mutex_handler_t mutex);
bool os_mutex_delete(os_mutex_handler_t mutex);
#endif //SMARTWATCH_STM32F4_OS_MUTEX_PORT_H