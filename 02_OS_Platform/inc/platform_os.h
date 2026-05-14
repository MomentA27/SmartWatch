//
// Created by 35540 on 2026/4/28.
//

#ifndef SMARTWATCH_STM32F4_PLATFORM_OS_H
#define SMARTWATCH_STM32F4_PLATFORM_OS_H
#include "os_mutex_port.h"
#include "os_queue_port.h"
#include "os_task_port.h"
#include "os_sema_port.h"
#include "os_event_port.h"
#define OS_MAX_DELAY 0xFFFFFFFF
/*========== 系统相关接口==============*/
void os_enter_critical(void);
void os_exit_critical(void);
/*========== 时间相关接口==============*/
void os_delay_ms(uint32_t ms);
uint32_t os_get_tick_ms(void);
/*========== 任务相关接口==============*/
bool os_task_create(
    const char *task_name,
    os_task_func_t func_pointer, // 【改】抽象的函数指针
    uint16_t stack_size,         // 标准类型
    void *argument,
    uint32_t priority,           // 标准类型
    os_task_handler_t *task_handle       // 【改】抽象的句柄指针
);
bool os_task_delete(os_task_handler_t task_handle);
/*========== 互斥锁相关接口==============*/
bool os_mutex_create(os_mutex_handler_t *mutex);
bool os_mutex_take(os_mutex_handler_t mutex, uint32_t timeout);
bool os_mutex_give(os_mutex_handler_t mutex);
os_task_handler_t os_task_get_handle(void);
/*========== 队列相关接口==============*/
bool os_queue_create(os_queue_handler_t *queue, uint32_t msg_size, uint32_t queue_len);
bool os_queue_put(os_queue_handler_t queue, void *msg, uint32_t timeout);
bool os_queue_get(os_queue_handler_t queue, void *msg, uint32_t timeout);
bool os_queue_delete(os_queue_handler_t queue);
bool os_queue_put_from_isr(os_queue_handler_t queue, void *msg);;
/*========== 信号量相关接口==============*/
bool os_sema_create(os_sema_handler_t *sema, bool is_binary, uint32_t max_count, uint32_t init_count);
bool os_sema_take(os_sema_handler_t sema, uint32_t timeout);
bool os_sema_give(os_sema_handler_t sema);
bool os_mutex_delete(os_mutex_handler_t mutex);;
/*========== 事件组相关接口==============*/
bool os_event_create(os_event_hanlder_t *event_handle);
bool os_event_wait(os_event_hanlder_t event, uint32_t wait_bits, bool wait_all, bool auto_clear, uint32_t timeout, uint32_t *out_active_bits);
bool os_event_set(os_event_hanlder_t event, uint32_t set_bits);
bool os_event_clear(os_event_hanlder_t event, uint32_t clear_bits);
bool os_event_delete(os_event_hanlder_t event);
#endif //SMARTWATCH_STM32F4_PLATFORM_OS_H