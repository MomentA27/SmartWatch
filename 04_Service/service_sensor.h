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

//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
//每个传感器通过左移运算分配独立二进制位（如SENSOR_TEMP占第0位，SENSOR_HUMIDITY占第1位），避免冲突。
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
  uint32_t motion_sample_rate;        /* 运动采样率(ms) */
  uint32_t last_temp_sample;          /* 上一次温度采样时间 */
  uint32_t last_motion_sample;        /* 上一次运动采样时间 */
  bool temp_sampling_enabled;         /* 温度采样使能 */
  bool motion_sampling_enabled;       /* 运动采样使能 */
}service_sensor_handler_t;
/*==========================================================================*/
/*陀螺仪传感器数据结构体*/
typedef struct {
  int16_t accel_x_raw;               /* 原始加速度计数据（x轴） */
  int16_t accel_y_raw;               /* 原始加速度计数据（y轴） */
  int16_t accel_z_raw;               /* 原始加速度计数据（z轴） */

  double ax;                        /* 处理后的加速度计数据（g单位，x轴） */
  double ay;                        /* 处理后的加速度计数据（g单位，y轴） */
  double az;                        /* 处理后的加速度计数据（g单位，z轴） */

  int16_t gyro_x_raw;               /* 原始陀螺仪数据（x轴） */
  int16_t gyro_y_raw;               /* 原始陀螺仪数据（y轴） */
  int16_t gyro_z_raw;               /* 原始陀螺仪数据（z轴） */

  double gx;                        /* 处理后的陀螺仪数据（度/秒，x轴） */
  double gy;                        /* 处理后的陀螺仪数据（度/秒，y轴） */
  double gz;                        /* 处理后的陀螺仪数据（度/秒，z轴） */

  float temperature;                /* 温度读数（摄氏度） */

  double kalman_angle_x;            /* 卡尔曼滤波处理的角度（x轴） */
  double kalman_angle_y;            /* 卡尔曼滤波处理的角度（y轴） */
} st_sensor_motion_data_t;
/*温湿度传感器数据结构体*/
typedef struct
{
  float temp;                       /*温度读数 */
  float humi;                       /*湿度读数 */
}st_sensor_humitemp_data_t;
/*传感器数据统一结构体*/
typedef struct
{
  st_sensor_motion_data_t st_sensor_motion_data;
  st_sensor_humitemp_data_t st_sensor_humitemp_data;
}st_sensor_data_t;
/*===========================================================================*/
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