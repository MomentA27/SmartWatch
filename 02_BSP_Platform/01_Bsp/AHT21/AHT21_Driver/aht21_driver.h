//
// Created by 35540 on 2026/3/28.
//

#ifndef SMARTWATCH_STM32F4_AHT21_DRIVER_H
#define SMARTWATCH_STM32F4_AHT21_DRIVER_H
//******************************** Includes *********************************//
#include <stdint.h>
#include "main.h"
#include <stdio.h>
#include "aht21_reg.h"
#include "elog.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define AHT21DRriver 1
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
/* 函数返回状态     */
/*  函数返回状态枚举                    */
typedef enum
{
    AHT21_OK                = 0,         //* 操作成功完成
    AHT21_ERROR             = 1,         //* 运行时错误，无匹配情况
    AHT21_ERRORTIMEOUT      = 2,         //* 操作失败，超市
    AHT21_ERRORRESOURCE     = 3,         //* 资源不可用
    AHT21_ERRORPARAMETER    = 4,         //* 参数错误
    AHT21_ERRORMEMORY       = 5,         //* 内存不足
    AHT21_ERRORISR          = 6,         //* 中断服务程序（ISR）上下文不允许
    AHT21_RESERVED          = 0x7FFFFFFF //* 保留
}aht21_status_t;
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//
/** 定义统一的IIC接口  **/
typedef struct
{
    aht21_status_t (*pf_i2c_init)        (void*);   /*   IIC init    interf.*/
    aht21_status_t (*pf_i2c_deinit)      (void*);   /*   IIC deinit  interf.*/
    aht21_status_t (*pf_i2c_start)       (void*);   /*   IIC start   interf.*/
    aht21_status_t (*pf_i2c_stop)         (void*);  /*   IIC stop    interf.*/
    aht21_status_t (*pf_i2c_wait_ack)     (void*);  /*   IIC w-ack   interf.*/
    aht21_status_t (*pf_i2c_send_ack)     (void*);  /*   IIC s-ack   interf.*/
    aht21_status_t (*pf_i2c_send_nack)    (void*);  /*   IIC s-n-ack interf.*/
    aht21_status_t (*pf_i2c_send_byte)    (void*,   /*   IIC s-byte  interf.*/
                                           const uint8_t);
    aht21_status_t (*pf_i2c_receive_byte) (void*,   /*   IIC r-byte  interf.*/
                                            uint8_t * const );
    aht21_status_t (*pf_critical_enter)   (void);   /* enter critical state.*/
    aht21_status_t (*pf_critical_exit)    (void);   /* exit  critical state.*/
}aht_i2c_driver_interface_t;

/** 获取时基数     */
typedef struct
{
  uint32_t (*pf_get_tick_count)(void);            /*Get Tick number interf.*/
}timebase_interface_t;

/** 获取让出CPU使用的函数 */
typedef struct
{
  void (*pf_rtos_yield)(uint32_t);           /*OS Not-Blocking Delay  */
}yield_interface_t;
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
typedef struct bsp_aht21_driver_struct bsp_aht21_driver_t ;
typedef struct bsp_aht21_driver_struct
{
  /** 依赖接口（由外部注入，实现底层I2C、延时、时基等） */
  aht_i2c_driver_interface_t *p_i2c_driver_interface;
  timebase_interface_t       *p_timebase_interface;
  yield_interface_t          *p_yield_interface;
  /** 对外方法（供应用层调用，实现传感器初始化、读温湿度等） */
  /**对象实例化函数指针*/
  aht21_status_t (*pf_inst)(
    bsp_aht21_driver_t*     const p_aht21_instance,
    aht_i2c_driver_interface_t* const p_i2c_driver_interface,
    timebase_interface_t*   const p_timebase_interface,
    yield_interface_t*      const p_yield_interface
  );
  /**初始化函数指针*/
  aht21_status_t (*pf_init)         (bsp_aht21_driver_t * const);
  /**反初始化函数指针*/
  aht21_status_t (*pf_deinit)       (bsp_aht21_driver_t * const);
  /**读取状态ID函数指针*/
  aht21_status_t (*pf_read_id)      (bsp_aht21_driver_t * const,
                                     uint8_t * const);
  /**  读取湿度和温度函数*/
  aht21_status_t (*pf_read_temp_humi)(
    bsp_aht21_driver_t * const,
    float              * const temp,
    float              * const humi);
  /**  读取湿度函数*/
  aht21_status_t (*pf_read_humi)(
    bsp_aht21_driver_t * const ,
    float              * const humi);
  /**  元件睡眠函数     */
  aht21_status_t (*pf_sleep)        (bsp_aht21_driver_t * const);
  /**  元件唤醒函数     */
  aht21_status_t (*pf_wakeup)       (bsp_aht21_driver_t * const);
}bsp_aht21_driver_t;
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 ***********************************//
aht21_status_t aht21_inst(
    bsp_aht21_driver_t*     const p_bsp_aht21_inst,
    aht_i2c_driver_interface_t* const p_i2c_driver_inst,
    timebase_interface_t*   const p_timebase_inst,
    yield_interface_t*      const p_yield_inst);
//******************************** 函数声明 ***********************************//
#endif //SMARTWATCH_STM32F4_AHT21_DRIVER_H