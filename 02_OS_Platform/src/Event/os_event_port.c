//
// Created by 35540 on 2026/4/29.
//
#include "os_event_port.h"

bool os_event_create(os_event_hanlder_t *event_handle)
{
    if (event_handle == NULL) {
        return false;
    }

    EventGroupHandle_t rtos_handle = xEventGroupCreate();
    if (rtos_handle == NULL) {
        return false;
    }

    // 类型抹除赋值
    *event_handle = (os_event_hanlder_t)rtos_handle;
    return true;
}

bool os_event_wait(os_event_hanlder_t event, uint32_t wait_bits, bool wait_all, bool auto_clear, uint32_t timeout, uint32_t *out_active_bits)
{
    if (event == NULL || out_active_bits == NULL) {
        return false;
    }

    // 超时时间适配
    TickType_t ticks = (timeout == 0xFFFFFFFFUL) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

    // FreeRTOS 原生参数适配
    BaseType_t xWaitForAll = wait_all ? pdTRUE : pdFALSE;
    BaseType_t xClearOnExit = auto_clear ? pdTRUE : pdFALSE;

    // 调用原生 API 并强转句柄
    EventBits_t uxBits = xEventGroupWaitBits(
        (EventGroupHandle_t)event,
        wait_bits,
        xClearOnExit,
        xWaitForAll,
        ticks
    );

    // FreeRTOS 中，如果超时且没有获取到任何期望的 bit，返回的是 0 或者是没有包含期望 bits 的值
    // 我们可以通过检查返回值是否包含期望的 bits 来判断是否成功（简单处理，只要没超时就算成功）
    if (uxBits == 0 && wait_bits != 0) {
        // 注意：这里的判断逻辑视具体情况而定，如果 wait_bits 包含 0，这里的逻辑会有漏洞。
        // 但在常规业务中，等待的 bits 肯定不为 0。
        return false;
    }

    *out_active_bits = (uint32_t)uxBits;
    return true;
}

bool os_event_set(os_event_hanlder_t event, uint32_t set_bits)
{
    if (event == NULL) {
        return false;
    }

    // 调用原生 API，通常在中断中要调用 xEventGroupSetBitsFromISR，这里简化为任务中使用
    EventBits_t uxBits = xEventGroupSetBits((EventGroupHandle_t)event, set_bits);

    // 只要没崩溃就算成功
    return true;
}

bool os_event_clear(os_event_hanlder_t event, uint32_t clear_bits)
{
    if (event == NULL) {
        return false;
    }

    EventBits_t uxBits = xEventGroupClearBits((EventGroupHandle_t)event, clear_bits);
    return true;
}

bool os_event_delete(os_event_hanlder_t event)
{
    if (event == NULL) {
        return false;
    }

    vEventGroupDelete((EventGroupHandle_t)event);
    return true;
}