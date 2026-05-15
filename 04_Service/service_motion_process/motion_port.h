//
// Created by 35540 on 2026/5/15.
//

#ifndef SMARTWATCH_STM32F411_MOTION_PORT_H
#define SMARTWATCH_STM32F411_MOTION_PORT_H
#include "main.h"
void motion_init(void);
void motion_deinit(void);
uint8_t* motion_readdata(void);
uint8_t motion_getreqstate(void);
void motion_readdataend(void);
#endif //SMARTWATCH_STM32F411_MOTION_PORT_H