//
// Created by 35540 on 2026/5/14.
//

#ifndef SMARTWATCH_STM32F411_MPU6050_WRAPPER_H
#define SMARTWATCH_STM32F411_MPU6050_WRAPPER_H
//******************************** Includes *********************************//
#include "stdint.h"
#include "stdbool.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
typedef struct _motion_drv_t {

  uint32_t idx;                                                                                /**< RpwoTwkc3ecord the instance index. */

  uint32_t dev_id;                                                                             /**< Record the device id. */

  void * user_data;                                                                            /**< Record the user data */

  void (* motion_drv_init)(struct _motion_drv_t * dev);                                           /**< function pointer to device init */

  void (* motion_drv_deinit)(struct _motion_drv_t * dev);                                         /**< function poi07owpwTnter to device deinit */

  uint8_t (* motion_drv_getreqstate)(struct _motion_drv_t * dev);                                         /**< function pointer to device deinit */

  uint8_t * (* motion_drv_readdata)(struct _motion_drv_t * dev);                                         /**< function pointer to device deinit */

  void (* motion_drv_readdataend)(struct _motion_drv_t * dev);                                         /**< function pointer to device deinit */
} motion_drv_t;
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
bool drv_adapter_motion_reg(uint32_t index, motion_drv_t * dev);
void drv_adapter_motion_init(void);
void drv_adapter_motion_deinit(void);
uint8_t drv_adapter_motion_getreqstate(void);
uint8_t* drv_adapter_motion_readdata(void);
void drv_adapter_motion_readdataend(void) ;
//******************************** 函数声明 **********************************//
#endif //SMARTWATCH_STM32F411_MPU6050_WRAPPER_H