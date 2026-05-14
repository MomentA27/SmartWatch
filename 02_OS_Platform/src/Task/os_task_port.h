//
// Created by 35540 on 2026/4/28.
//

#ifndef SMARTWATCH_STM32F4_OS_TASK_PORT_H
#define SMARTWATCH_STM32F4_OS_TASK_PORT_H
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include <stdbool.h>
// 定义平台无关的任务句柄
typedef void* os_task_handler_t;

// 定义平台无关的任务函数指针签名 (FreeRTOS 标准签名就是 void func(void *))
typedef void (*os_task_func_t)(void *);

bool os_task_create(
    const char *task_name,
    os_task_func_t func_pointer,
    uint16_t stack_size,
    void *argument,
    uint32_t priority,
    os_task_handler_t *task_handle
);
bool os_task_delete(os_task_handler_t task_handle);
bool os_task_delay(uint32_t ms);
os_task_handler_t os_task_get_handle(void);
#endif //SMARTWATCH_STM32F4_OS_TASK_PORT_H