//
// Created by 35540 on 2026/4/28.
//

#ifndef SMARTWATCH_STM32F4_OS_MUTEX_PORT_H
#define SMARTWATCH_STM32F4_OS_MUTEX_PORT_H
#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"
typedef void* os_mutex_handler_t;

uint8_t os_mutex_create(os_mutex_handler_t *mutex);
uint8_t os_mutex_take(os_mutex_handler_t mutex, uint32_t timeout);
uint8_t os_mutex_give(os_mutex_handler_t mutex);
#endif //SMARTWATCH_STM32F4_OS_MUTEX_PORT_H