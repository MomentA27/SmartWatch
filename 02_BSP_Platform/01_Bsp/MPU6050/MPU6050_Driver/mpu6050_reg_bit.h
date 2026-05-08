/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file mpu6050.h
 *
 * @par dependencies
 *
 * - stdint.h
 *
 * @author liu
 *
 * @brief Provide the register bit definitions of MPU6050.
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.0 2024-12-10
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
#ifndef __BSP_MPU6050_REG_BIT_H__
#define __BSP_MPU6050_REG_BIT_H__


/* MPU_SELF_TEST_X_REG (0x0D) ************************************************/
// 读/写寄存器
// XA_TEST: X轴加速度计自检使能
// XG_TEST: X轴陀螺仪自检使能
#define XA_TEST_BIT(x)     (x << 5)
#define XG_TEST_BIT(x)     (x << 0)

/* MPU_SELF_TEST_Y_REG (0x0E) ************************************************/
// 读/写寄存器
// YA_TEST: Y轴加速度计自检使能
// YG_TEST: Y轴陀螺仪自检使能
#define YA_TEST_BIT(x)     (x << 5)
#define YG_TEST_BIT(x)     (x << 0)

/* MPU_SELF_TEST_Z_REG (0x0F) ************************************************/
// 读/写寄存器
// ZA_TEST: Z轴加速度计自检使能
// ZG_TEST: Z轴陀螺仪自检使能
#define ZA_TEST_BIT(x)     (x << 5)
#define ZG_TEST_BIT(x)     (x << 0)

/* MPU_SELF_TEST_A_REG (0x10) ************************************************/
// 读/写寄存器
// XA_TEST: X轴加速度计自检微调位
// YA_TEST: Y轴加速度计自检微调位
// ZA_TEST: Z轴加速度计自检微调位
#define XA_TEST_BIT1(x)    (x << 4)
#define YA_TEST_BIT1(x)    (x << 2)
#define ZA_TEST_BIT1(x)    (x << 0)

/* MPU_SAMPLE_RATE_REG (0x19) ************************************************/
// 读/写寄存器
// 采样率 = 陀螺仪输出频率 / (1 + SMPLRT_DIV)
// 其中陀螺仪输出频率在DLPF禁用（DLPF_CFG = 0 或 7）时为8kHz，
// DLPF启用（DLPF_CFG = 1 或 2）时为1kHz
#define SMPLRT_DIV_BIT(x) (x << 0)

/* MPU_CFG_REG (0x1A) ********************************************************/
// 读/写寄存器
// DLPF_CFG: 3位无符号值。配置数字低通滤波器（DLPF）
#define DLPF_CFG_BIT(x)     (x << 0)
// --------------------------------------------------------------------
// DLPF_CFG     |     加速度计               |           陀螺仪        |
//              |     (Fs = 1kHz)            |                         |
//              |--------------|-------------|-------------------------|
//              | 带宽 (Hz)    | 延迟 (ms)   | 带宽(Hz) | 延迟(ms) | Fs(kHz)|
// --------------------------------------------------------------------
// 0            |  260          |   0        |    256   | 0.98     | 8    |
// 1            |  184          |   2        |    188   | 1.9      | 1    |
// 2            |  94           |   3        |    98    | 2.8      | 1    |
// 3            |  44           |   4.9      |    42    | 4.8      | 1    |
// 4            |  21           |   8.5      |    20    | 8.3      | 1    |
// 5            |  10           |   13.8     |    10    | 13.4     | 1    |
// 6            |  5            |   19.0     |    5     | 18.6     | 1    |
// 7            |  保留                     | 保留              | 8    |
// --------------------------------------------------------------------
// EXT_SYNC_SET: 3位无符号值。启用FSYNC引脚数据采样
#define EXT_SYNC_SET_BIT(x) (x << 3)

/* MPU_GYRO_CFG_REG (0x1B) ***************************************************/
// 读/写寄存器
// FS_SEL: 2位无符号值。选择陀螺仪满量程范围
//   0 = ±250  °/s
//   1 = ±500  °/s
//   2 = ±1000 °/s
//   3 = ±2000 °/s
// XG_ST / YG_ST / ZG_ST: 各轴陀螺仪自检使能
#define FS_SEL_BIT(x) (x << 3)
#define ZG_ST_BIT(x)  (x << 5)
#define YG_ST_BIT(x)  (x << 6)
#define XG_ST_BIT(x)  (x << 7)

/* MPU_ACCEL_CFG_REG (0x1C) **************************************************/
// 读/写寄存器
// AFS_SEL: 2位无符号值。选择加速度计满量程范围
//   0 = ±2g
//   1 = ±4g
//   2 = ±8g
//   3 = ±16g
// XA_ST / YA_ST / ZA_ST: 各轴加速度计自检使能
#define AFS_SEL_BIT(x) (x << 3)
#define ZA_ST_BIT(x)   (x << 5)
#define YA_ST_BIT(x)   (x << 6)
#define XA_ST_BIT(x)   (x << 7)

/* MPU_MOTION_DET_REG (0x1F) *************************************************/
// 读/写寄存器。指定运动检测阈值
#define MOT_THR_BIT(X) (X << 0)

/* MPU_FIFO_EN_REG (0x23) ****************************************************/
// 读/写寄存器。使能特定传感器数据写入FIFO缓冲区
// TEMP_FIFO_EN:   温度传感器数据写入FIFO
// XG_FIFO_EN:     X轴陀螺仪数据写入FIFO
// YG_FIFO_EN:     Y轴陀螺仪数据写入FIFO
// ZG_FIFO_EN:     Z轴陀螺仪数据写入FIFO
// ACCEL_FIFO_EN:  加速度计数据（全部3轴）写入FIFO
// SLV2_FIFO_EN:   I2C从机2数据写入FIFO
// SLV1_FIFO_EN:   I2C从机1数据写入FIFO
// SLV0_FIFO_EN:   I2C从机0数据写入FIFO
#define TEMP_FIFO_EN_BIT(x) (x << 7)
#define XG_FIFO_EN_BIT(x)   (x << 6)
#define YG_FIFO_EN_BIT(x)   (x << 5)
#define ZG_FIFO_EN_BIT(x)   (x << 4)
#define ACCEL_FIFO_EN_BIT(x)(x << 3)
#define SLV2_FIFO_EN_BIT(x) (x << 2)
#define SLV1_FIFO_EN_BIT(x) (x << 1)
#define SLV0_FIFO_EN_BIT(x) (x << 0)

/* MPU_I2C_MST_CTRL_REG (0x24) ***********************************************/
// 读/写寄存器。I2C主模式控制寄存器
// MULT_MST_EN:   使能多主机功能
// WAIT_FOR_ES:   等待外部传感器数据就绪后再产生中断
// SLV3_FIFO_EN:  从机3数据写入FIFO
// I2C_MST_P_NSR: I2C主机在从机间切换方式: 0=重启动, 1=停止后启动
// I2C_MST_CLK:   5位I2C主机时钟分频器
#define MULT_MST_EN_BIT(x)  (x << 7)
#define WAIT_FOR_ES_BIT(x)  (x << 6)
#define SLV3_FIFO_EN_BIT(x) (x << 5)
#define I2C_MST_P_NSR_BIT(x)(x << 4)
#define I2C_MST_CLK_BIT(x)  (x << 0)

/* MPU_I2C_SLV0_ADDR_REG (0x25) **********************************************/
// 读/写寄存器。I2C从机0地址及读/写控制
// I2C_SLV0_RW:   0=写, 1=读
// I2C_SLV0_ADDR: 7位I2C从机地址
#define I2C_SLV0_RW_BIT(x)  (x << 7)
#define I2C_SLV0_ADDR_BIT(x)(x << 0)

/* MPU_I2C_SLV0_REG_REG (0x26) ***********************************************/
// 读/写寄存器。I2C从机0待读/写的寄存器地址
#define I2C_SLV0_REG_BIT(x) (x << 0)

/* MPU_I2C_SLV0_CTRL_REG (0x27) **********************************************/
// 读/写寄存器。I2C从机0控制寄存器
// I2C_SLV0_EN:      使能从机0传输
// I2C_SLV0_BYTE_SW: 读取16位寄存器时交换高低字节
// I2C_SLV0_REG_DIS: 禁止寄存器地址写入
// I2C_SLV0_GRP:     将从机0与下一个从机（奇数编号）编组
// I2C_SLV0_LEN:     4位传输字节数
#define I2C_SLV0_EN_BIT(x)     (x << 7)
#define I2C_SLV0_BYTE_SW_BIT(x)(x << 6)
#define I2C_SLV0_REG_DIS_BIT(x)(x << 5)
#define I2C_SLV0_GRP_BIT(x)    (x << 4)
#define I2C_SLV0_LEN_BIT(x)    (x << 0)

/* MPU_I2C_SLV1_ADDR_REG (0x28) **********************************************/
// 读/写寄存器。I2C从机1地址及读/写控制
// I2C_SLV1_RW:   0=写, 1=读
// I2C_SLV1_ADDR: 7位I2C从机地址
#define I2C_SLV1_RW_BIT(x)  (x << 7)
#define I2C_SLV1_ADDR_BIT(x)(x << 0)

/* MPU_I2C_SLV1_REG_REG (0x29) ***********************************************/
// 读/写寄存器。I2C从机1待读/写的寄存器地址
#define I2C_SLV1_REG_BIT(x) (x << 0)

/* MPU_I2C_SLV1_CTRL_REG (0x2A) **********************************************/
// 读/写寄存器。I2C从机1控制寄存器
// I2C_SLV1_EN:      使能从机1传输
// I2C_SLV1_BYTE_SW: 读取16位寄存器时交换高低字节
// I2C_SLV1_REG_DIS: 禁止寄存器地址写入
// I2C_SLV1_GRP:     将从机1与下一个从机（奇数编号）编组
// I2C_SLV1_LEN:     4位传输字节数
#define I2C_SLV1_EN_BIT(x)     (x << 7)
#define I2C_SLV1_BYTE_SW_BIT(x)(x << 6)
#define I2C_SLV1_REG_DIS_BIT(x)(x << 5)
#define I2C_SLV1_GRP_BIT(x)    (x << 4)
#define I2C_SLV1_LEN_BIT(x)    (x << 0)

/* MPU_I2C_SLV2_ADDR_REG (0x2B) **********************************************/
// 读/写寄存器。I2C从机2地址及读/写控制
// I2C_SLV2_RW:   0=写, 1=读
// I2C_SLV2_ADDR: 7位I2C从机地址
#define I2C_SLV2_RW_BIT(x)  (x << 7)
#define I2C_SLV2_ADDR_BIT(x)(x << 0)

/* MPU_I2C_SLV2_REG_REG (0x2C) ***********************************************/
// 读/写寄存器。I2C从机2待读/写的寄存器地址
#define I2C_SLV2_REG_BIT(x) (x << 0)

/* MPU_I2C_SLV2_CTRL_REG (0x2D) **********************************************/
// 读/写寄存器。I2C从机2控制寄存器
// I2C_SLV2_EN:      使能从机2传输
// I2C_SLV2_BYTE_SW: 读取16位寄存器时交换高低字节
// I2C_SLV2_REG_DIS: 禁止寄存器地址写入
// I2C_SLV2_GRP:     将从机2与下一个从机（奇数编号）编组
// I2C_SLV2_LEN:     4位传输字节数
#define I2C_SLV2_EN_BIT(x)     (x << 7)
#define I2C_SLV2_BYTE_SW_BIT(x)(x << 6)
#define I2C_SLV2_REG_DIS_BIT(x)(x << 5)
#define I2C_SLV2_GRP_BIT(x)    (x << 4)
#define I2C_SLV2_LEN_BIT(x)    (x << 0)

/* MPU_I2C_SLV3_ADDR_REG (0x2E) **********************************************/
// 读/写寄存器。I2C从机3地址及读/写控制
// I2C_SLV3_RW:   0=写, 1=读
// I2C_SLV3_ADDR: 7位I2C从机地址
#define I2C_SLV3_RW_BIT(x)  (x << 7)
#define I2C_SLV3_ADDR_BIT(x)(x << 0)

/* MPU_I2C_SLV3_REG_REG (0x2F) ***********************************************/
// 读/写寄存器。I2C从机3待读/写的寄存器地址
#define I2C_SLV3_REG_BIT(x) (x << 0)

/* MPU_I2C_SLV3_CTRL_REG (0x30) **********************************************/
// 读/写寄存器。I2C从机3控制寄存器
// I2C_SLV3_EN:      使能从机3传输
// I2C_SLV3_BYTE_SW: 读取16位寄存器时交换高低字节
// I2C_SLV3_REG_DIS: 禁止寄存器地址写入
// I2C_SLV3_GRP:     将从机3与下一个从机（奇数编号）编组
// I2C_SLV3_LEN:     4位传输字节数
#define I2C_SLV3_EN_BIT(x)     (x << 7)
#define I2C_SLV3_BYTE_SW_BIT(x)(x << 6)
#define I2C_SLV3_REG_DIS_BIT(x)(x << 5)
#define I2C_SLV3_GRP_BIT(x)    (x << 4)
#define I2C_SLV3_LEN_BIT(x)    (x << 0)

/* MPU_I2C_SLV4_ADDR_REG (0x31) **********************************************/
// 读/写寄存器。I2C从机4地址及读/写控制
// I2C_SLV4_RW:   0=写, 1=读
// I2C_SLV4_ADDR: 7位I2C从机地址
#define I2C_SLV4_RW_BIT(x)  (x << 7)
#define I2C_SLV4_ADDR_BIT(x)(x << 0)

/* MPU_I2C_SLV4_REG_REG (0x32) ***********************************************/
// 读/写寄存器。I2C从机4待读/写的寄存器地址
#define I2C_SLV4_REG_BIT(x) (x << 0)

/* MPU_I2C_SLV4_DO_REG (0x33) ************************************************/
// 读/写寄存器。I2C从机4数据输出寄存器
#define I2C_SLV4_DO_BIT(x)  (x << 0)

/* MPU_I2C_SLV4_CTRL_REG (0x34) **********************************************/
// 读/写寄存器。I2C从机4控制寄存器
// I2C_SLV4_EN:      使能从机4传输
// I2C_SLV4_INT_EN:  从机4传输完成时产生中断
// I2C_SLV4_REG_DIS: 禁止寄存器地址写入
// I2C_MST_DLY:      5位I2C主机访问延迟
#define I2C_SLV4_EN_BIT(x)     (x << 7)
#define I2C_SLV4_INT_EN_BIT(x) (x << 6)
#define I2C_SLV4_REG_DIS_BIT(x)(x << 5)
#define I2C_MST_DLY_BIT(x)     (x << 0)

/* MPU_I2C_SLV4_DI_REG (0x35) ************************************************/
// 读/写寄存器。I2C从机4数据输入寄存器
#define I2C_SLV4_DI_BIT(x)  (x << 0)

/* MPU_I2C_MST_STATUS_REG (0x36) *********************************************/
// 只读寄存器。指示I2C主机传输状态
// PASS_THROUGH:    I2C主机处于直通模式（FSYNC用作I2C）
// I2C_SLV4_DONE:  从机4传输完成
// I2C_LOST_ARB:   I2C主机失去仲裁
// I2C_SLV4_NACK:  从机4返回NACK
// I2C_SLV3_NACK:  从机3返回NACK
// I2C_SLV2_NACK:  从机2返回NACK
// I2C_SLV1_NACK:  从机1返回NACK
// I2C_SLV0_NACK:  从机0返回NACK
#define PASS_THROUGH_BIT    (1 << 7)
#define I2C_SLV4_DONE_BIT   (1 << 6)
#define I2C_LOST_ARB_BIT    (1 << 5)
#define I2C_SLV4_NACK_BIT   (1 << 4)
#define I2C_SLV3_NACK_BIT   (1 << 3)
#define I2C_SLV2_NACK_BIT   (1 << 2)
#define I2C_SLV1_NACK_BIT   (1 << 1)
#define I2C_SLV0_NACK_BIT   (1 << 0)

/* MPU_INTBP_CFG_REG (0x37) **************************************************/
// 读/写寄存器。INT引脚 / 旁路使能配置
// INT_LEVEL:       当此位为0时，INT引脚逻辑电平为高有效。
//                  当此位为1时，INT引脚逻辑电平为低有效。
// INT_OPEN:        当此位为0时，INT引脚配置为推挽输出。
//                  当此位为1时，INT引脚配置为开漏输出。
// LATCH_INT_EN:    当此位为0时，INT引脚发出50us脉冲。
//                  当此位为1时，INT引脚保持高电平直到中断被清除。
// INT_RD_CLEAR:    当此位为0时，中断状态位仅在读取INT_STATUS时清除。
//                  当此位为1时，中断状态位在任何读操作时清除。
// FSYNC_INT_LEVEL: 当此位为0时，FSYNC引脚（用作主机中断）的逻辑电平为高有效。
//                  当此位为1时，FSYNC引脚（用作主机中断）的逻辑电平为低有效。
// FSYNC_INT_EN:    当此位为0时，禁止FSYNC引脚向主机产生中断。
//                  当此位为1时，使能FSYNC引脚用作主机中断。
// I2C_BYPASS_EN:   当此位为1时，主机应用处理器可通过MPU6050直接访问辅助I2C总线。
#define I2C_BYPASS_EN_BIT(X)   (X << 1)
#define FSYNC_INT_EN_BIT(X)    (X << 2)
#define FSYNC_INT_LEVEL_BIT(X) (X << 3)
#define INT_RD_CLEAR_BIT(X)    (X << 4)
#define LATCH_INT_EN_BIT(X)    (X << 5)
#define INT_OPEN_BIT(X)        (X << 6)
#define INT_LEVEL_BIT(X)       (X << 7)


/* MPU_INT_EN_REG (0x38) *****************************************************/
// 读/写寄存器。使能特定中断源
// DATA_RDY_EN:     当设置为1时，使能数据就绪中断。每次所有传感器寄存器写操作完成时触发。
// I2C_MST_INT_EN:  当设置为1时，使能任意I2C主机中断源产生中断。
// FIFO_OFLOW_EN:   当设置为1时，使能FIFO缓冲区溢出产生中断。
// MOT_EN:          当设置为1时，使能运动检测产生中断。
#define DATA_RDY_EN_BIT(x)      (x << 0)
#define I2C_MST_INT_EN_BIT(x)   (x << 3)
#define FIFO_OVERFLOW_EN_BIT(x) (x << 4)
#define MOT_EN_BIT(x)           (x << 6)

/* MPU_INT_STA_REG (0x3A) ****************************************************/
// 只读寄存器。指示中断状态
// DATA_RDY_INT:     数据就绪中断已发生（所有传感器寄存器已更新）
// I2C_MST_INT:      I2C主机中断已发生
// FIFO_OVERFLOW_INT: FIFO缓冲区溢出中断已发生
// MOT_INT:          运动检测中断已发生
#define DATA_RDY_INT_BIT      (1 << 0)
#define I2C_MST_INT_BIT       (1 << 3)
#define FIFO_OVERFLOW_INT_BIT (1 << 4)
#define MOT_INT_BIT           (1 << 6)

/* MPU_ACCEL_XOUT_H_REG (0x3B) ***********************************************/
// 只读寄存器。加速度计X轴测量值高字节
/* MPU_ACCEL_XOUT_L_REG (0x3C) ***********************************************/
// 只读寄存器。加速度计X轴测量值低字节
/* MPU_ACCEL_YOUT_H_REG (0x3D) ***********************************************/
// 只读寄存器。加速度计Y轴测量值高字节
/* MPU_ACCEL_YOUT_L_REG (0x3E) ***********************************************/
// 只读寄存器。加速度计Y轴测量值低字节
/* MPU_ACCEL_ZOUT_H_REG (0x3F) ***********************************************/
// 只读寄存器。加速度计Z轴测量值高字节
/* MPU_ACCEL_ZOUT_L_REG (0x40) ***********************************************/
// 只读寄存器。加速度计Z轴测量值低字节

/* MPU_TEMP_OUT_H_REG (0x41) *************************************************/
// 只读寄存器。温度测量值高字节
// 温度计算公式: Temperature = (TEMP_OUT / 340) + 36.53 [°C]
/* MPU_TEMP_OUT_L_REG (0x42) *************************************************/
// 只读寄存器。温度测量值低字节

/* MPU_GYRO_XOUT_H_REG (0x43) ************************************************/
// 只读寄存器。陀螺仪X轴测量值高字节
/* MPU_GYRO_XOUT_L_REG (0x44) ************************************************/
// 只读寄存器。陀螺仪X轴测量值低字节
/* MPU_GYRO_YOUT_H_REG (0x45) ************************************************/
// 只读寄存器。陀螺仪Y轴测量值高字节
/* MPU_GYRO_YOUT_L_REG (0x46) ************************************************/
// 只读寄存器。陀螺仪Y轴测量值低字节
/* MPU_GYRO_ZOUT_H_REG (0x47) ************************************************/
// 只读寄存器。陀螺仪Z轴测量值高字节
/* MPU_GYRO_ZOUT_L_REG (0x48) ************************************************/
// 只读寄存器。陀螺仪Z轴测量值低字节

/* MPU_I2CMST_DELAY_REG (0x67) ***********************************************/
// 读/写寄存器。I2C主机延迟影子控制
// I2C_SLVx_DLY_EN:  使能对应从机的影子传输延迟
// DELAY_ES_SHADOW:  延迟外部传感器影子传输，直到所有数据接收完毕
#define I2C_SELV0_DLY_EN_BIT(x) (x << 0)
#define I2C_SELV1_DLY_EN_BIT(x) (x << 1)
#define I2C_SELV2_DLY_EN_BIT(x) (x << 2)
#define I2C_SELV3_DLY_EN_BIT(x) (x << 3)
#define I2C_SELV4_DLY_EN_BIT(x) (x << 4)
#define DELAY_ES_SHADOW_BIT(x)  (x << 7)

/* MPU_SIGPATH_RST_REG (0x68) ***********************************************/
// 只写寄存器。复位传感器信号通路
// TEMP_RESET:  复位温度传感器数字信号通路
// ACCEL_RESET: 复位加速度计数字信号通路
// GYRO_RESET:  复位陀螺仪数字信号通路
#define TEMP_RESET_BIT(x)  (x << 0)
#define ACCEL_RESET_BIT(x) (x << 1)
#define GYRO_RESET_BIT(x)  (x << 2)

/* MPU_MDETECT_CTRL_REG (0x69) ***********************************************/
// 读/写寄存器。运动检测控制
// ACCEL_ON_DELAY: 确认运动检测的采样计数器
#define ACCEL_ON_DELAY_BIT(x) (x << 4)

/* MPU_USER_CTRL_REG (0x6A) **************************************************/
// 读/写寄存器。用户控制寄存器
// FIFO_EN:          使能FIFO操作
// I2C_MST_EN:       使能I2C主模式
// I2C_IF_DIS:       禁用I2C总线接口（使用SPI）
// FIFO_RESET:       复位FIFO缓冲区（自动清除）
// I2C_MST_RESET:    复位I2C主机状态机（自动清除）
// SIG_COND_RESET:   复位所有传感器信号通路和FIFO（自动清除）
#define SIG_COND_RESET_BIT(x) (x << 0)
#define I2C_MST_RESET_BIT(x)  (x << 1)
#define FIFO_RESET_BIT(x)     (x << 2)
#define I2C_IF_DIS_BIT(x)     (x << 4)
#define I2C_MST_EN_BIT(x)     (x << 5)
#define FIFO_EN_BIT(x)        (x << 6)

/* MPU_PWR_MGMT1_REG (0x6B) **************************************************/
// 读/写寄存器。电源管理1寄存器
// DEVICE_RESET:  当设置为1时，将所有内部寄存器复位为默认值（自动清除）
// SLEEP:         当设置为1时，使MPU6050进入睡眠模式
// CYCLE:         当设置为1时，MPU6050在睡眠与唤醒之间循环
// TEMP_DIS:      当设置为1时，禁用温度传感器
// CLKSEL:        3位无符号值。指定器件的时钟源
//   CLKSEL   时钟源
//     0       内部8MHz振荡器
//     1       以X轴陀螺仪为参考的PLL
//     2       以Y轴陀螺仪为参考的PLL
//     3       以Z轴陀螺仪为参考的PLL
//     4       以外部32.768kHz为参考的PLL
//     5       以外部19.2MHz为参考的PLL
//     6       保留
//     7       停止时钟并将时序发生器保持在复位状态
#define DEVICE_RESET_BIT(x)  (x << 7)
#define SLEEP_BIT(x)         (x << 6)
#define CYCLE_BIT(x)         (x << 5)
#define TEMP_DIS_BIT(x)      (x << 3)
#define CLKSEL_BIT(x)        (x << 0)

/* MPU_PWR_MGMT2_REG (0x6C) **************************************************/
// 读/写寄存器。电源管理2寄存器
// LP_WAKE_CTRL:  2位无符号值。指定仅加速度计低功耗模式下的唤醒频率
//   LP_WAKE_CTRL   唤醒频率
//       0              1.25 Hz
//       1              5 Hz
//       2              20 Hz
//       3              40 Hz
// STBY_XA:  当设置为1时，将X轴加速度计置于待机模式
// STBY_YA:  当设置为1时，将Y轴加速度计置于待机模式
// STBY_ZA:  当设置为1时，将Z轴加速度计置于待机模式
// STBY_XG:  当设置为1时，将X轴陀螺仪置于待机模式
// STBY_YG:  当设置为1时，将Y轴陀螺仪置于待机模式
// STBY_ZG:  当设置为1时，将Z轴陀螺仪置于待机模式
#define LP_WAKE_CTRL_BIT(x) (x << 6)
#define STBY_XA_BIT(x)      (x << 5)
#define STBY_YA_BIT(x)      (x << 4)
#define STBY_ZA_BIT(x)      (x << 3)
#define STBY_XG_BIT(x)      (x << 2)
#define STBY_YG_BIT(x)      (x << 1)
#define STBY_ZG_BIT(x)      (x << 0)


#endif /* __BSP_MPU6050_REG_BIT_H__ */
