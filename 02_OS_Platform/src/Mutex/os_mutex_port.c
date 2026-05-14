//
// Created by 35540 on 2026/4/28.
//
#include "os_mutex_port.h"



bool os_mutex_create(os_mutex_handler_t *mutex)
{
  // 创建二值信号量（互斥锁本质上是包含优先级继承的二值信号量）
  SemaphoreHandle_t h_mutex = xSemaphoreCreateMutex();

  if (h_mutex == NULL) {
    return false;
  }

  *mutex = (os_mutex_handler_t)h_mutex;
  return true;
}

bool os_mutex_take(os_mutex_handler_t mutex, uint32_t timeout)
{
  TickType_t ticks = (timeout == 0xFFFFFFFFUL) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

  // FreeRTOS 获取信号量就是加锁
  if (xSemaphoreTake((SemaphoreHandle_t)mutex, ticks) != pdTRUE) {
    return false;
  }
  return true;
}

bool os_mutex_give(os_mutex_handler_t mutex)
{
  // FreeRTOS 释放信号量就是解锁
  if (xSemaphoreGive((SemaphoreHandle_t)mutex) != pdTRUE) {
    return false;
  }
  return true;
}

bool os_mutex_delete(os_mutex_handler_t mutex)
{
  // 防御性编程：FreeRTOS 的 vSemaphoreDelete 如果传入 NULL 会触发断言死机
  if (mutex == NULL) {
    return false;
  }

  // 强转回 FreeRTOS 的句柄类型并调用原生删除 API
  // 注意：vSemaphoreDelete 是一个宏，本质上调用的是 vQueueDelete
  vSemaphoreDelete((SemaphoreHandle_t)mutex);

  // FreeRTOS 的删除函数没有返回值（void），只要参数合法就不会失败
  return true;
}

