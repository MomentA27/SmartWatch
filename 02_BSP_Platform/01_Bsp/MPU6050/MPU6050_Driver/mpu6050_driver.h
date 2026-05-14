// // Created by 35540 on 2026/5/7. //
#ifndef SMARTWATCH_STM32F4_MPU6050_H
#define SMARTWATCH_STM32F4_MPU6050_H

//******************************** Includes *********************************//
#include <stdint.h>
#include "mpu6050_reg.h"
#include "mpu6050_reg_bit.h"
#include "elog.h"
#include "main.h"
//******************************** Includes *********************************//

//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define MPU6050Driver // 修正了拼写错误

#ifndef OS_SUPPORTING
#define OS_SUPPORTING 1 /* OS supporting，统一OS宏定义 */
#endif
//******************************** Defines **********************************//

//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
/**
 * @brief MPU6050 操作状态枚举
 */
typedef enum {
    MPU6050_OK = 0,             /* 操作成功完成 */
    MPU6050_ERROR = 1,          /* 运行时错误，未匹配到对应情况 */
    MPU6050_ERRORTIMEOUT = 2,   /* 操作超时失败 */
    MPU6050_ERRORRESOURCE = 3,  /* 资源不可用 */
    MPU6050_ERRORPARAMETER = 4, /* 参数错误 */
    MPU6050_ERRORNOMEMORY = 5,  /* 内存不足 */
    MPU6050_ERRORISR = 6,       /* 不允许在中断服务程序(ISR)中使用 */
    MPU6050_RESERVED = 0x7FFFFFFF, /* 保留 */
} mpu6050_status_t;

//******************************** Typedefs *********************************//

//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//

/**
 * @brief IIC 驱动接口结构体
 * 抽象了 I2C 通信所需的硬件相关操作
 */
typedef struct {
    void *hi2c;                   /* 指向IIC句柄的指针 */
    mpu6050_status_t (*pf_iic_init) (void *);       /* IIC初始化接口 */
    mpu6050_status_t (*pf_iic_deinit) (void *);     /* IIC反初始化接口 */
    //IIC写寄存器
    mpu6050_status_t (*pf_iic_mem_write)(void *hi2c, uint16_t dst_address, uint16_t mem_addr, uint16_t mem_size, uint8_t *p_data, uint16_t size, uint32_t timeout);
    // IIC读寄存器
    mpu6050_status_t (*pf_iic_mem_read) (void *hi2c, uint16_t dst_address, uint16_t mem_addr, uint16_t mem_size, uint8_t *p_data, uint16_t size, uint32_t timeout);
    //使用DMA异步读取寄存器
    mpu6050_status_t (*pf_iic_mem_read_dma) (void *hi2c, uint16_t dst_address, uint16_t mem_addr, uint16_t mem_size, uint8_t *p_data, uint16_t size );
} mpu6050_iic_driver_interface_t;

/**
 * @brief 硬件中断接口结构体
 * 用于抽象外部硬件中断和时钟的控制
 */
typedef struct {
    mpu6050_status_t (*pf_init) (void);             /* 硬件中断初始化 */
    mpu6050_status_t (*pf_deinit) (void);           /* 硬件中断反初始化 */
    mpu6050_status_t (*pf_enable_interrupt) (void);  /* 使能硬件中断 */
    mpu6050_status_t (*pf_disable_interrupt)(void);  /* 禁止硬件中断 */
    mpu6050_status_t (*pf_enable_clock) (void);      /* 使能时钟 */
    mpu6050_status_t (*pf_disable_clock) (void);     /* 禁止时钟 */
} hardware_interrupt_interface_t;

/**
 * @brief 时间基准接口结构体
 * 提供系统滴答计数值
 */
typedef struct {
    uint32_t (*pf_get_tick_count) (void);            /* 获取系统滴答计数值 */
} mpu6050_timebase_interface_t;

/**
 * @brief 缓冲区接口结构体
 * 用于管理数据处理过程中使用的缓冲区
 */
typedef struct {
    void      *p_ctx;                          // 把上下文收归到接口内部
    void     (*pf_buffer_init) (void *p_ctx,uint8_t size);
    uint8_t *(*pf_get_rbuffer_addr) (void *p_ctx);
    uint8_t *(*pf_get_wbuffer_addr) (void *p_ctx);
    void     (*pf_data_writed) (void *p_ctx);
    void     (*pf_data_readed) (void *p_ctx);
}mpu6050_buffer_interface_t;
/**
 * @brief 延时接口结构体
 * 提供微秒和毫秒级延时功能
 */
typedef struct {
    void (*pf_delay_init) (void);                    /* 延时初始化 */
    void (*pf_delay_us) (const uint32_t);            /* 微秒延时 */
    void (*pf_delay_ms) (const uint32_t);            /* 毫秒延时 */
} mpu6050_delay_interface_t;

#ifdef OS_SUPPORTING
/**
 * @brief 任务让出接口结构体
 * 用于RTOS下的无阻塞延时，让出CPU使用权
 */
typedef struct {
    void (*pf_rtos_yield) (const uint32_t);          /* OS任务让出延时 */
} mpu6050_yield_interface_t;

/**
 * @brief 操作系统接口结构体
 * 封装了队列、互斥锁、信号量等操作系统功能
 */
typedef struct {
    /* 队列操作 */
    mpu6050_status_t (*os_queue_create) (uint32_t const item_num, uint32_t const item_size, void ** const queue_handle);
    mpu6050_status_t (*os_queue_put) (void * const queue_handle, void * const item, uint32_t const timeout);
    mpu6050_status_t (*os_queue_put_isr) (void * const queue_handle, void * const item);
    mpu6050_status_t (*os_queue_get) (void * const queue_handle, void * const item, uint32_t const timeout);
    mpu6050_status_t (*os_queue_delete) (void * const queue_handle);
    /* 互斥锁操作 */
    mpu6050_status_t (*os_semaphore_create_mutex) (void **mutex_handle);
    mpu6050_status_t (*os_semaphore_delete_mutex) (void * const mutex_handle);
    mpu6050_status_t (*os_semaphore_lock_mutex) (void * const mutex_handle);
    mpu6050_status_t (*os_semaphore_unlock_mutex) (void * const mutex_handle);
    /* 二进制信号量操作 */
    mpu6050_status_t (*os_semaphore_create_binary) (void **binary_handle);
    mpu6050_status_t (*os_semaphore_delete_binary) (void * const binary_handle);
    mpu6050_status_t (*os_semaphore_wait_binary) (void * const binary_handle);
    mpu6050_status_t (*os_semaphore_signal_binary) (void * const binary_handle);
    /* 补充缺失的ISR信号量和任务通知接口 */
    mpu6050_status_t (*os_semaphore_signal_binary_isr) (void * const binary_handle, long * const HigherPriorityTaskWoken);
    mpu6050_status_t (*os_semaphore_signal_notify_isr) (void * const notify_handle, uint32_t ulValue, uint32_t eAction, long * const HigherPriorityTaskWoken);
    mpu6050_status_t (*os_semaphore_wait_notify) (uint32_t ulBitsToClearOnEntry, uint32_t ulBitsToClearOnExit, uint32_t *pulNotificationValue, uint32_t timeout);
} mpu6050_os_interface_t;
#endif /* End of OS_SUPPORTING */

/**
 * @brief MPU6050 数据结构体
 * 存储传感器原始数据、处理后的物理量及卡尔曼滤波角度
 */
typedef struct {
    int16_t accel_x_raw; double ax;
    int16_t accel_y_raw; double ay;
    int16_t accel_z_raw; double az;
    int16_t gyro_x_raw;  double gx;
    int16_t gyro_y_raw;  double gy;
    int16_t gyro_z_raw;  double gz;
    float temperature;
    double kalman_angle_x;
    double kalman_angle_y;
} mpu6050_data_t;

//**************************** Interface Structs ****************************//

//---------------------------------------------------------------------------//
//******************************** Classes **********************************//

/**
 * @brief MPU6050 驱动核心结构体
 * 集成了所有底层硬件接口、操作系统接口以及驱动功能函数
 */
typedef struct bsp_mpu6050_driver {
    /* 核心层接口 */
    mpu6050_iic_driver_interface_t *p_iic_driver_interface;
    hardware_interrupt_interface_t *p_interrupt_interface;
    mpu6050_delay_interface_t *p_delay_interface;
    mpu6050_timebase_interface_t *p_timebase_interface;

#ifdef OS_SUPPORTING
    /* 操作系统层接口 */
    mpu6050_yield_interface_t   *p_yield_interface;
    mpu6050_os_interface_t      *p_os_interface;
    mpu6050_buffer_interface_t  *p_buffer_interface;
    /* 操作系统对象句柄 */
    void *queue_handle;
    void *semaphore_mutex_handle;
    void *semaphore_binary_handle;
    void *notify_handle;  // 补充缺失的句柄
    /* 回调函数 */
    void (*pf_dma_completed_callback)(void);
    void (*pf_int_interrupt_callback)(void);
#endif /* End of OS_SUPPORTING */

    /* 驱动功能接口 */
    mpu6050_status_t (*pf_deinit) (struct bsp_mpu6050_driver *);
    mpu6050_status_t (*pf_sleep) (struct bsp_mpu6050_driver *);
    mpu6050_status_t (*pf_wakeup) (struct bsp_mpu6050_driver *);
    mpu6050_status_t (*pf_set_gyro_fsr) (struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_set_accel_fsr) (struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_set_lpf) (struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_set_rate) (struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_set_interrupt_enable)(struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_set_motion_threshold)(struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_set_INT_level) (struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_set_user_ctrl) (struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_set_pwr_mgmt1_reg) (struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_set_pwr_mgmt2_reg) (struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_set_fifo_en_reg) (struct bsp_mpu6050_driver *, uint8_t);
    mpu6050_status_t (*pf_get_temperature) (struct bsp_mpu6050_driver *, mpu6050_data_t *);
    mpu6050_status_t (*pf_get_accel) (struct bsp_mpu6050_driver *, mpu6050_data_t *);
    mpu6050_status_t (*pf_get_gyro) (struct bsp_mpu6050_driver *, mpu6050_data_t *);
    mpu6050_status_t (*pf_get_all_data) (struct bsp_mpu6050_driver *, mpu6050_data_t *);
    mpu6050_status_t (*pf_get_interrupt_status_reg)(struct bsp_mpu6050_driver *, uint8_t *);
    mpu6050_status_t (*pf_read_fifo_packet) (struct bsp_mpu6050_driver *p_mpu_driver, mpu6050_data_t *p_data);
    mpu6050_status_t (*pf_read_fifo_isr_occur) (struct bsp_mpu6050_driver *p_mpu_driver, mpu6050_data_t *p_data);
} bsp_mpu6050_driver_t;

//******************************** Classes **********************************//

//---------------------------------------------------------------------------//
//******************************** 函数声明 **********************************//

mpu6050_status_t bsp_mpu6050_driver_inst(
    bsp_mpu6050_driver_t *p_mpu6050_driver,
    mpu6050_iic_driver_interface_t *p_iic_driver_interface,
    mpu6050_buffer_interface_t *p_buffer_interface,
#ifdef OS_SUPPORTING
    mpu6050_yield_interface_t *p_yield_interface,
    mpu6050_os_interface_t *p_os_interfece,
#endif /* End of OS_SUPPORTING */
    mpu6050_delay_interface_t *p_delay_interface,
    mpu6050_timebase_interface_t *p_timebase_interface,
    void (*callback_register) (void (*callback)(void *, void *)),
    void (*callback_register_dma)(void (*callback)(void *, void *))
#ifdef OS_SUPPORTING
    ,void *queue_handle,
    void *semaphore_handle,
    void *notify_handle
#endif /* End of OS_SUPPORTING */
);

//******************************** 函数声明 **********************************//
#endif //SMARTWATCH_STM32F4_MPU6050_H
