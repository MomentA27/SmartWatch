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


//---------------------------------------------------------------------------
// 队列管理
//---------------------------------------------------------------------------

bool os_queue_create(os_queue_handler_t *queue, uint32_t msg_size, uint32_t queue_len)
{
    // 调用 FreeRTOS 原生 API 创建队列
    QueueHandle_t h_queue = xQueueCreate(queue_len, msg_size);

    if (h_queue == NULL) {
        return false;
    }

    // 将 FreeRTOS 的句柄强转为我们的抽象类型 void*
    *queue = (os_queue_handler_t)h_queue;
    return true;
}

bool os_queue_put(os_queue_handler_t queue, void *msg, uint32_t timeout)
{
    // 注意：FreeRTOS 的时间参数是 Tick，需要用 pdMS_TO_TICKS 转换毫秒
    TickType_t ticks = (timeout == 0xFFFFFFFFUL) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

    // 强转回 FreeRTOS 句柄调用原生 API
    if (xQueueSend((QueueHandle_t)queue, msg, ticks) != pdTRUE) {
        return false;
    }
    return true;
}

bool os_queue_get(os_queue_handler_t queue, void *msg, uint32_t timeout)
{
    TickType_t ticks = (timeout == 0xFFFFFFFFUL) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

    if (xQueueReceive((QueueHandle_t)queue, msg, ticks) != pdTRUE) {
        return false;
    }
    return true;
}

bool os_queue_delete(os_queue_handler_t queue)
{
    // 防御性编程：FreeRTOS 的 vQueueDelete 传入 NULL 会触发断言死机
    if (queue == NULL) {
        return false;
    }

    // 强转回 FreeRTOS 句柄调用原生 API
    vQueueDelete((QueueHandle_t)queue);

    return true;
}

bool os_queue_put_from_isr(os_queue_handler_t queue, void *msg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // 强转回 FreeRTOS 句柄，调用中断专属 API，并传入上下文切换标记
    if (xQueueSendFromISR((QueueHandle_t)queue, msg, &xHigherPriorityTaskWoken) != pdTRUE) {
        return false;
    }

    // 【核心脏活】：在中断中发送消息后，如果唤醒了更高优先级的任务，
    // 必须请求上下文切换，否则高优先级任务不会立即执行。
    // 这一步操作对上层完全透明，上层不需要知道 portYIELD_FROM_ISR 的存在。
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return true;
}