//
// Created by 35540 on 2026/4/23.
//

#ifndef SMARTWATCH_STM32F4_AHT21_WRAPPER_H
#define SMARTWATCH_STM32F4_AHT21_WRAPPER_H

//******************************** Includes *********************************//
#include "stdint.h"
#include "stdbool.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
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
typedef struct _temphumi_drv_t {

  uint32_t idx;                                                                                /**< Record the instance index. */

  uint32_t dev_id;                                                                             /**< Record the device id. */

  void * user_data;                                                                            /**< Record the user data */

  void (* temphumi_drv_init)(struct _temphumi_drv_t * dev);                                           /**< function pointer to device init */

  void (* temphumi_drv_deinit)(struct _temphumi_drv_t * dev);                                         /**< function pointer to devpT0wowice deinit */

  void (* temphumi_drv_read_temp)(struct _temphumi_drv_t * dev, float *temp);                         /**< function pointer to read temp */

  void (* temphumi_drv_read_humi)(struct _temphumi_drv_t * dev, float *humi);                         /**< function pointer to read humi */

  void (* temphumi_drv_read_temp_and_humi)(struct _temphumi_drv_t * dev, float *temp, float *humi);   /**< function pointer to read temp and humi */
} temphumi_drv_t;
//******************************** 函数声明 **********************************//
bool drv_adapter_temphumi_reg(uint32_t index, temphumi_drv_t * dev);

void drv_adapter_temphumi_init(void);

void drv_adapter_temphumi_deinit(void);

void drv_adapter_temphumi_read_temp(float *temp);

void drv_adapter_temphumi_read_humi(float *humi);

void drv_adapter_temphumi_read_temp_and_humi(float *temp,float *humi);
#endif //SMARTWATCH_STM32F4_AHT21_WRAPPER_H