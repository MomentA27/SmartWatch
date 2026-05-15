//
// Created by 35540 on 2026/3/28.
//

#ifndef SMARTWATCH_STM32F4_AHT21_HANDLER_H
#define SMARTWATCH_STM32F4_AHT21_HANDLER_H
//******************************** Includes *********************************//
#include <stdint.h>
#include "aht21_driver.h"
#include <stdio.h>
#include "platform_os.h"
#include "watchdog_monitor.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define AHT21Han_DEBUG 1
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
/** 状态枚举 */
typedef enum
{
  TEMP_HUMI_OK                = 0,         //* 操作成功完成
  TEMP_HUMI_ERROR             = 1,         //* 运行时错误：无匹配情况
  TEMP_HUMI_ERRORTIMEOUT      = 2,         //* 操作失败：超时
  TEMP_HUMI_ERRORRESOURCE     = 3,         //* 资源不可用
  TEMP_HUMI_ERRORPARAMETER    = 4,         //* 参数错误
  TEMP_HUMI_ERRORNOMEMORY     = 5,         //* 内存不足
  TEMP_HUMI_ERRORISR          = 6,         //* 中断服务程序（ISR）上下文中不允许
  TEMP_HUMI_RESERVED          = 0x7FFFFFFF //* 保留
}temp_humi_status_t;

/** 事件类型枚举 */
typedef enum
{
  TEMP_HUMI_EVENT_TEMP = 0,
  TEMP_HUMI_EVENT_HUMI,
  TEMP_HUMI_EVENT_BOTH
}tempt_humi_event_data_type_t;

/** 事件结构体 */
typedef struct
{
  float                    *temperature;  /* 温度参考    */
  float                       *humidity;  /* 湿度参考    */
  uint32_t                     lifetime;  /* 数据有效期  */
  uint32_t                     timestap;  /* 事件时间戳  */
  tempt_humi_event_data_type_t     type;  /* 数据模式    */
  void (*pf_callback)(float *, float *);  /* 回调函数    */
}temp_humi_event_t;
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//
/** 接口结构体 */
typedef struct
{
  temp_humi_status_t (*os_delay)      (uint32_t ms);
  temp_humi_status_t (*os_queue_creat)(uint32_t num,
                                       uint32_t size,
                                       void** p_queue_handler);
  temp_humi_status_t (*os_queue_put)  (void* queue_handler,
                                       void* item,
                                       uint32_t timeout);
  temp_humi_status_t (*os_queue_get)  (void* queue_handler,
                                       void* item,
                                       uint32_t timeout);
}temp_humi_handler_os_api_t;

typedef struct
{
  /* 传递给驱动层的接口 */
  aht21_driver_input_api_t   *driver_api; // 👈 替代原来的 I2C、Tick、Yield
  //原本的timebase包含在driver_api中 hanlder无法访问 但由于handler层中有多时基函数的需求 所以需要加上自己的timebase接口
  aht21_timebase_interface_t       *timebase_interface  ;
  aht21_yield_interface_t          *yield_interface     ;

  /* os操作接口 */
  temp_humi_handler_os_api_t *os_interface;
}temp_humi_handler_input_api_t;

typedef struct
{
  uint8_t init_flag;
}temp_humi_handler_private_data_t;
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
typedef struct
{
  /* 传递给驱动层的接口 */
  aht21_driver_input_api_t   *driver_api;       // 👈 只要包，不碰具体线束
  /* Handler 自己算生命周期仍需要 Tick，保留 */
  aht21_timebase_interface_t   *timebase_interface;
  /* os操作接口 */
  temp_humi_handler_os_api_t *os_interface;

  /* 底层驱动的具体实例对象 */
  bsp_aht21_driver_t *p_aht21_instance;

  /* 事件队列处理程序 */
  void *event_queue_handler;

  /* 私人数据 */
  temp_humi_handler_private_data_t *p_private_data;

  /* 上一次读取温度的时间戳 */
  uint32_t last_temp_tick;

  /* 上一次读取湿度的时间戳 */
  uint32_t last_humi_tick;
}bsp_temp_humi_handler_t;
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//**************************** Extern Variables *****************************//
//**************************** Extern Variables *****************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 **********************************//
temp_humi_status_t bsp_temp_humi_handler_inst(
                            bsp_temp_humi_handler_t* p_handler,
                      const temp_humi_handler_input_api_t* const p_input_api);
temp_humi_status_t bsp_temp_humi_get_data(
                            bsp_temp_humi_handler_t* handler_instance,
                            temp_humi_event_t  event,
                            float* temp,
                            float* humi);
temp_humi_status_t bsp_temp_humi_read(temp_humi_event_t* event);
void temp_humi_handler_thread(void* argument);
//******************************** 函数声明 **********************************//
#endif //SMARTWATCH_STM32F4_AHT21_HANDLER_H