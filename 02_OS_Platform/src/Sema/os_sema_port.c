//
// Created by 35540 on 2026/4/29.
//
#include "os_sema_port.h"



uint8_t os_sema_create(os_sema_handler_t *sema, bool is_binary, uint32_t max_count, uint32_t init_count)
{
  if (sema == NULL) {
    return ERROR;
  }

  SemaphoreHandle_t h_sema = NULL;

  // 根据 is_binary 调用 FreeRTOS 不同的原生创建 API
  if (is_binary) {
    // 创建二值信号量（FreeRTOS 创建后默认是空的，需要先 post 才能 wait）
    h_sema = xSemaphoreCreateBinary();
  } else {
    // 创建计数信号量
    h_sema = xSemaphoreCreateCounting(max_count, init_count);
  }

  if (h_sema == NULL) {
    return ERROR;
  }

  // 类型抹除，将原生句柄赋值给抽象句柄
  *sema = (os_sema_handler_t)h_sema;
  return SUCCESS;
}

uint8_t os_sema_take(os_sema_handler_t sema, uint32_t timeout)
{
  if (sema == NULL) {
    return ERROR;
  }

  // 统一处理超时时间转换
  TickType_t ticks = (timeout == 0xFFFFFFFFUL) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

  // FreeRTOS 获取信号量
  if (xSemaphoreTake((SemaphoreHandle_t)sema, ticks) != pdTRUE) {
    return ERROR;
  }
  return SUCCESS;
}

uint8_t os_sema_give(os_sema_handler_t sema)
{
  if (sema == NULL) {
    return ERROR;
  }

  // FreeRTOS 释放信号量
  if (xSemaphoreGive((SemaphoreHandle_t)sema) != pdTRUE) {
    return ERROR;
  }
  return SUCCESS;
}