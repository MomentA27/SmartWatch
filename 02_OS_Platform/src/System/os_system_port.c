//
// Created by 35540 on 2026/4/29.
//
#include "os_system_port.h"
/*              临界区管理            */
void os_enter_critical(void) {
  taskENTER_CRITICAL();
}

void os_exit_critical(void) {
  taskEXIT_CRITICAL();
}


// ================= 2. 时间管理 =================

void os_delay_ms(uint32_t ms) {
  // 直接调用 FreeRTOS API，内部会自动完成 ms 到 tick 的转换
  vTaskDelay(pdMS_TO_TICKS(ms));
}

uint32_t os_get_tick_ms(void) {
  return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}