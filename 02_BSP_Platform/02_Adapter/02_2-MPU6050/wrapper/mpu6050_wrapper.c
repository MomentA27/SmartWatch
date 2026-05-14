//
// Created by 35540 on 2026/5/14.
//
//******************************** Includes *********************************//
#include "mpu6050_wrapper.h"

//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//

//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//
static motion_drv_t      _motion_drv_dev[2];
static uint32_t         _s_current_motion_dev_idx = 0;
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
bool drv_adapter_motion_reg(uint32_t index, motion_drv_t * dev)
{
    if (index >= 2) {
        return false;
    }
    _motion_drv_dev[index].idx = \
                           index;
    _motion_drv_dev[index].dev_id = \
                           dev->dev_id;
    _motion_drv_dev[index].user_data = \
                           dev->user_data;
    _motion_drv_dev[index].motion_drv_init = \
                           dev->motion_drv_init;
    _motion_drv_dev[index].motion_drv_deinit = \
                           dev->motion_drv_deinit;
    _motion_drv_dev[index].motion_drv_getreqstate = \
                           dev->motion_drv_getreqstate;
    _motion_drv_dev[index].motion_drv_readdata = \
                           dev->motion_drv_readdata;
    _motion_drv_dev[index].motion_drv_readdataend = \
                           dev->motion_drv_readdataend;
    _s_current_motion_dev_idx = index;
    return true;
}

void drv_adapter_motion_init(void) {

    motion_drv_t * dev = \
            &_motion_drv_dev[_s_current_motion_dev_idx];

    if(dev->motion_drv_init) {
        dev->motion_drv_init(dev);
    }

    return;
}

void drv_adapter_motion_deinit(void) {
    motion_drv_t * dev = \
            &_motion_drv_dev[_s_current_motion_dev_idx];

    if(dev->motion_drv_deinit) {
        dev->motion_drv_deinit(dev);
    }

    return;
}

uint8_t drv_adapter_motion_getreqstate(void) {

    motion_drv_t * dev = \
            &_motion_drv_dev[_s_current_motion_dev_idx];

    if(dev->motion_drv_getreqstate) {
      return dev->motion_drv_getreqstate(dev);
    }

    return 1;
}

uint8_t* drv_adapter_motion_readdata(void) {
    motion_drv_t * dev = \
            &_motion_drv_dev[_s_current_motion_dev_idx];

    if(dev->motion_drv_readdata) {
      return dev->motion_drv_readdata(dev);
    }
    return 0;
}

void drv_adapter_motion_readdataend(void) {
    motion_drv_t * dev = \
            &_motion_drv_dev[_s_current_motion_dev_idx];

    if(dev->motion_drv_readdataend) {
		dev->motion_drv_readdataend(dev);
    }
}