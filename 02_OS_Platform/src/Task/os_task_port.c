//
// Created by 35540 on 2026/4/28.
//
#include "os_task_port.h"


//创建任务
uint8_t os_task_create(
    const char *task_name,
    os_task_func_t func_pointer,
    uint16_t stack_size,
    void *argument,
    uint32_t priority,
    os_task_handler_t *task_handle
) {
  if (task_name == NULL || func_pointer == NULL || task_handle == NULL) {
    return ERROR;
  }

  // 定义一个真正的 FreeRTOS 句柄用于接收底层返回值
  TaskHandle_t rtos_handle = NULL;

  // 调用原生 API，把抽象类型强转过去
  BaseType_t ret = xTaskCreate(
      (TaskFunction_t)func_pointer,  // 强转
      task_name,
      stack_size,
      argument,
      (UBaseType_t)priority,         // 强转
      &rtos_handle                   // 传原生句柄的地址
  );

  if (ret == pdPASS) {
    // 【核心解耦点】把原生句柄“抹除类型”后，赋值给上层的抽象句柄
    *task_handle = (os_task_handler_t)rtos_handle;
    return SUCCESS;
  }
  return ERROR;
}

uint8_t os_task_delete(os_task_handler_t task_handle) {
  if (task_handle == NULL) {
    return ERROR;
  }

  // 调用时，把抽象句柄强转回原生句柄
  vTaskDelete((TaskHandle_t)task_handle);
  return SUCCESS;
}

uint8_t os_task_delay(uint32_t ms) {
  vTaskDelay(pdMS_TO_TICKS(ms));
  return SUCCESS;
}

os_task_handler_t os_task_get_handle(void)
{
  // 获取原生句柄
  TaskHandle_t rtos_handle = xTaskGetCurrentTaskHandle();
  // 抹掉类型，返回抽象句柄
  return (os_task_handler_t)rtos_handle;
}
