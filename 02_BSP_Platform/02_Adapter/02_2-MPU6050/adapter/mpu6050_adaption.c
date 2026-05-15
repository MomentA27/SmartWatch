//
// Created by 35540 on 2026/5/14.
//
//******************************** Includes *********************************//
#include "i2c_port.h"
#include "mpu6050_handler.h"
#include "platform_os.h"
#include "mpu6050_wrapper.h"
#include "circular_buffer.h"
#include "DWT_delay.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//

//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//
extern mpu6050_handler_input_args_t mpu6050_input_args;
extern bsp_mpu6050_handler_t handler_instance;
extern circular_buffer_t circular_buffer;
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
static void mpu6050_drv_init(motion_drv_t *dev);
static void mpu6050_drv_deinit(motion_drv_t *dev);
static uint8_t mpu6050_drv_getreqstate(motion_drv_t *dev);    //获取MPU6050数据请求状态
static uint8_t * mpu6050_drv_readdata(motion_drv_t *dev);     //读取MPU6050传感器数据
static void mpu6050_drv_readdataend(motion_drv_t *dev);       //结束MPU6050数据读取操作
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
void drv_adapter_motion_register(void)
{
  motion_drv_t _motion_drv = {
    .idx = 0,
    .dev_id = 0,
    .motion_drv_init = mpu6050_drv_init,
    .motion_drv_deinit = mpu6050_drv_deinit,
    .motion_drv_getreqstate = mpu6050_drv_getreqstate,
    .motion_drv_readdata = mpu6050_drv_readdata,
    .motion_drv_readdataend = mpu6050_drv_readdataend,
};

  drv_adapter_motion_reg(0, &_motion_drv);
}
static void mpu6050_drv_init(motion_drv_t *dev)
{
  //Handler线程已完成

}

static void mpu6050_drv_deinit(motion_drv_t *dev)
{
  //销毁线程
}

static uint8_t mpu6050_drv_getreqstate(motion_drv_t *dev)
{
  uint8_t data = 0;
  mpu6050_status_t ret = MPU6050_OK;
  //等待队列唤醒
  ret = handler_instance.p_input_args->p_os->os_queue_get\
                  ( handler_instance.p_unpack_queue_handle,
                                              &data,
                                              30 );

  return (uint8_t)ret;
}
static uint8_t * mpu6050_drv_readdata(motion_drv_t *dev)
{
  uint8_t *addr = circular_buffer.pfget_rbuffer_addr(&circular_buffer);

  return addr;
}

static void mpu6050_drv_readdataend(motion_drv_t *dev)
{
  circular_buffer.pfdata_readed(&circular_buffer);
}
/*=======================IIC接口===========================*/
mpu6050_status_t iic_driver_init(void * iic_bus)
{
  // has already inited in main.c
  return MPU6050_OK;
}

mpu6050_status_t iic_driver_deinit(void *iic_bus)
{
  __HAL_RCC_I2C1_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);
  return MPU6050_OK;
}

mpu6050_status_t iic_mem_read(void *hi2c,
                              uint16_t dst_address,
                              uint16_t mem_addr,
                              uint16_t mem_size,
                              uint8_t  *p_data,
                              uint16_t size,
                              uint32_t timeout)
{
      uint8_t ret = HAL_OK;
  //ret = HAL_I2C_Mem_Read(hi2c, dst_address, mem_addr, mem_size, p_data, size, timeout);
  //TODO:
  ret = SENSOR_I2C_HARDWARE_MEM_READ(dst_address, mem_addr, mem_size, p_data, size, timeout);
  if (ret != HAL_OK)
  {
    return MPU6050_ERROR;
  }
  return MPU6050_OK;
}

mpu6050_status_t iic_mem_write(void *hi2c,
                               uint16_t dst_address,
                               uint16_t mem_addr,
                               uint16_t mem_size,
                               uint8_t  *p_data,
                               uint16_t size,
                               uint32_t timeout)
{
  uint8_t ret = HAL_OK;
  // ret = HAL_I2C_Mem_Write(hi2c, dst_address, mem_addr, mem_size, p_data, size, timeout);
  ret = SENSOR_I2C_HARDWARE_MEM_WRITE(dst_address, mem_addr, mem_size, p_data, size, timeout);
  if (ret != HAL_OK)
  {
    return MPU6050_ERROR;
  }
  return MPU6050_OK;
}

mpu6050_status_t iic_mem_read_dma(void *hi2c,
                                 uint16_t dst_address,
                                 uint16_t mem_addr,
                                 uint16_t mem_size,
                                 uint8_t  *p_data,
                                 uint16_t size )
{
  uint8_t ret = HAL_OK;
  ret = HAL_I2C_Mem_Read_DMA(hi2c, dst_address, mem_addr, mem_size, p_data, size);
  if (ret != HAL_OK)
  {
    return MPU6050_ERROR;
  }
  return MPU6050_OK;
}

mpu6050_iic_driver_interface_t iic_driver_interface = {
  .hi2c                = &hi2c1,
  .pf_iic_init         = iic_driver_init,
  .pf_iic_deinit       = iic_driver_deinit,
  .pf_iic_mem_read     = iic_mem_read,
  .pf_iic_mem_write    = iic_mem_write,
  .pf_iic_mem_read_dma = iic_mem_read_dma,
};
/*=======================IIC接口===========================*/
/*=======================时基接口===========================*/
mpu6050_timebase_interface_t timebase_interface = {
  .pf_get_tick_count = HAL_GetTick,
};
/*=======================时基接口===========================*/
/*=======================缓冲区接口=========================*/

circular_buffer_t circular_buffer;
mpu6050_buffer_interface_t buffer_interface = {
    .p_ctx = &circular_buffer,
    .pf_buffer_init = buffer_init,
    .pf_get_rbuffer_addr = get_rbuffer_addr,
    .pf_get_wbuffer_addr = get_wbuffer_addr,
    .pf_data_writed = data_writed,
    .pf_data_readed = data_readed
};
/*=======================缓冲区接口=========================*/
/*=======================临界段接口=========================*/
#ifdef OS_SUPPORTING
mpu6050_yield_interface_t yield_interface = {
  .pf_rtos_yield = vTaskDelay,
};
#endif
/*=======================临界段接口=========================*/
/*=======================延时接口=========================*/
mpu6050_delay_interface_t delay_interface = {
  .pf_delay_init = DWT_Delay_Init,
  .pf_delay_us = DWT_Delay_us,
  .pf_delay_ms = DWT_Delay_ms,
};
/*=======================延时接口=========================*/
/*=======================OS接口=========================*/
#ifdef OS_SUPPORTING
mpu6050_status_t os_queue_create_myown(uint32_t const queue_size,
                                       uint32_t const item_size,
                                      void **queue_handle)
{
  if (NULL == queue_handle)
  {
    return MPU6050_ERRORPARAMETER;
  }

  os_queue_create(queue_handle,queue_size, item_size);
  if (NULL == *queue_handle)
  {
    return MPU6050_ERRORNOMEMORY;
  }
  return MPU6050_OK;
}

mpu6050_status_t os_queue_delete_myown(void * const queue_handle)
{
  if (NULL == queue_handle)
  {
    return MPU6050_ERRORPARAMETER;
  }
  os_queue_delete(queue_handle);
  return MPU6050_OK;
}

mpu6050_status_t os_queue_put_myown(void * const queue_handle,
                            void * const item,
                            uint32_t const timeout)
{
  if (NULL == queue_handle || NULL == item)
  {
    return MPU6050_ERRORPARAMETER;
  }

  bool ret = os_queue_put(queue_handle, item, timeout);
  return (SUCCESS == ret) ? MPU6050_OK : MPU6050_ERRORTIMEOUT;
}

mpu6050_status_t os_queue_put_isr_myown(void * const queue_handle,
                                         void * const item)
{
  if (NULL == queue_handle || NULL == item) {
    return MPU6050_ERRORPARAMETER;
  }

  // 底层会自动处理上下文切换，上层啥也不用管
  bool ret = os_queue_put_from_isr(queue_handle, item); // 注意：之前底层返回的是 uint8_t，不是 bool

  return (ret) ? MPU6050_OK : MPU6050_ERROR; 
}

mpu6050_status_t os_queue_get_myown(void * const queue_handle,
                            void * const item,
                            uint32_t const timeout)
{
  if (NULL == queue_handle || NULL == item)
  {
    return MPU6050_ERRORPARAMETER;
  }

  bool ret = os_queue_get(queue_handle, item, timeout);
  return (SUCCESS == ret) ? MPU6050_OK : MPU6050_ERRORTIMEOUT;
}

mpu6050_status_t os_semaphore_create_mutex_myown(void **mutex_handle)
{
  if (NULL == mutex_handle)
  {
    return MPU6050_ERRORPARAMETER;
  }

  os_mutex_create(mutex_handle);
  if (NULL == *mutex_handle)
  {
    return MPU6050_ERRORNOMEMORY;
  }
  return MPU6050_OK;
}

mpu6050_status_t os_semaphore_delete_mutex_myown(void * const mutex_handle)
{
  if (NULL == mutex_handle)
  {
    return MPU6050_ERRORPARAMETER;
  }
  //vQueueDelete(mutex_handle);
  return MPU6050_OK;
}

mpu6050_status_t os_semaphore_lock_mutex_myown(void * const mutex_handle)
{
  if (NULL == mutex_handle)
  {
    return MPU6050_ERRORPARAMETER;
  }

  bool ret = os_mutex_take(mutex_handle, portMAX_DELAY);
  return (ret) ? MPU6050_OK : MPU6050_ERROR; 
}

mpu6050_status_t os_semaphore_unlock_mutex_myown(void * const mutex_handle)
{
  if (NULL == mutex_handle)
  {
    return MPU6050_ERRORPARAMETER;
  }

  bool ret = os_mutex_give(mutex_handle);
  return (ret) ? MPU6050_OK : MPU6050_ERROR; 
}

mpu6050_status_t os_semaphore_create_binary_myown(void **binary_handle)
{
  if (NULL == binary_handle)
  {
    return MPU6050_ERRORPARAMETER;
  }

  os_sema_create(binary_handle,1,0,0 );
  if (NULL == *binary_handle)
  {
    return MPU6050_ERRORNOMEMORY;
  }
  return MPU6050_OK;
}

mpu6050_status_t os_semaphore_delete_binary_myown(void * const binary_handle)
{
  //vQpjTwowueueDelete(binary_handle);
  return MPU6050_OK;
}
//调用 FreeRTOS 的 xSemaphoreGive，将信号量的值从 0 变成 1。
//如果当前没有人在等（没有任务因为 wait 而阻塞）：信号量被置为 1。这就好比按了铃，铃铛一直响着，直到有人来处理
//如果当前正好有人在等（有任务因为 wait 而阻塞）：信号量被消费，值保持为 0，但操作系统会立刻唤醒那个等待的任务，让它继续往下跑。
mpu6050_status_t os_semaphore_signal_binary_myown(void * const binary_handle)
{
  if (NULL == binary_handle)
  {
    return MPU6050_ERRORPARAMETER;
  }

  bool ret = os_sema_give(binary_handle);
  return (ret) ? MPU6050_OK : MPU6050_ERROR; 
}

mpu6050_status_t os_semaphore_wait_binary_myown(void * const binary_handle)
{
  if (NULL == binary_handle)
  {
    return MPU6050_ERRORPARAMETER;
  }

  bool ret = os_sema_take(binary_handle, portMAX_DELAY);
  return (ret) ? MPU6050_OK : MPU6050_ERROR; 
}

mpu6050_os_interface_t os_interface = {
  .os_queue_create = os_queue_create_myown,
  .os_queue_delete = os_queue_delete_myown,
  .os_queue_put    = os_queue_put_myown,
  .os_queue_put_isr= os_queue_put_isr_myown,
  .os_queue_get    = os_queue_get_myown,

  .os_semaphore_create_mutex = os_semaphore_create_mutex_myown,
  .os_semaphore_delete_mutex = os_semaphore_delete_mutex_myown,
  .os_semaphore_lock_mutex   = os_semaphore_lock_mutex_myown,
  .os_semaphore_unlock_mutex = os_semaphore_unlock_mutex_myown,

  .os_semaphore_create_binary = os_semaphore_create_binary_myown,
  .os_semaphore_delete_binary = os_semaphore_delete_binary_myown,
  .os_semaphore_wait_binary   = os_semaphore_wait_binary_myown,
  .os_semaphore_signal_binary = os_semaphore_signal_binary_myown,
};
#endif
/*=================底层硬件驱动依赖注入=======================*/
// 1. 组装 Driver 的依赖包（底层硬件）
static mpu6050_driver_input_api_t mpu6050_driver_api = {
  .p_iic_driver      = &iic_driver_interface,
  .p_interrupt = NULL, // 如果不用
  .p_delay           = &delay_interface,
  .p_timebase        = &timebase_interface,
  .p_buffer          = &buffer_interface,
  .p_yield           = &yield_interface,
  .p_os              = &os_interface
};
// 2. 组装 Handler 的依赖包（业务调度）
mpu6050_handler_input_args_t mpu6050_input_args = {
  .p_driver_api = &mpu6050_driver_api,  // 👈 把 Driver 包直接丢给 Handler
  .p_os         = &os_interface,         // Handler 自己也要用 OS 创建队列
  .p_timebase   = &timebase_interface    // Handler 预留
};