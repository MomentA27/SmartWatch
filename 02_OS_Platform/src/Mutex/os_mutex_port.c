//
// Created by 35540 on 2026/4/28.
//
#include "os_mutex_port.h"


uint8_t os_mutex_create(os_mutex_handler_t *mutex)
{
  // 创建二值信号量（互斥锁本质上是包含优先级继承的二值信号量）
  SemaphoreHandle_t h_mutex = xSemaphoreCreateMutex();

  if (h_mutex == NULL) {
    return ERROR;
  }

  *mutex = (os_mutex_handler_t)h_mutex;
  return SUCCESS;
}

uint8_t os_mutex_take(os_mutex_handler_t mutex, uint32_t timeout)
{
  TickType_t ticks = (timeout == 0xFFFFFFFFUL) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

  // FreeRTOS 获取信号量就是加锁
  if (xSemaphoreTake((SemaphoreHandle_t)mutex, ticks) != pdTRUE) {
    return ERROR;
  }
  return SUCCESS;
}

uint8_t os_mutex_give(os_mutex_handler_t mutex)
{
  // FreeRTOS 释放信号量就是解锁
  if (xSemaphoreGive((SemaphoreHandle_t)mutex) != pdTRUE) {
    return ERROR;
  }
  return SUCCESS;
}

