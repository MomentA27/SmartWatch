//
// Created by 35540 on 2026/4/21.
//

#ifndef SMARTWATCH_STM32F4_I2C_PORT_H
#define SMARTWATCH_STM32F4_I2C_PORT_H
//******************************** Includes *********************************//
#include "main.h"
#include "i2c.h"   // hardware i2c
#include "i2c_bus.h" // software i2c
#include "gpio_define.h"
#include "platform_os.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
/*=====================传感器IIC的宏==========================*/
// 互斥锁注册
#define SENSOR_I2C_REGISTER_MUTEX(mutex)        \
        core_i2c_register_mutex(CORE_I2C_BUS_1,mutex)
// 硬件i2c写
#define SENSOR_I2C_HARDWARE_WRITE(dev_addr,data,size,timeout)        \
        core_i2c_write(CORE_I2C_BUS_1, dev_addr, data, size, timeout)
// 硬件i2c读
#define SENSOR_I2C_HARDWARE_READ(dev_addr,data,size,timeout)        \
        core_i2c_read(CORE_I2C_BUS_1, dev_addr, data, size, timeout)
// 硬件 IIC 内存写
#define SENSOR_I2C_HARDWARE_MEM_WRITE(dev_addr, mem_addr, em_size, data, size, timeout) \
        core_i2c_mem_write(CORE_I2C_BUS_1, dev_addr, mem_addr, em_size, data, size, timeout)
// 硬件 IIC 内存读
#define SENSOR_I2C_HARDWARE_MEM_READ(dev_addr, mem_addr, em_size, data, size, timeout) \
        core_i2c_mem_read(CORE_I2C_BUS_1,  dev_addr, mem_addr, em_size, data, size, timeout)
// 硬件 IIC 内存读 DMA 模式
#define SENSOR_I2C_HARDWARE_MEM_READ_DMA(dev_addr, mem_addr, em_size, data, size, timeout) \
        core_i2c_mem_read_dma(CORE_I2C_BUS_1,  dev_addr, mem_addr, em_size, data, size)
// 软件 IIC 启动
#define SENSOR_I2C_SOFTWARE_START()  \
        core_i2c_soft_start(CORE_I2C_BUS_1)
// 软件 IIC 停止
#define SENSOR_I2C_SOFTWARE_STOP()  \
        core_i2c_soft_stop(CORE_I2C_BUS_1)
// 软件 IIC 等待应答
#define SENSOR_I2C_SOFTWARE_WAITACK()  \
        core_i2c_soft_wait_ack(CORE_I2C_BUS_1)
// 软件 IIC 发送应答
#define SENSOR_I2C_SOFTWARE_SENDACK()  \
        core_i2c_soft_send_ack(CORE_I2C_BUS_1)
// 软件 IIC 发送应答
#define SENSOR_I2C_SOFTWARE_SENDNOACK()  \
        core_i2c_soft_send_no_ack(CORE_I2C_BUS_1)
// 软件 IIC 发送字节
#define SENSOR_I2C_SOFTWARE_SENDBYTE(data)  \
        core_i2c_soft_send_byte(CORE_I2C_BUS_1,data)
// 软件 IIC 接收字节
#define SENSOR_I2C_SOFTWARE_RECEIVEBYTE(data)  \
        core_i2c_soft_receive_byte(CORE_I2C_BUS_1,data)
#define SENSOR_I2C_SOFTWARE_INIT()  \
        core_i2c_software_init(CORE_I2C_BUS_1)
/*=====================传感器IIC的宏==========================*/
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
typedef enum
{
  EN_HARDWARE_I2C = 0,
  EN_SOFTWARE_I2C
}en_i2c_state_t;

typedef enum
{
  CORE_I2C_OK = 0,
  CORE_I2C_ERROR
}en_core_i2c_status_t;

typedef enum
{
  CORE_I2C_BUS_1 = 0,
  CORE_I2C_BUS_2,
  CORE_I2C_BUS_MAX
}en_core_i2c_bus_t;


typedef struct
{
  en_i2c_state_t en_i2c_state;
  i2c_bus_t st_iic_bus_inst;                          //软件i2c接口
  I2C_HandleTypeDef* st_I2C_HandleTypeDef;             //硬件i2c接口
  SemaphoreHandle_t st_osMutexId;                   //互斥锁接口
} st_i2c_port_t;

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
void core_i2c_register_mutex(en_core_i2c_bus_t bus, SemaphoreHandle_t mutex);
en_core_i2c_status_t core_i2c_write(en_core_i2c_bus_t bus, uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout);
en_core_i2c_status_t core_i2c_read(en_core_i2c_bus_t bus, uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout);
en_core_i2c_status_t core_i2c_mem_write(en_core_i2c_bus_t bus, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size,
                                     uint8_t *data, uint16_t size, uint32_t timeout);
en_core_i2c_status_t core_i2c_mem_read(en_core_i2c_bus_t bus, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size,
                                    uint8_t *data, uint16_t size, uint32_t timeout);
en_core_i2c_status_t core_i2c_mem_read_dma(en_core_i2c_bus_t bus, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size,
                                            uint8_t *data, uint16_t size);
en_core_i2c_status_t core_i2c_soft_start(en_core_i2c_bus_t bus);
en_core_i2c_status_t core_i2c_soft_stop(en_core_i2c_bus_t bus);
en_core_i2c_status_t core_i2c_soft_wait_ack(en_core_i2c_bus_t bus);
en_core_i2c_status_t core_i2c_soft_send_ack(en_core_i2c_bus_t bus);
en_core_i2c_status_t core_i2c_soft_send_no_ack(en_core_i2c_bus_t bus);
en_core_i2c_status_t core_i2c_soft_send_byte(en_core_i2c_bus_t bus,uint8_t data);
en_core_i2c_status_t core_i2c_soft_receive_byte(en_core_i2c_bus_t bus,uint8_t * const data);
en_core_i2c_status_t core_i2c_software_init(en_core_i2c_bus_t bus);
//******************************** 函数声明 **********************************//
#endif //SMARTWATCH_STM32F4_I2C_PORT_H