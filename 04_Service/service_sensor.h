//
// Created by 35540 on 2026/4/24.
//

#ifndef SMARTWATCH_STM32F4_SERVICE_SENSOR_H
#define SMARTWATCH_STM32F4_SERVICE_SENSOR_H
//******************************** Includes *********************************//
#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "elog.h"
#include "platform_os.h"
#include "temp_humi_port.h"
#include "user_init.h"
#include "watchdog_monitor.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define SENSOR_TEMP         (1 << 0)
#define SENSOR_HUMIDITY     (1 << 1)
#define SENSOR_MOTION       (1 << 2)
#define SENSOR_PRESSURE     (1 << 3)
#define SENSOR_ALTITUDE     (1 << 4)
#define SENSOR_HEARTRATE    (1 << 5)
#define SENSOR_ALL          (SENSOR_TEMP | SENSOR_HUMIDITY | SENSOR_MOTION | SENSOR_PRESSURE | SENSOR_ALTITUDE | SENSOR_HEARTRATE)

#define SERVICE_SENSOR_DEBUG 1
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
/* 传感器状态结构体 */
typedef struct {
  uint32_t active_sensors;            /* 当前活跃的传感器掩码 */
  uint32_t temp_sample_rate;          /* 温度采样率(ms) */
  uint32_t last_temp_sample;          /* 上一次温度采样时间 */
  bool temp_sampling_enabled;         /* 温度采样使能 */
}service_sensor_handler_t;
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//**************************** Extern Variables *****************************//
//**************************** Extern Variables *****************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 **********************************//
void sensor_polling_task(void *argument);
//******************************** 函数声明 **********************************//
#endif //SMARTWATCH_STM32F4_SERVICE_SENSOR_H