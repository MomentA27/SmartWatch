//
// Created by 35540 on 2026/5/15.
//
#include "motion_port.h"
#include "mpu6050_wrapper.h"
void motion_init(void) {
  drv_adapter_motion_init();

}

void motion_deinit(void) {
  drv_adapter_motion_deinit();

}

uint8_t* motion_readdata(void) {
  return drv_adapter_motion_readdata();
}

uint8_t motion_getreqstate(void) {
  return drv_adapter_motion_getreqstate();
}

void motion_readdataend(void) {
  drv_adapter_motion_readdataend();
}