// // Created by 35540 on 2026/5/8. //
//******************************** Includes *********************************//
#include "mpu6050_driver.h"
#include "FreeRTOS.h"
#include "task.h"
//******************************** Includes *********************************//

//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "MPU6050Driver"
#else
#define LOG_TAG "MPU6050Driver"
#endif

#define IIC_MEMADD_SIZE_8BIT 0x00000001U
#define TIME_OUT_MS 1000
#define MPU6050_NOT_INIT 0
#define MPU6050_INIT 1

// 补充缺失的宏定义
#define CLOSE_ALL 0x00                  // 关闭所有中断
#define MPU6050_FIFO_DMA_READ_SIZE   1024   //MPU6050硬件FIFO大小

//******************************** Defines **********************************//

//---------------------------------------------------------------------------//
//******************************** Variables ********************************//
static double g_gyro_scale = 131.0;
static double g_accel_scale = 16384.0;
static uint8_t g_is_init_flag = MPU6050_NOT_INIT;
__attribute__((unused)) static uint32_t g_is_dma_readed = 0; // 添加属性消除未使用警告


//******************************** Variables ********************************//

//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#if defined(MPU6050Driver) && defined(MYDEBUG)
#define LOG_DEBUG(fmt, ...) log_d("[%s][%s:%d][DEBUG] " fmt "\r\n", LOG_TAG, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_e("[%s][%s:%d][ERROR] " fmt "\r\n", LOG_TAG, __func__, __LINE__, ##__VA_ARGS__)
#define ASSERT_NOT_NULL(ptr) do { if ((ptr) == NULL) { LOG_ERROR("Invalid parameter: %s is NULL", #ptr); while(1);} }while(0)
#define ASSERT_CONDITION(cond) do { if (!(cond)) { LOG_ERROR("Condition failed: %s", #cond); while(1);} }while(0)
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#define LOG_ERROR(fmt, ...) ((void)0)
#define ASSERT_NOT_NULL(ptr) ((void)0)
#define ASSERT_CONDITION(cond) ((void)0)
#endif


#define MPU6050_WRITE_REG(p_mpu_driver, reg, p_data, len)\
p_mpu_driver->p_driver_api->p_iic_driver->pf_iic_mem_write(\
p_mpu_driver->p_driver_api->p_iic_driver->hi2c,\
(MPU_ADDR << 1) | 0, reg, IIC_MEMADD_SIZE_8BIT, p_data, len, TIME_OUT_MS)

#define MPU6050_READ_REG(p_mpu_driver, reg, p_data, len)\
p_mpu_driver->p_driver_api->p_iic_driver->pf_iic_mem_read(\
p_mpu_driver->p_driver_api->p_iic_driver->hi2c,\
(MPU_ADDR << 1) | 1, reg, IIC_MEMADD_SIZE_8BIT, p_data, len, TIME_OUT_MS)
//******************************** Macros ***********************************//

//---------------------------------------------------------------------------//
//******************************** Functions ********************************//

/**
 * 反初始化MPU6050驱动,复位初始化标志
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @return  MPU6050_OK 成功; MPU6050_ERROR 未初始化
 */
static mpu6050_status_t mpu_driver_deinit(bsp_mpu6050_driver_t *p_mpu6050)
{
    if (MPU6050_NOT_INIT == g_is_init_flag) {
        LOG_ERROR("mpu6050 not inited");
        return MPU6050_ERROR;
    }
    g_is_init_flag = MPU6050_NOT_INIT; ///< 清除初始化标志
    return MPU6050_OK;
}

/**
 * 使MPU6050进入休眠状态
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @return  MPU6050_OK
 */
static mpu6050_status_t mpu_driver_sleep(bsp_mpu6050_driver_t *p_mpu6050)
{
    g_is_init_flag = MPU6050_NOT_INIT; ///< 休眠时清除初始化标志
    return MPU6050_OK;
}

/**
 * 唤醒MPU6050:向电源管理寄存器写0退出睡眠
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @return  MPU6050_OK 成功; 其他 写入寄存器失败
 */
static mpu6050_status_t mpu_driver_wakeup(bsp_mpu6050_driver_t *p_mpu6050)
{
    uint8_t data = SLEEP_BIT(0);
    mpu6050_status_t ret = MPU6050_WRITE_REG(p_mpu6050, MPU_PWR_MGMT1_REG, &data, 1); ///< 清除SLEEP位唤醒设备
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 write MPU_PWR_MGMT1_REG error"); return ret; }
    return ret;
}

/**
 * 设置陀螺仪满量程范围(FSR)并更新对应的比例因子
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[in] fsr        量程选择(0:±250, 1:±500, 2:±1000, 3:±2000 °/s)
 * @return  MPU6050_OK 成功; 其他 写入寄存器失败
 */
static mpu6050_status_t mpu_driver_set_gyro_fsr(bsp_mpu6050_driver_t *p_mpu6050, uint8_t fsr)
{
    LOG_DEBUG("=======set gyro fsr=======");
    mpu6050_status_t ret = MPU6050_OK;
    fsr = FS_SEL_BIT(fsr); ///< 将量程值写入FS_SEL位段
    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_GYRO_CFG_REG, &fsr, 1);
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set gyro fsr error"); return ret; }
    switch (fsr) { ///< 根据量程更新陀螺仪比例因子
        case 0: g_gyro_scale = 131.0;break;
        case 1: g_gyro_scale = 65.5;break;
        case 2: g_gyro_scale = 32.8;break;
        case 3: g_gyro_scale = 16.4;break;
        default:g_gyro_scale = 131.0;break;
    }
    return ret;
}

/**
 * 设置加速度计满量程范围(FSR)并更新对应的比例因子
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[in] fsr        量程选择(0:±2g, 1:±4g, 2:±8g, 3:±16g)
 * @return  MPU6050_OK 成功; 其他 写入寄存器失败
 */
static mpu6050_status_t mpu_driver_set_accel_fsr(bsp_mpu6050_driver_t *p_mpu6050, uint8_t fsr)
{
    LOG_DEBUG("=======set accel fsr=======");
    mpu6050_status_t ret = MPU6050_OK;
    fsr = AFS_SEL_BIT(fsr); ///< 将量程值写入AFS_SEL位段
    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_ACCEL_CFG_REG, &fsr, 1);
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set accel fsr error"); return ret; }
    switch (fsr) { ///< 根据量程更新加速度计比例因子
        case 0: g_accel_scale = 16384;break;
        case 1: g_accel_scale = 8192;break;
        case 2: g_accel_scale = 4096;break;
        case 3: g_accel_scale = 2048;break;
        default:g_accel_scale = 16384;break;
    }
    return ret;
}

/**
 * 设置数字低通滤波器(DLPF)带宽
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[in] data       滤波器配置值(DLPF_CFG位段)
 * @return  MPU6050_OK 成功; 其他 写入寄存器失败
 */
static mpu6050_status_t mpu_driver_set_lpf(bsp_mpu6050_driver_t *p_mpu6050, uint8_t data)
{
    LOG_DEBUG("=======set lpf=======");
    mpu6050_status_t ret = MPU6050_OK;
    data = DLPF_CFG_BIT(data); ///< 将DLPF配置写入对应位段
    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_CFG_REG, &data, 1);
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set dlpf error"); return ret; }
    return ret;
}

/**
 * 设置采样率分频器(SMPLRT_DIV)
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[in] data       分频系数
 * @return  MPU6050_OK 成功; 其他 写入寄存器失败
 */
static mpu6050_status_t mpu_driver_set_rate(bsp_mpu6050_driver_t *p_mpu6050, uint8_t data)
{
    LOG_DEBUG("=======set SMPLRT=======");
    mpu6050_status_t ret = MPU6050_OK;
    data = SMPLRT_DIV_BIT(data); ///< 写入采样率分频位段
    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_SAMPLE_RATE_REG, &data, 1);
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set SMPLRT error"); return ret; }
    return ret;
}

/**
 * @brief 设置中断使能
 * @param[in,out] p_mpu6050 MPU驱动结构体指针
 * @param[in] data 中断使能设置值
 *|data |    中断名称
 *|BIT0	| 控制数据准备中断
 *|BIT1	|    *预留*
 *|BIT2	|  	 *预留*
 *|BIT3	| I2C主机相关中断
 *|BIT4	| FIFO 溢出中断
 *|BIT5	|   控制静态
 *|BIT6	| 检测运动中断
 *|BIT7	| 自由落体中断
 * @return 执行状态
 */
mpu6050_status_t mpu_driver_set_interrupt_enable(bsp_mpu6050_driver_t *p_mpu6050, uint8_t data)
{
    mpu6050_status_t ret = MPU6050_WRITE_REG(p_mpu6050, MPU_INT_EN_REG, &data, 1); ///< 写中断使能寄存器
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set interrupt_enable error"); return ret; }
    return ret;
}

/**
 * 设置运动检测阈值
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[in] data       阈值(0~255 LSB)
 * @return  MPU6050_OK 成功; 其他 写入寄存器失败
 */
static mpu6050_status_t mpu_driver_set_motion_threshold(bsp_mpu6050_driver_t *p_mpu6050, uint8_t data)
{
    LOG_DEBUG("=======set motion_threshold=======");
    mpu6050_status_t ret = MPU6050_WRITE_REG(p_mpu6050, MPU_MOTION_DET_REG, &data, 1); ///< 写运动检测阈值寄存器
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set motion_threshold error"); return ret; }
    return ret;
}

/**
 * 设置中断引脚配置(INT_PIN_CFG):电平/边沿及清除方式
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[in] data       中断引脚配置位掩码
 * @return  MPU6050_OK 成功; 其他 写入寄存器失败
 */
static mpu6050_status_t mpu_driver_set_INT_level(bsp_mpu6050_driver_t *p_mpu6050, uint8_t data)
{
    LOG_DEBUG("=======set INT_level=======");
    mpu6050_status_t ret = MPU6050_OK;
    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_INTBP_CFG_REG, &data, 1); ///< 写中断旁路配置寄存器
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set INT_level error"); return ret; }
    return ret;
}

/**
 * @brief 设置用户控制寄存器
 * @param[in,out] p_mpu6050 MPU驱动结构体指针
 * @param[in] data 用户控制设置值
 * |---------------------------------------------------------------|
 * |  BIT7     |    BIT6       |   BIT5        |    BIT4    | BIT3 |
 * |-----------|---------------|---------------|------------|------|
 * |   /       |  FIFO_EN      |I2C_MST_EN     | I2C_IF_DIS |  /   |
 * |-----------|---------------|---------------|------------|------|
 * |-----------|---------------|---------------|------------|------|
 * |  BIT2     |    BIT1       |    BIT0       |            |      |
 * |-----------|---------------|---------------|------------|------|
 * |FIFO_RESET | I2C_MST_RESET |SIG_COND_RESET |            |      |
 * |---------------------------------------------------------------|
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_user_ctrl(bsp_mpu6050_driver_t *p_mpu6050, uint8_t data)
{
    LOG_DEBUG("=======set user_ctrl=======");
    mpu6050_status_t ret = MPU6050_OK;
    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_USER_CTRL_REG, &data, 1); ///< 写用户控制寄存器(FIFO使能/复位等)
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set user_ctrl error"); return ret; }
    return ret;
}

/**
 * @brief 设置电源管理1寄存器
 * @param[in,out] p_mpu6050 MPU驱动结构体指针
 * @param[in] data 电源管理1设置值
 *|------------------------------------ |
 *|BIT7：置位后所有传感器恢复默认值           |
 *|BIT6：置位后进入睡眠模式                 |
 *|BIT5：置位切没有进入睡眠模式则MPU进行循环模式|
 *|BIT4：保留                            |
 *|BIT3：置位禁止温度传感器                 |
 *|BIT[0~2]：配置时钟                     |
 *|-------------------------------------|
 *|BIT[0~2]的值      时钟        |
 *|----------------------------|
 *|    0          内部时钟       |
 *|    1        x轴陀螺仪锁相环   |
 *|    2        Y轴陀螺仪锁相环   |
 *|    3        z轴陀螺仪锁相环   |
 *|    4        外部32.768kHz   |
 *|    5        外部19.2MHz     |
 *|    6            保留        |
 *|    7        关闭所有时钟      |
 *|----------------------------|
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_pwr_mgmt1_reg(bsp_mpu6050_driver_t *p_mpu6050, uint8_t data)
{
    LOG_DEBUG("=======set pwr_mgmt1 reg=======");
    mpu6050_status_t ret = MPU6050_OK;
    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_PWR_MGMT1_REG, &data, 1); ///< 配置时钟源与电源模式
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set pwr_mgmt1 reg error"); return ret; }
    return ret;
}

/**
 * @brief 设置电源管理2寄存器
 * @param[in,out] p_mpu6050 MPU驱动结构体指针
 * @param[in] data 电源管理2设置值
 *          置位为休眠，清零为正常工作，
 * BIT7：x轴陀螺仪休眠   BIT4：x轴加速度计休眠
 * BIT6：y轴陀螺仪休眠   BIT3：y轴加速度计休眠
 * BIT5：z轴陀螺仪休眠   BIT2：z轴加速度计休眠
 *仅当传感器处于 “循环睡眠 - 唤醒 ” 模式有效
 * BIT[0,1]     唤醒周期
 *    0         1.25ms
 *    1          2.5ms
 *    2            5ms
 *    3           10ms
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_pwr_mgmt2_reg(bsp_mpu6050_driver_t *p_mpu6050, uint8_t data)
{
    LOG_DEBUG("=======set pwr_mgmt2 reg=======");
    mpu6050_status_t ret = MPU6050_OK;
    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_PWR_MGMT2_REG, &data, 1); ///< 配置各轴加速度计/陀螺仪待机
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set pwr_mgmt2 reg error"); return ret; }
    return ret;
}

/**
 * @brief 设置FIFO使能寄存器
 * @param[in,out] p_mpu6050 MPU驱动结构体指针
 * @param[in] data FIFO使能设置值
 * Bit7	Bit6	Bit5	Bit4	Bit3	Bit2	Bit1	Bit0
 * 温度	陀螺仪X 陀螺仪Y  陀螺仪Z  加速度计  从设备 2  从设备1 从设备 0
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_fifo_en_reg(bsp_mpu6050_driver_t *p_mpu6050, uint8_t data)
{
    LOG_DEBUG("=======set pwr fifo_en reg=======");
    mpu6050_status_t ret = MPU6050_OK;
    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_FIFO_EN_REG, &data, 1); ///< 使能陀螺仪/加速度计FIFO写入
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 set fifo_en reg error"); return ret; }
    return ret;
}

/**
 * 读取温度传感器原始值并转换为摄氏度
 * @param[in]  p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[out] p_data     输出数据结构体(写入temperature字段)
 * @return  MPU6050_OK 成功; 其他 I2C读取失败
 */
static mpu6050_status_t mpu_driver_get_temperature(bsp_mpu6050_driver_t *p_mpu6050, mpu6050_data_t *p_data)
{
    LOG_DEBUG("=======get temperature=======");
    uint8_t data[2] = {0};
    int16_t temp = 0;
    mpu6050_status_t ret = MPU6050_READ_REG(p_mpu6050, MPU_TEMP_OUTH_REG, data, 2); ///< 读温度高/低字节
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 get temperature error"); return ret; }
    temp = ((int16_t)(data[0] << 8) | data[1]); ///< 拼接高8位与低8位
    p_data->temperature = ((float)temp / 340.0f + 36.53f); ///< 转换为摄氏度
    return ret;
}

/**
 * 读取加速度计原始数据并转换为g值
 * @param[in]  p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[out] p_data     输出数据结构体(写入ax/ay/az及raw字段)
 * @return  MPU6050_OK 成功; 其他 I2C读取失败
 */
static mpu6050_status_t mpu_driver_get_accel(bsp_mpu6050_driver_t *p_mpu6050, mpu6050_data_t *p_data)
{
    LOG_DEBUG("=======get accel=======");
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data[6] = {0};
    ret = MPU6050_READ_REG(p_mpu6050, MPU_ACCEL_XOUTH_REG, data, 6); ///< 连续读6字节: X/Y/Z各2字节
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 get accel error"); return ret; }
    p_data->accel_x_raw = (int16_t)(data[0]<<8 | data[1]); ///< X轴原始值(高字节|低字节)
    p_data->accel_y_raw = (int16_t)(data[2]<<8 | data[3]); ///< Y轴原始值
    p_data->accel_z_raw = (int16_t)(data[4]<<8 | data[5]); ///< Z轴原始值
    p_data->ax = (double)(p_data->accel_x_raw/g_accel_scale); ///< 转换为g单位
    p_data->ay = (double)(p_data->accel_y_raw/g_accel_scale);
    p_data->az = (double)(p_data->accel_z_raw/g_accel_scale);
    return ret;
}

/**
 * 读取陀螺仪原始数据并转换为°/s值
 * @param[in]  p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[out] p_data     输出数据结构体(写入gx/gy/gz及raw字段)
 * @return  MPU6050_OK 成功; 其他 I2C读取失败
 */
static mpu6050_status_t mpu_driver_get_gyro(bsp_mpu6050_driver_t *p_mpu6050, mpu6050_data_t *p_data)
{
    LOG_DEBUG("=======get gyro=======");
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data[6] = {0};
    ret = MPU6050_READ_REG(p_mpu6050, MPU_GYRO_XOUTH_REG, data, 6); ///< 连续读6字节: X/Y/Z各2字节
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 get gyro error"); return ret; }
    p_data->gyro_x_raw = (int16_t)(data[0]<<8 | data[1]); ///< X轴原始值(高字节|低字节)
    p_data->gyro_y_raw = (int16_t)(data[2]<<8 | data[3]); ///< Y轴原始值
    p_data->gyro_z_raw = (int16_t)(data[4]<<8 | data[5]); ///< Z轴原始值
    p_data->gx = (double)(p_data->gyro_x_raw/g_gyro_scale); ///< 转换为°/s单位
    p_data->gy = (double)(p_data->gyro_y_raw/g_gyro_scale);
    p_data->gz = (double)(p_data->gyro_z_raw/g_gyro_scale);
    return ret;
}

/**
 * 一次性读取全部传感器数据(加速度计+温度+陀螺仪,共14字节)
 * @param[in]  p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[out] p_data     输出数据结构体(写入所有物理量及raw字段)
 * @return  MPU6050_OK 成功; 其他 I2C读取失败
 */
static mpu6050_status_t mpu_driver_get_all_data(bsp_mpu6050_driver_t *p_mpu6050, mpu6050_data_t *p_data)
{
    LOG_DEBUG("=======get all data=======");
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data[14] = {0};
    int16_t temp = 0;
    ret = MPU6050_READ_REG(p_mpu6050, MPU_ACCEL_XOUTH_REG, data, 14); ///< 从ACCEL_XOUTH起连续读14字节
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 get all data error"); return ret; }
    p_data->accel_x_raw = (int16_t)(data[0]<<8 | data[1]); ///< 字节0-1: 加速度X
    p_data->accel_y_raw = (int16_t)(data[2]<<8 | data[3]); ///< 字节2-3: 加速度Y
    p_data->accel_z_raw = (int16_t)(data[4]<<8 | data[5]); ///< 字节4-5: 加速度Z
    p_data->ax = (double)(p_data->accel_x_raw/g_accel_scale); ///< 加速度X转g值
    p_data->ay = (double)(p_data->accel_y_raw/g_accel_scale);
    p_data->az = (double)(p_data->accel_z_raw/g_accel_scale);
    temp = ((int16_t)(data[6] << 8) | data[7]); ///< 字节6-7: 温度原始值
    p_data->temperature = ((float)temp / 340.0f + 36.53f); ///< 温度转摄氏度
    p_data->gyro_x_raw = (int16_t)(data[8]<<8 | data[9]);  ///< 字节8-9: 陀螺仪X
    p_data->gyro_y_raw = (int16_t)(data[10]<<8 | data[11]); ///< 字节10-11: 陀螺仪Y
    p_data->gyro_z_raw = (int16_t)(data[12]<<8 | data[13]); ///< 字节12-13: 陀螺仪Z
    p_data->gx = (double)(p_data->gyro_x_raw/g_gyro_scale); ///< 陀螺仪X转°/s
    p_data->gy = (double)(p_data->gyro_y_raw/g_gyro_scale);
    p_data->gz = (double)(p_data->gyro_z_raw/g_gyro_scale);
    return ret;
}

/**
 * 读取中断状态寄存器(INT_STATUS)以确定中断源
 * @param[in]  p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[out] p_data     输出中断状态字节
 * @return  MPU6050_OK 成功; 其他 I2C读取失败
 */
static mpu6050_status_t mpu_driver_get_interrupt_status_reg(bsp_mpu6050_driver_t *p_mpu6050, uint8_t *p_data)
{
    LOG_DEBUG("=======get interrupt_status reg=======");
    mpu6050_status_t ret = MPU6050_OK;
    ret = MPU6050_READ_REG(p_mpu6050, MPU_INT_STA_REG, p_data, 1); ///< 读中断状态寄存器1字节
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 get interrupt_status reg error"); return ret; }
    return ret;
}

/**
 * 读取FIFO中的数据包并解析为物理量
 * @param[in]  p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[out] p_data     输出数据数组(长度需能容纳所有FIFO包)
 * @return  MPU6050_OK 成功; 其他 I2C读取或FIFO为空
 */
static mpu6050_status_t mpu_driver_read_fifo_packet(bsp_mpu6050_driver_t *p_mpu6050, mpu6050_data_t *p_data)
{
    LOG_DEBUG("=======read fifo packet=======");
    mpu6050_status_t ret = MPU6050_OK;
    uint16_t fifo_count = 0;
    uint16_t fifo_pack_count = 0;
    uint8_t fifo_buffer[12] = {0};
    ret = MPU6050_READ_REG(p_mpu6050, MPU_FIFO_CNTH_REG, fifo_buffer, 2); ///< 读FIFO计数器高/低字节
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 read_fifo_CNTH error"); return ret; }
    fifo_count = (fifo_buffer[0]<<8) | fifo_buffer[1]; ///< 拼接16位FIFO字节计数
    LOG_DEBUG("mpu6050 read fifo cnt is [%u]",fifo_count);
    if (fifo_count >= 12) { ///< 至少有1个完整包(12字节)
        fifo_pack_count = fifo_count / 12; ///< 计算完整包数量
        for (uint16_t i = 0; i < fifo_pack_count; i++) {
            ret = MPU6050_READ_REG(p_mpu6050, MPU_FIFO_RW_REG, fifo_buffer, 12); ///< 从FIFO读取一个完整包
            if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 fifo read error"); return ret; }
            p_data[i].accel_x_raw=(int16_t)(fifo_buffer[0]<<8)|fifo_buffer[1]; ///< 解析: 加速度X
            p_data[i].accel_y_raw=(int16_t)(fifo_buffer[2]<<8)|fifo_buffer[3]; ///< 加速度Y
            p_data[i].accel_z_raw=(int16_t)(fifo_buffer[4]<<8)|fifo_buffer[5]; ///< 加速度Z
            p_data[i].gyro_x_raw=(int16_t)(fifo_buffer[6]<<8)|fifo_buffer[7];   ///< 陀螺仪X
            p_data[i].gyro_y_raw=(int16_t)(fifo_buffer[8]<<8)|fifo_buffer[9];   ///< 陀螺仪Y
            p_data[i].gyro_z_raw=(int16_t)(fifo_buffer[10]<<8)|fifo_buffer[11]; ///< 陀螺仪Z
            p_data[i].ax=(double)p_data[i].accel_x_raw/g_accel_scale; ///< 转换为g值
            p_data[i].ay=(double)p_data[i].accel_y_raw/g_accel_scale;
            p_data[i].az=(double)p_data[i].accel_z_raw/g_accel_scale;
            p_data[i].gx=(double)p_data[i].gyro_x_raw/g_gyro_scale; ///< 转换为°/s
            p_data[i].gy=(double)p_data[i].gyro_y_raw/g_gyro_scale;
            p_data[i].gz=(double)p_data[i].gyro_z_raw/g_gyro_scale;
        }
    }
    return ret;
}

/**
 * FIFO中断发生时的读取处理(当前为空实现)
 * @param[in]  p_mpu6050  指向MPU6050驱动结构体的指针
 * @param[out] p_data     输出数据指针
 * @return  MPU6050_OK
 */
static mpu6050_status_t mpu_driver_read_fifo_isr_occur(bsp_mpu6050_driver_t *p_mpu6050, mpu6050_data_t *p_data)
{
    mpu6050_status_t ret = MPU6050_OK;
    return ret;
}

/**
 * 初始化运动检测功能:设置阈值、中断引脚和使能
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @return  MPU6050_OK 成功; 其他 寄存器写入失败
 */
// 添加 __attribute__((unused)) 消除编译器警告
__attribute__((unused)) static mpu6050_status_t mpu_motion_init(bsp_mpu6050_driver_t* p_mpu6050)
{
    mpu6050_status_t ret = MPU6050_OK;
    ret = mpu_driver_set_motion_threshold(p_mpu6050, 0x10); ///< 设置运动检测阈值为0x10
    if (MPU6050_OK != ret) { LOG_ERROR("motion threshold set error"); return ret; }
    ret = mpu_driver_set_INT_level(p_mpu6050, ( INT_RD_CLEAR_BIT(1) | INT_LEVEL_BIT(1) )); ///< 配置中断为读取清除 + 高电平有效
    if (MPU6050_OK != ret) { LOG_ERROR("INT level set error"); return ret; }
    ret = mpu_driver_set_interrupt_enable(p_mpu6050, MOT_EN_BIT(1)); ///< 使能运动检测中断
    if (MPU6050_OK!= ret) { LOG_ERROR("INT enable error"); return ret; }
    return ret;
}

/**
 * 初始化FIFO:复位FIFO,使能传感器数据写入FIFO,配置FIFO溢出中断
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @return  MPU6050_OK 成功; 其他 寄存器写入失败
 */
static mpu6050_status_t mpu_fifo_init(bsp_mpu6050_driver_t* p_mpu6050)
{
    mpu6050_status_t ret = MPU6050_OK;
    ret = mpu_driver_set_user_ctrl(p_mpu6050, FIFO_RESET_BIT(1)); ///< 复位FIFO
    if (MPU6050_OK!=ret) { LOG_ERROR("writer user ctrl error"); return ret; }
#ifdef OS_SUPPORTING
    p_mpu6050->p_driver_api->p_yield->pf_rtos_yield(10); ///< 等待FIFO复位完成
#else
    p_mpu6050->p_delay_interface->pf_delay_ms(10);
#endif
    ret = mpu_driver_set_fifo_en_reg(p_mpu6050, XG_FIFO_EN_BIT(1)| YG_FIFO_EN_BIT(1) | ZG_FIFO_EN_BIT(1) | ACCEL_FIFO_EN_BIT(1)); ///< 使能陀螺仪三轴+加速度计写入FIFO
    if (MPU6050_OK!=ret) { LOG_ERROR("mpu_write error"); return ret; }
    ret = mpu_driver_set_INT_level(p_mpu6050, INT_RD_CLEAR_BIT(1) | INT_LEVEL_BIT(1)); ///< 中断读取清除 + 高电平有效
    if (MPU6050_OK != ret) { LOG_ERROR("set INT level error"); return ret; }
    ret = mpu_driver_set_interrupt_enable(p_mpu6050,FIFO_OVERFLOW_EN_BIT(1)); ///< 使能FIFO溢出中断
    if (MPU6050_OK != ret) { LOG_ERROR("set interrupt enable error"); return ret; }
    return ret;
}

/**
 * MPU6050底层初始化序列:复位 → 唤醒 → 配置量程/采样率/DLPF → 验证ID → 设置时钟源
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @return  MPU6050_OK 成功; 其他 初始化步骤失败
 */
static mpu6050_status_t mpu_init(bsp_mpu6050_driver_t* p_mpu6050)
{
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t id = 0;
#ifndef OS_SUPPORTING
    p_mpu6050->p_delay_interface->pf_delay_init();
#endif
    p_mpu6050->p_driver_api->p_iic_driver->pf_iic_init(NULL); ///< 初始化I2C总线
    ret = mpu_driver_set_pwr_mgmt1_reg(p_mpu6050, DEVICE_RESET_BIT(1)); ///< 触发设备复位
    if (MPU6050_OK != ret) { LOG_ERROR("set power reset error"); return ret; }
#ifdef OS_SUPPORTING
    p_mpu6050->p_driver_api->p_yield->pf_rtos_yield(100); ///< 等待复位完成(100ms)
#else
    p_mpu6050->p_delay_interface->pf_delay_ms(100);
#endif
    ret = mpu_driver_wakeup(p_mpu6050); ///< 唤醒设备(清除SLEEP位)
    if (MPU6050_OK != ret) { LOG_ERROR("set power reset error"); return ret; }
    ret = mpu_driver_set_gyro_fsr(p_mpu6050, 3); ///< 设置陀螺仪量程: ±2000°/s
    if (MPU6050_OK != ret) { LOG_ERROR("set gyro fsr error"); return ret; }
    ret = mpu_driver_set_accel_fsr(p_mpu6050, 3); ///< 设置加速度计量程: ±16g
    if (MPU6050_OK != ret) { LOG_ERROR("set accel fsr error"); return ret; }
    ret = mpu_driver_set_rate(p_mpu6050,0x19); ///< 设置采样率分频(1kHz/(1+25)=~38Hz)
    if (MPU6050_OK != ret) { LOG_ERROR("set rate error"); return ret; }
    ret = mpu_driver_set_lpf(p_mpu6050,0x04); ///< 设置DLPF带宽: 20Hz(陀螺仪)/21.2Hz(加速度计)
    if (MPU6050_OK != ret) { LOG_ERROR("set lpf error"); return ret; }
    ret = mpu_driver_set_interrupt_enable(p_mpu6050,DATA_RDY_EN_BIT(1)); ///< 使能数据就绪中断
    if (MPU6050_OK != ret) { LOG_ERROR("set interrupt is ng"); return ret; }
    ret = MPU6050_READ_REG(p_mpu6050,MPU_DEVICE_ID_REG,&id,1); ///< 读取WHO_AM_I验证设备身份
    if (MPU6050_OK!=ret || id !=MPU_ID) { LOG_ERROR("device ID error"); return ret; }
    ret = mpu_driver_set_pwr_mgmt1_reg(p_mpu6050, CLKSEL_BIT(1)); ///< 选择PLL with X轴陀螺仪作为时钟源
    if (MPU6050_OK != ret) { LOG_ERROR("set pwr1 error"); return ret; }
    ret = mpu_driver_set_pwr_mgmt2_reg(p_mpu6050, LP_WAKE_CTRL_BIT(0) | STBY_XA_BIT(0) | STBY_YA_BIT(0) | STBY_ZA_BIT(0) | STBY_XG_BIT(0) | STBY_YG_BIT(0) | STBY_ZG_BIT(0) ); ///< 所有轴退出待机,正常工作
    if (MPU6050_OK != ret) { LOG_ERROR("set pwr2 error"); return ret; }
    return ret;
}

/**
 * BSP层MPU6050驱动初始化:检查状态 → 底层初始化 → FIFO初始化
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针
 * @return  MPU6050_OK 成功; MPU6050_ERRORPARAMETER 已初始化; 其他 初始化失败
 */
static mpu6050_status_t bsp_mpu6050_driver_init(bsp_mpu6050_driver_t* p_mpu6050)
{
    mpu6050_status_t ret = MPU6050_OK;
    if (MPU6050_INIT == g_is_init_flag) { LOG_ERROR("mpu6050 is inited,not need init"); return MPU6050_ERRORPARAMETER; } ///< 防止重复初始化
    LOG_DEBUG("mpu6050 driver init is start");
    ret = mpu_init(p_mpu6050); ///< 执行底层硬件初始化
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 init error"); return ret; }
    ret = mpu_fifo_init(p_mpu6050); ///< 初始化FIFO模式
    if (MPU6050_OK != ret) { LOG_ERROR("mpu6050 fifo init error"); return ret; }
    return ret;
}

/**
 * 外部中断回调:响应MPU6050 INT引脚中断
 * - 非OS模式: 直接读取全部传感器数据
 * - OS模式: 关闭中断 → 读中断状态 → 启动DMA读取 → 由DMA完成回调通知任务
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针(void*)
 * @param[in] p_data      数据缓冲区的用户指针
 */
void int_interrupt_callback(void *p_mpu6050, void *p_data)
{
    LOG_DEBUG("=====int_interrupt_callback start=====");
    mpu6050_status_t ret = MPU6050_OK;
    bsp_mpu6050_driver_t *p_mpu_driver = NULL;
    ASSERT_NOT_NULL(p_mpu6050);
    p_mpu_driver = (bsp_mpu6050_driver_t*)p_mpu6050;

#ifndef OS_SUPPORTING
    ret = mpu_driver_get_all_data(p_mpu_driver, (mpu6050_data_t*)p_data); ///< 非OS: 同步读取全部数据
    if (MPU6050_OK != ret) { LOG_ERROR("int_interrupt_callback mpu6050_get_all_data error"); LOG_ERROR("ret = %d", ret); }
#else
    // 1. 获取环形缓冲区的写入地址
    uint8_t *wbuff = NULL;

    // 获取当前写包的物理首地址
    if (p_mpu_driver->p_driver_api->p_buffer != NULL && p_mpu_driver->p_driver_api->p_buffer->pf_get_wbuffer_addr != NULL) {
        wbuff = p_mpu_driver->p_driver_api->p_buffer->pf_get_wbuffer_addr(p_mpu_driver->p_driver_api->p_buffer->p_ctx);
    }

    // 极其重要：如果缓冲区满了，返回的地址可能是NULL，必须拦截，否则DMA写入空地址会HardFault
    if (NULL == wbuff) {
        LOG_ERROR("Buffer is full or get_wbuffer_addr failed! Abort DMA.");
        return;
    }
    LOG_DEBUG("int_interrupt_callback wbuff = %p", wbuff);

    uint32_t timestamp_start = p_mpu_driver->p_driver_api->p_timebase->pf_get_tick_count(); ///< 记录DMA传输起始时间戳
    LOG_DEBUG("get timestamp start : %d", timestamp_start);

    ret = p_mpu_driver->p_driver_api->p_iic_driver->pf_iic_mem_read_dma( ///< 启动DMA异步读取14字节传感器数据
            p_mpu_driver->p_driver_api->p_iic_driver->hi2c,
            (MPU_ADDR << 1) | 1,
            MPU_FIFO_RW_REG,
            IIC_MEMADD_SIZE_8BIT,
            wbuff,
            MPU6050_FIFO_DMA_READ_SIZE);
    if (MPU6050_OK != ret) { LOG_ERROR("int_interrupt_callback read accel data error"); LOG_ERROR("ret = %d", ret); }
#endif
    LOG_DEBUG("=====int_interrupt_callback end=====");
}

/**
 * DMA传输完成回调:推进环形缓冲区写指针 → 重新使能中断 → 通过队列通知RTOS任务处理数据
 * @param[in] p_mpu6050  指向MPU6050驱动结构体的指针(void*)
 * @param[in] p_data      未使用
 */
void dma_interrupt_callback(void *p_mpu6050, void *p_data)
{
    LOG_DEBUG("=====dma_interrupt_callback start=====");
    ASSERT_NOT_NULL(p_mpu6050);
    mpu6050_status_t ret = MPU6050_OK;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bsp_mpu6050_driver_t *p_mpu_driver = NULL;
    p_mpu_driver = (bsp_mpu6050_driver_t*)p_mpu6050;
    uint32_t timestamp_end = p_mpu_driver->p_driver_api->p_timebase->pf_get_tick_count(); ///< 记录DMA传输结束时间戳
    LOG_DEBUG("get timestamp end : %d", timestamp_end);

#ifdef OS_SUPPORTING
    if (p_mpu_driver->p_driver_api->p_buffer != NULL && p_mpu_driver->p_driver_api->p_buffer->pf_data_writed != NULL) {
        p_mpu_driver->p_driver_api->p_buffer->pf_data_writed(p_mpu_driver->p_driver_api->p_buffer->p_ctx); ///< 通知上层：DMA写完了，指针往前走吧！
    } else {
        LOG_ERROR("pf_data_writed callback is NULL!");
    }

    if (NULL == p_mpu_driver->p_os_handles->queue_handle) { LOG_DEBUG("queue_handle is NULL"); }
    uint8_t tx_data = 1;
    //将xHigherPriorityTaskWoken和portYIELD_FROM_ISR(xHigherPriorityTaskWoken)隐藏在os_queue_put_isr接口内部
    ret = p_mpu_driver->p_driver_api->p_os->os_queue_put_isr( ///< 在ISR中向队列发送通知
            p_mpu_driver->p_os_handles->queue_handle, &tx_data);
    if (MPU6050_OK != ret) { LOG_ERROR("dma_interrupt_callback put queue error"); LOG_ERROR("ret = %d", ret); }
    LOG_DEBUG("-----dma_interrupt_callback end-----");
#endif
}

/**
 * @brief MPU6050驱动实例化:校验依赖包 → 绑定驱动方法 → 初始化硬件 → 注册中断回调
 *
 * @details 该函数是驱动层的核心入口，采用依赖注入模式。
 *          所有的底层硬件接口和OS句柄均已在外部打包为 p_driver_api 和 p_os_handles，
 *          本函数仅需校验其完整性并挂载到驱动实例上，彻底解耦了底层硬件细节。
 *
 * @param[in] p_mpu6050_driver     驱动结构体指针（对象自身）
 * @param[in] p_driver_api         驱动层依赖接口包裹（包含I2C、延时、时基、Buffer、OS等抽象接口）
 * @param[in] p_os_handles         运行时OS句柄包裹（包含队列、信号量、任务通知，仅OS模式需要）
 * @param[in] callback_register    注册INT引脚中断回调的函数
 * @param[in] callback_register_dma 注册DMA传输完成回调的函数
 *
 * @return mpu6050_status_t
 *         MPU6050_OK 成功;
 *         MPU6050_ERRORPARAMETER 参数错误;
 *         MPU6050_ERROR 初始化失败
 */
mpu6050_status_t bsp_mpu6050_driver_inst(
    bsp_mpu6050_driver_t *p_mpu6050_driver,
    mpu6050_driver_input_api_t *p_driver_api,
#ifdef OS_SUPPORTING
    mpu6050_driver_os_handles_t *p_os_handles,
#endif
    void (*callback_register) (void (*callback)(void *, void *)),
    void (*callback_register_dma)(void (*callback)(void *, void *))
)
{
    mpu6050_status_t ret = MPU6050_OK;
    LOG_DEBUG("===mpu6050_driver inst start===");

    /*---------------- 1. 顶级指针校验 ----------------*/
    ASSERT_NOT_NULL(p_mpu6050_driver);
    ASSERT_NOT_NULL(p_driver_api);
    ASSERT_NOT_NULL(callback_register);
    ASSERT_NOT_NULL(callback_register_dma);
#ifdef OS_SUPPORTING
    ASSERT_NOT_NULL(p_os_handles);
#endif

    /*---------------- 2. 依赖接口包内部校验 (验收包裹) ----------------*/
    // 校验I2C接口（最核心）
    ASSERT_NOT_NULL(p_driver_api->p_iic_driver);
    ASSERT_NOT_NULL(p_driver_api->p_iic_driver->pf_iic_init);
    ASSERT_NOT_NULL(p_driver_api->p_iic_driver->pf_iic_mem_write);
    ASSERT_NOT_NULL(p_driver_api->p_iic_driver->pf_iic_mem_read);
    ASSERT_NOT_NULL(p_driver_api->p_iic_driver->pf_iic_mem_read_dma);

    // 校验延时与时基接口
    ASSERT_NOT_NULL(p_driver_api->p_delay);
    ASSERT_NOT_NULL(p_driver_api->p_delay->pf_delay_ms);
    ASSERT_NOT_NULL(p_driver_api->p_timebase);
    ASSERT_NOT_NULL(p_driver_api->p_timebase->pf_get_tick_count);

    // 校验缓冲区接口
    ASSERT_NOT_NULL(p_driver_api->p_buffer);
    ASSERT_NOT_NULL(p_driver_api->p_buffer->p_ctx);
    ASSERT_NOT_NULL(p_driver_api->p_buffer->pf_get_wbuffer_addr);
    ASSERT_NOT_NULL(p_driver_api->p_buffer->pf_data_writed);

#ifdef OS_SUPPORTING
    // 校验OS模式专属接口
    ASSERT_NOT_NULL(p_driver_api->p_yield);
    ASSERT_NOT_NULL(p_driver_api->p_yield->pf_rtos_yield);
    ASSERT_NOT_NULL(p_driver_api->p_os);
    ASSERT_NOT_NULL(p_driver_api->p_os->os_queue_put_isr);

    // 校验OS句柄
    ASSERT_NOT_NULL(p_os_handles->queue_handle);
#endif

    /*---------------- 3. 挂载依赖包 (签收上架) ----------------*/
    // ✨ 注意：这里不再像旧代码那样逐个赋值接口，而是直接挂载整个包的指针！
    p_mpu6050_driver->p_driver_api = p_driver_api;
#ifdef OS_SUPPORTING
    p_mpu6050_driver->p_os_handles = p_os_handles;
#endif

    /*---------------- 4. 绑定驱动功能函数指针 (对象方法) ----------------*/
    p_mpu6050_driver->pf_deinit = mpu_driver_deinit;
    p_mpu6050_driver->pf_sleep = mpu_driver_sleep;
    p_mpu6050_driver->pf_wakeup = mpu_driver_wakeup;
    p_mpu6050_driver->pf_set_gyro_fsr = mpu_driver_set_gyro_fsr;
    p_mpu6050_driver->pf_set_accel_fsr = mpu_driver_set_accel_fsr;
    p_mpu6050_driver->pf_set_lpf = mpu_driver_set_lpf;
    p_mpu6050_driver->pf_set_rate = mpu_driver_set_rate;
    p_mpu6050_driver->pf_set_interrupt_enable = mpu_driver_set_interrupt_enable;
    p_mpu6050_driver->pf_set_motion_threshold = mpu_driver_set_motion_threshold;
    p_mpu6050_driver->pf_set_INT_level = mpu_driver_set_INT_level;
    p_mpu6050_driver->pf_set_user_ctrl = mpu_driver_set_user_ctrl;
    p_mpu6050_driver->pf_set_pwr_mgmt1_reg = mpu_driver_set_pwr_mgmt1_reg;
    p_mpu6050_driver->pf_set_pwr_mgmt2_reg = mpu_driver_set_pwr_mgmt2_reg;
    p_mpu6050_driver->pf_set_fifo_en_reg = mpu_driver_set_fifo_en_reg;
    p_mpu6050_driver->pf_get_temperature = mpu_driver_get_temperature;
    p_mpu6050_driver->pf_get_accel = mpu_driver_get_accel;
    p_mpu6050_driver->pf_get_gyro = mpu_driver_get_gyro;
    p_mpu6050_driver->pf_get_all_data = mpu_driver_get_all_data;
    p_mpu6050_driver->pf_get_interrupt_status_reg = mpu_driver_get_interrupt_status_reg;
    p_mpu6050_driver->pf_read_fifo_packet = mpu_driver_read_fifo_packet;
    p_mpu6050_driver->pf_read_fifo_isr_occur = mpu_driver_read_fifo_isr_occur;

    /*---------------- 5. 执行硬件初始化与回调注册 ----------------*/
    ret = bsp_mpu6050_driver_init(p_mpu6050_driver);
    if (MPU6050_OK != ret) {
        LOG_ERROR("mpu init error");
        return MPU6050_ERROR;
    }

    callback_register(int_interrupt_callback);
    callback_register_dma(dma_interrupt_callback);

    LOG_DEBUG("===mpu6050_driver inst end===");
    return ret;
}

