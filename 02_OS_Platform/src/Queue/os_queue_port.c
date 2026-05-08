//
// Created by 35540 on 2026/4/28.
//
/**
 * @file platform_os.c
 * @brief 操作系统抽象层的 FreeRTOS 实现后端
 * @note  脏活累活都在这里，上层业务绝对不要 include FreeRTOS 的头文件
 */

#include "os_queue_port.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// 为了代码可读性，内部定义统一的返回状态
#define OS_RET_OK     0
#define OS_RET_ERR    1

//---------------------------------------------------------------------------
// 队列管理
//---------------------------------------------------------------------------

uint8_t os_queue_create(os_queue_handler_t *queue, uint32_t msg_size, uint32_t queue_len)
{
    // 调用 FreeRTOS 原生 API 创建队列
    QueueHandle_t h_queue = xQueueCreate(queue_len, msg_size);

    if (h_queue == NULL) {
        return OS_RET_ERR;
    }

    // 将 FreeRTOS 的句柄强转为我们的抽象类型 void*
    *queue = (os_queue_handler_t)h_queue;
    return OS_RET_OK;
}

uint8_t os_queue_put(os_queue_handler_t queue, void *msg, uint32_t timeout)
{
    // 注意：FreeRTOS 的时间参数是 Tick，需要用 pdMS_TO_TICKS 转换毫秒
    TickType_t ticks = (timeout == 0xFFFFFFFFUL) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

    // 强转回 FreeRTOS 句柄调用原生 API
    if (xQueueSend((QueueHandle_t)queue, msg, ticks) != pdTRUE) {
        return OS_RET_ERR;
    }
    return OS_RET_OK;
}

uint8_t os_queue_get(os_queue_handler_t queue, void *msg, uint32_t timeout)
{
    TickType_t ticks = (timeout == 0xFFFFFFFFUL) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

    if (xQueueReceive((QueueHandle_t)queue, msg, ticks) != pdTRUE) {
        return OS_RET_ERR;
    }
    return OS_RET_OK;
}