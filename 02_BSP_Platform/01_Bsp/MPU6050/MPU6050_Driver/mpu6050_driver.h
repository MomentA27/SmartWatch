//
// Created by 35540 on 2026/5/7.
//

#ifndef SMARTWATCH_STM32F4_MPU6050_H
#define SMARTWATCH_STM32F4_MPU6050_H
//******************************** Includes *********************************//
#include <stdint.h>

#include "mpu6050_reg.h"
#include "mpu6050_reg_bit.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
/**
 * @brief MPU6050 操作状态枚举
 */
typedef enum
{
  MPU6050_OK             = 0,          /* 操作成功完成 */
  MPU6050_ERROR          = 1,          /* 运行时错误，未匹配到对应情况 */
  MPU6050_ERRORTIMEOUT   = 2,          /* 操作超时失败 */
  MPU6050_ERRORRESOURCE  = 3,          /* 资源不可用 */
  MPU6050_ERRORPARAMETER = 4,          /* 参数错误 */
  MPU6050_ERRORNOMEMORY  = 5,          /* 内存不足 */
  MPU6050_ERRORISR       = 6,          /* 不允许在中断服务程序(ISR)中使用 */
  MPU6050_RESERVED       = 0x7FFFFFFF, /* 保留 */
} mpu6050_status_t;
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//

/**
 * @brief IIC 驱动接口结构体
 * 抽象了 I2C 通信所需的硬件相关操作
 */
typedef struct
{
  void *hi2c;             /* 指向IIC句柄的指针 */
  mpu6050_status_t (*pf_iic_init)      (void *);   /* IIC初始化接口 */
  mpu6050_status_t (*pf_iic_deinit)    (void *);   /* IIC反初始化接口 */
  //IIC写寄存器
  mpu6050_status_t (*pf_iic_mem_write)(void *hi2c,
                                      uint16_t dst_address,
                                      uint16_t mem_addr,
                                      uint16_t mem_size,
                                      uint8_t  *p_data,
                                      uint16_t size,
                                      uint32_t timeout);
  // IIC读寄存器
  mpu6050_status_t (*pf_iic_mem_read) (void *hi2c,
                                      uint16_t dst_address,
                                      uint16_t mem_addr,
                                      uint16_t mem_size,
                                      uint8_t  *p_data,
                                      uint16_t size,
                                      uint32_t timeout);
  //使用DMA异步读取寄存器
  mpu6050_status_t (*pf_iic_mem_read_dma)
                                     (void *hi2c,
                                      uint16_t dst_address,
                                      uint16_t mem_addr,
                                      uint16_t mem_size,
                                      uint8_t  *p_data,
                                      uint16_t size );
} mpu6050_iic_driver_interface_t;

/**
 * @brief 硬件中断接口结构体
 * 用于抽象外部硬件中断和时钟的控制
 */
typedef struct
{
  mpu6050_status_t (*pf_init)             (void);   /* 硬件中断初始化 */
  mpu6050_status_t (*pf_deinit)           (void);   /* 硬件中断反初始化 */
  mpu6050_status_t (*pf_enable_interrupt) (void);   /* 使能硬件中断 */
  mpu6050_status_t (*pf_disable_interrupt)(void);   /* 禁止硬件中断 */
  mpu6050_status_t (*pf_enable_clock)     (void);   /* 使能时钟 */
  mpu6050_status_t (*pf_disable_clock)    (void);   /* 禁止时钟 */
} hardware_interrupt_interface_t;

/**
 * @brief 时间基准接口结构体
 * 提供系统滴答计数值
 */
typedef struct
{
  uint32_t (*pf_get_tick_count) (void);        /* 获取系统滴答计数值 */
} mpu6050_timebase_interface_t;

/**
 * @brief 缓冲区接口结构体
 * 用于管理数据处理过程中使用的缓冲区
 */
typedef struct
{
  uint8_t *(*pf_buffer_init) (uint8_t size);   /* 初始化缓冲区，指定大小 */
  uint8_t *(*pf_get_rbuffer_addr) (void);      /* 获取读缓冲区地址 */
  uint8_t *(*pf_get_wbuffer_addr) (void);      /* 获取写缓冲区地址 */
} buffer_interface_t;

/**
 * @brief 延时接口结构体
 * 提供微秒和毫秒级延时功能
 */
typedef struct
{
  void (*pf_delay_init) (void);               /* 延时初始化 */
  void (*pf_delay_us) (const uint32_t);       /* 微秒延时 */
  void (*pf_delay_ms) (const uint32_t);       /* 毫秒延时 */
} mpu6050_delay_interface_t;

/**
 * @brief 任务让出接口结构体
 * 用于RTOS下的无阻塞延时，让出CPU使用权
 */
typedef struct
{
  void (*pf_rtos_yield) (const uint32_t);         /* OS任务让出延时 */
} mpu6050_yield_interface_t;

/**
 * @brief 操作系统接口结构体
 * 封装了队列、互斥锁、信号量等操作系统功能
 */
typedef struct
{
  /* 队列操作 */
  mpu6050_status_t (*os_queue_create)
                                  (uint32_t const item_num,
                                  uint32_t const item_size,
                                  void ** const queue_handle);          /* 创建队列 */
  mpu6050_status_t (*os_queue_put)
                                  (void * const queue_handle,
                                  void * const item,
                                  uint32_t const timeout);              /* 向队列发送数据 */
  mpu6050_status_t (*os_queue_put_isr)
                  (void * const queue_handle,
                  void * const item,
                  long * const HigherPriorityTaskWoken);                /* 在ISR中向队列发送数据 */
  mpu6050_status_t (*os_queue_get)
                                  (void * const queue_handle,
                                  void * const item,
                                  uint32_t const timeout);              /* 从队列接收数据 */
  mpu6050_status_t (*os_queue_delete)
                                  (void * const queue_handle);          /* 删除队列 */

  /* 互斥锁操作 */
  mpu6050_status_t (*os_semaphore_create_mutex) (void **mutex_handle);  /* 创建互斥锁 */
  mpu6050_status_t (*os_semaphore_delete_mutex) (void * const mutex_handle); /* 删除互斥锁 */
  mpu6050_status_t (*os_semaphore_lock_mutex)   (void * const mutex_handle);  /* 加锁互斥锁 */
  mpu6050_status_t (*os_semaphore_unlock_mutex) (void * const mutex_handle);  /* 解锁互斥锁 */

  /* 二进制信号量操作 */
  mpu6050_status_t (*os_semaphore_create_binary) (void **binary_handle);      /* 创建二进制信号量 */
  mpu6050_status_t (*os_semaphore_delete_binary) (void * const binary_handle);/* 删除二进制信号量 */
  mpu6050_status_t (*os_semaphore_wait_binary)   (void * const binary_handle);/* 等待二进制信号量 */
  mpu6050_status_t (*os_semaphore_signal_binary) (void * const binary_handle);/* 发送二进制信号量 */
} mpu6050_os_interface_t;

/**
 * @brief MPU6050 数据结构体
 * 存储传感器原始数据、处理后的物理量及卡尔曼滤波角度
 */
typedef struct
{
  /* 加速度计原始数据 */
  int16_t accel_x_raw;    /* X轴加速度原始值 */
  int16_t accel_y_raw;    /* Y轴加速度原始值 */
  int16_t accel_z_raw;    /* Z轴加速度原始值 */

  /* 处理后的加速度数据，单位：g */
  double ax;              /* X轴加速度（g） */
  double ay;              /* Y轴加速度（g） */
  double az;              /* Z轴加速度（g） */

  /* 陀螺仪原始数据 */
  int16_t gyro_x_raw;     /* X轴陀螺仪原始值 */
  int16_t gyro_y_raw;     /* Y轴陀螺仪原始值 */
  int16_t gyro_z_raw;     /* Z轴陀螺仪原始值 */

  /* 处理后的陀螺仪数据，单位：度/秒 */
  double gx;              /* X轴角速度（°/s） */
  double gy;              /* Y轴角速度（°/s） */
  double gz;              /* Z轴角速度（°/s） */

  /* 温度读数，单位：摄氏度 */
  float temperature;

  /* 卡尔曼滤波处理后的角度 */
  double kalman_angle_x;  /* X轴角度 */
  double kalman_angle_y;  /* Y轴角度 */
} mpu6050_data_t;
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//

/**
 * @brief MPU6050 驱动核心结构体
 * 集成了所有底层硬件接口、操作系统接口以及驱动功能函数
 */
typedef struct bsp_mpu6050_driver
{
    /* 核心层接口 */
    mpu6050_iic_driver_interface_t           *p_iic_driver_interface;   /* IIC驱动接口 */
    hardware_interrupt_interface_t           *p_interrupt_interface;    /* 硬件中断接口 */
    mpu6050_delay_interface_t                *p_delay_interface;        /* 延时接口 */
    mpu6050_timebase_interface_t             *p_timebase_interface;     /* 时间基准接口 */

    /* 操作系统层接口 */
    mpu6050_yield_interface_t  *p_yield_interface;    /* 任务让出接口 */
    mpu6050_os_interface_t     *p_os_interface;       /* OS接口 */
    buffer_interface_t         *p_buffer_interface;   /* 缓冲区接口 */

    /* 操作系统对象句柄 */
    void *queue_handle;               /* 队列句柄 */
    void *semaphore_mutex_handle;     /* 互斥锁句柄 */
    void *semaphore_binary_handle;    /* 二进制信号量句柄 */

    /* 回调函数 */
    void (*pf_dma_completed_callback)(void);   /* DMA传输完成回调 */
    void (*pf_int_interrupt_callback)(void);   /* 外部中断回调 */

    /* 驱动功能接口 */
    mpu6050_status_t (*pf_deinit)              (struct bsp_mpu6050_driver *);   /* 反初始化 */
    mpu6050_status_t (*pf_sleep)               (struct bsp_mpu6050_driver *);   /* 进入睡眠模式 */
    mpu6050_status_t (*pf_wakeup)              (struct bsp_mpu6050_driver *);   /* 唤醒 */
    mpu6050_status_t (*pf_set_gyro_fsr)        (struct bsp_mpu6050_driver *, uint8_t); /* 设置陀螺仪满量程 */
    mpu6050_status_t (*pf_set_accel_fsr)       (struct bsp_mpu6050_driver *, uint8_t); /* 设置加速度计满量程 */
    mpu6050_status_t (*pf_set_lpf)             (struct bsp_mpu6050_driver *, uint8_t); /* 设置数字低通滤波器 */
    mpu6050_status_t (*pf_set_rate)            (struct bsp_mpu6050_driver *, uint8_t); /* 设置采样率 */
    mpu6050_status_t (*pf_set_interrupt_enable)(struct bsp_mpu6050_driver *, uint8_t); /* 使能/禁止中断 */
    mpu6050_status_t (*pf_set_motion_threshold)(struct bsp_mpu6050_driver *, uint8_t); /* 设置运动检测阈值 */
    mpu6050_status_t (*pf_set_INT_level)       (struct bsp_mpu6050_driver *, uint8_t); /* 设置中断触发电平 */
    mpu6050_status_t (*pf_set_user_ctrl)       (struct bsp_mpu6050_driver *, uint8_t); /* 设置用户控制寄存器 */
    mpu6050_status_t (*pf_set_pwr_mgmt1_reg)   (struct bsp_mpu6050_driver *, uint8_t); /* 设置电源管理1寄存器 */
    mpu6050_status_t (*pf_set_pwr_mgmt2_reg)   (struct bsp_mpu6050_driver *, uint8_t); /* 设置电源管理2寄存器 */
    mpu6050_status_t (*pf_set_fifo_en_reg)     (struct bsp_mpu6050_driver *, uint8_t); /* 设置FIFO使能寄存器 */
    mpu6050_status_t (*pf_get_temperature)     (struct bsp_mpu6050_driver *, mpu6050_data_t *); /* 获取温度 */
    mpu6050_status_t (*pf_get_accel)           (struct bsp_mpu6050_driver *, mpu6050_data_t *); /* 获取加速度计数据 */
    mpu6050_status_t (*pf_get_gyro)            (struct bsp_mpu6050_driver *, mpu6050_data_t *); /* 获取陀螺仪数据 */
    mpu6050_status_t (*pf_get_all_data)        (struct bsp_mpu6050_driver *, mpu6050_data_t *); /* 获取所有传感器数据 */
    mpu6050_status_t (*pf_get_interrupt_status_reg)(struct bsp_mpu6050_driver *, uint8_t *); /* 读取中断状态寄存器 */
    mpu6050_status_t (*pf_read_fifo_packet)    (struct bsp_mpu6050_driver *p_mpu_driver,    /* 从FIFO读取一个完整数据包 */
                                                                           mpu6050_data_t *p_data);
    mpu6050_status_t (*pf_read_fifo_isr_occur) (struct bsp_mpu6050_driver *p_mpu_driver,    /* 在ISR中读取FIFO数据包（发生中断时使用） */
                                                                           mpu6050_data_t *p_data);
} bsp_mpu6050_driver_t;
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//**************************** Extern Variables *****************************//
//**************************** Extern Variables *****************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 **********************************//
/**
 * @brief 使用提供的接口和回调函数初始化 MPU6050 驱动实例
 *
 * @param p_mpu6050_driver    指向待初始化的 MPU6050 驱动结构体的指针
 * @param p_iic_driver_interface 指向 IIC 驱动接口实现的指针
 * @param p_yield_interface      指向任务让出接口的指针（仅 OS 模式下使用）
 * @param p_os_interfece         指向 OS 接口的指针（仅 OS 模式下使用）
 * @param p_delay_interface      指向延时接口的指针
 * @param p_timebase_interface   指向时间基准接口的指针
 * @param callback_register      用于注册中断回调的函数
 * @param callback_register_dma  用于注册 DMA 回调的函数
 * @param queue_handle           用于数据传输的队列句柄（仅 OS 模式下使用）
 *
 * @return 成功时返回 MPU6050_OK，否则返回错误代码
 */
mpu6050_status_t bsp_mpu6050_driver_inst(
                bsp_mpu6050_driver_t    *p_mpu6050_driver,
                mpu6050_iic_driver_interface_t *p_iic_driver_interface,
                mpu6050_yield_interface_t      *p_yield_interface,
                mpu6050_os_interface_t         *p_os_interfece,
                mpu6050_delay_interface_t      *p_delay_interface,
                mpu6050_timebase_interface_t   *p_timebase_interface,
                void (*callback_register)    (void (*callback)(void *, void *)),
                void (*callback_register_dma)(void (*callback)(void *, void *)),
                void *queue_handle
                );
//******************************** 函数声明 **********************************//
#endif //SMARTWATCH_STM32F4_MPU6050_H