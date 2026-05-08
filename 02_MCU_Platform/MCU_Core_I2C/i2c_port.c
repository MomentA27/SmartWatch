//
// Created by 35540 on 2026/4/21.
//
//******************************** Includes *********************************//
#include "i2c_port.h"

#include "cmsis_os.h"
#include "gpio_port.h"


//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//

//******************************** Defines *********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
static st_i2c_port_t st_i2c_port[CORE_I2C_BUS_MAX] =
{
  [CORE_I2C_BUS_1] = {
    .en_i2c_state = EN_SOFTWARE_I2C,
    .st_iic_bus_inst = {
      .I2C_SDA_PORT = Sensor_I2C_SDA_Port,
      .I2C_SDA_PIN  = Sensor_I2C_SDA_Pin,
      .I2C_SCL_PORT = Sensor_I2C_SCL_Port,
      .I2C_SCL_PIN  = Sensor_I2C_SCL_Pin
  },
    .st_I2C_HandleTypeDef = &hi2c1,
  }
};

static void core_i2c_switch(en_core_i2c_bus_t bus, en_i2c_state_t state);


/**
 * @brief 向指定I2C总线注册互斥锁，用于线程安全访问
 * @param[in] bus I2C总线编号（BUS_1或BUS_2）
 * @param[Out] mutex 互斥锁句柄
 * @return None
 * */
void core_i2c_register_mutex(en_core_i2c_bus_t bus, SemaphoreHandle_t mutex)
{
    if (bus < CORE_I2C_BUS_MAX) {
        st_i2c_port[bus].st_osMutexId = mutex;
    }
}

/**
 * @brief 锁定指定I2C总线，获取互斥锁
 * @param[in] bus I2C总线编号
 * @param[Out] None
 * @return None
 * */
static inline void core_i2c_lock(en_core_i2c_bus_t bus)
{
    if (st_i2c_port[bus].st_osMutexId) {
        os_mutex_take(st_i2c_port[bus].st_osMutexId, osWaitForever);
    }
}

/**
 * @brief 解锁指定I2C总线，释放互斥锁
 * @param[in] bus I2C总线编号
 * @param[Out] None
 * @return None
 * */
static inline void core_i2c_unlock(en_core_i2c_bus_t bus)
{
    if (st_i2c_port[bus].st_osMutexId) {
        os_mutex_give(st_i2c_port[bus].st_osMutexId);
    }
}

/**
 * @brief 向指定I2C总线的从设备发送数据（主发送模式）
 * @param[in] bus I2C总线编号
 * @param[in] dev_addr 从设备地址
 * @param[in] data 发送数据缓冲区指针
 * @param[in] size 数据长度
 * @param[in] timeout 超时时间（ms）
 * @param[Out] None
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_write(en_core_i2c_bus_t bus, uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (bus >= CORE_I2C_BUS_MAX || (st_i2c_port[bus].st_I2C_HandleTypeDef == NULL)) return CORE_I2C_ERROR;
    //core_i2c_lock(bus);
    core_i2c_switch(bus,EN_HARDWARE_I2C);
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(st_i2c_port[bus].st_I2C_HandleTypeDef, dev_addr, data, size, timeout);
    //core_i2c_unlock(bus);
    return (ret == HAL_OK) ? CORE_I2C_OK : CORE_I2C_ERROR;
}

/**
 * @brief 从指定I2C总线的从设备接收数据（主接收模式）
 * @param[in] bus I2C总线编号
 * @param[in] dev_addr 从设备地址
 * @param[Out] data 接收数据缓冲区指针
 * @param[in] size 数据长度
 * @param[in] timeout 超时时间（ms）
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_read(en_core_i2c_bus_t bus, uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (bus >= CORE_I2C_BUS_MAX || (st_i2c_port[bus].st_I2C_HandleTypeDef == NULL)) return CORE_I2C_ERROR;
    //core_i2c_lock(bus);
    core_i2c_switch(bus,EN_HARDWARE_I2C);
    HAL_StatusTypeDef ret = HAL_I2C_Master_Receive(st_i2c_port[bus].st_I2C_HandleTypeDef, dev_addr, data, size, timeout);
    //core_i2c_unlock(bus);
    return (ret == HAL_OK) ? CORE_I2C_OK : CORE_I2C_ERROR;
}

/**
 * @brief 向指定I2C从设备的指定内存地址写入数据
 * @param[in] bus I2C总线编号
 * @param[in] dev_addr 从设备地址
 * @param[in] mem_addr 内存地址
 * @param[in] mem_size 内存地址长度（8bit/16bit）
 * @param[in] data 发送数据缓冲区指针
 * @param[in] size 数据长度
 * @param[in] timeout 超时时间（ms）
 * @param[Out] None
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_mem_write(en_core_i2c_bus_t bus, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size,
                                     uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (bus >= CORE_I2C_BUS_MAX || (st_i2c_port[bus].st_I2C_HandleTypeDef == NULL)) return CORE_I2C_ERROR;
    //core_i2c_lock(bus);
    core_i2c_switch(bus,EN_HARDWARE_I2C);
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(st_i2c_port[bus].st_I2C_HandleTypeDef, dev_addr, mem_addr, mem_size, data, size, timeout);
    //core_i2c_unlock(bus);
    return (ret == HAL_OK) ? CORE_I2C_OK : CORE_I2C_ERROR;
}
/**
 * @brief 从指定I2C从设备的指定内存地址读取数据（阻塞模式）
 * @param[in] bus I2C总线编号
 * @param[in] dev_addr 从设备地址
 * @param[in] mem_addr 内存地址
 * @param[in] mem_size 内存地址长度（8bit/16bit）
 * @param[Out] data 接收数据缓冲区指针
 * @param[in] size 数据长度
 * @param[in] timeout 超时时间（ms）
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_mem_read(en_core_i2c_bus_t bus, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size,
                                    uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (bus >= CORE_I2C_BUS_MAX || (st_i2c_port[bus].st_I2C_HandleTypeDef == NULL)) return CORE_I2C_ERROR;
    //core_i2c_lock(bus);
    core_i2c_switch(bus,EN_HARDWARE_I2C);
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(st_i2c_port[bus].st_I2C_HandleTypeDef, dev_addr, mem_addr, mem_size, data, size, timeout);
    //core_i2c_unlock(bus);
    return (ret == HAL_OK) ? CORE_I2C_OK : CORE_I2C_ERROR;
}

/**
 * @brief 从指定I2C从设备的指定内存地址读取数据（DMA模式）
 * @param[in] bus I2C总线编号
 * @param[in] dev_addr 从设备地址
 * @param[in] mem_addr 内存地址
 * @param[in] mem_size 内存地址长度（8bit/16bit）
 * @param[Out] data 接收数据缓冲区指针
 * @param[in] size 数据长度
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_mem_read_dma(en_core_i2c_bus_t bus, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size,
                                            uint8_t *data, uint16_t size)
{
    if (bus >= CORE_I2C_BUS_MAX || (st_i2c_port[bus].st_I2C_HandleTypeDef == NULL)) return CORE_I2C_ERROR;
    ////core_i2c_lock(bus);
    core_i2c_switch(bus,EN_HARDWARE_I2C);
    HAL_StatusTypeDef ret = \
    HAL_I2C_Mem_Read_DMA(st_i2c_port[bus].st_I2C_HandleTypeDef, \
                        dev_addr, mem_addr, mem_size, data, size);
    ////core_i2c_unlock(bus);
    return (ret == HAL_OK) ? CORE_I2C_OK : CORE_I2C_ERROR;
}
/**
 * @brief 发送I2C软件模拟起始信号
 * @param[in] bus I2C总线编号
 * @param[Out] None
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_soft_start(en_core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX || st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT == NULL) return CORE_I2C_ERROR;
    //core_i2c_lock(bus);
    core_i2c_switch(bus,EN_SOFTWARE_I2C);
    I2CStart(&st_i2c_port[bus].st_iic_bus_inst);
    ////core_i2c_unlock(bus);
    return CORE_I2C_OK;
}

/**
 * @brief 发送I2C软件模拟停止信号
 * @param[in] bus I2C总线编号
 * @param[Out] None
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_soft_stop(en_core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX || st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT == NULL) return CORE_I2C_ERROR;
    I2CStop(&st_i2c_port[bus].st_iic_bus_inst);
    //core_i2c_unlock(bus);
    return CORE_I2C_OK;
}

/**
 * @brief 等待I2C软件模拟模式的从设备应答信号
 * @param[in] bus I2C总线编号
 * @param[Out] None
 * @return CORE_I2C_OK收到应答， CORE_I2C_ERROR未收到应答
 * */
en_core_i2c_status_t core_i2c_soft_wait_ack(en_core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX || st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT == NULL) return CORE_I2C_ERROR;
    unsigned char ret = SUCCESS; // should be ErrorStatus but IICWaitAck(X)
    ret = I2CWaitAck(&st_i2c_port[bus].st_iic_bus_inst);
    if(SUCCESS == ret)
    {
        return CORE_I2C_OK;
    } else {
        return CORE_I2C_ERROR;
    }
}

/**
 * @brief 发送I2C软件模拟模式的ACK应答信号
 * @param[in] bus I2C总线编号
 * @param[Out] None
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_soft_send_ack(en_core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX || st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT == NULL) return CORE_I2C_ERROR;
    I2CSendAck(&st_i2c_port[bus].st_iic_bus_inst);
    return CORE_I2C_OK;
}

/**
 * @brief 发送I2C软件模拟模式的NACK非应答信号
 * @param[in] bus I2C总线编号
 * @param[Out] None
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_soft_send_no_ack(en_core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX || st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT == NULL) return CORE_I2C_ERROR;
    I2CSendNotAck(&st_i2c_port[bus].st_iic_bus_inst);
    return CORE_I2C_OK;
}

/**
 * @brief 通过I2C软件模拟模式发送一个字节数据
 * @param[in] bus I2C总线编号
 * @param[in] data 待发送的字节数据
 * @param[Out] None
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_soft_send_byte(en_core_i2c_bus_t bus,uint8_t data)
{
    if (bus >= CORE_I2C_BUS_MAX || st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT == NULL) return CORE_I2C_ERROR;
    I2CSendByte(&st_i2c_port[bus].st_iic_bus_inst, data);
    return CORE_I2C_OK;
}

/**
 * @brief 通过I2C软件模拟模式接收一个字节数据
 * @param[in] bus I2C总线编号
 * @param[Out] data 接收到的字节数据存放缓冲区指针
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_soft_receive_byte(en_core_i2c_bus_t bus,uint8_t * const data)
{
    if (bus >= CORE_I2C_BUS_MAX || st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT == NULL) return CORE_I2C_ERROR;
    *data = I2CReceiveByte(&st_i2c_port[bus].st_iic_bus_inst);
    return CORE_I2C_OK;
}

/**
 * @brief 初始化I2C软件模拟模式所需的GPIO引脚配置
 * @param[in] bus I2C总线编号
 * @param[Out] None
 * @return CORE_I2C_OK成功， CORE_I2C_ERROR失败
 * */
en_core_i2c_status_t core_i2c_software_init(en_core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX || st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT == NULL) return CORE_I2C_ERROR;
    GPIO_InitTypeDef GPIO_InitStructure = {0};

	core_gpio_clk_enable(st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT);
    core_gpio_clk_enable(st_i2c_port[bus].st_iic_bus_inst.I2C_SCL_PORT);

    GPIO_InitStructure.Pin = st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = st_i2c_port[bus].st_iic_bus_inst.I2C_SCL_PIN ;
    HAL_GPIO_Init(st_i2c_port[bus].st_iic_bus_inst.I2C_SCL_PORT, &GPIO_InitStructure);

    core_gpio_write_pin_direct(st_i2c_port[bus].st_iic_bus_inst.I2C_SCL_PORT,
                               st_i2c_port[bus].st_iic_bus_inst.I2C_SCL_PIN, GPIO_PIN_SET);
    core_gpio_write_pin_direct(st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT,
                               st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PIN, GPIO_PIN_SET);
    return CORE_I2C_OK;
}

/**
 * @brief 在指定I2C总线上切换硬件/软件I2C模式
 * @param[in] bus I2C总线编号
 * @param[in] state 目标模式（EN_HARDWARE_I2C或EN_SOFTWARE_I2C）
 * @param[Out] None
 * @return None
 * */
static void core_i2c_switch(en_core_i2c_bus_t bus, en_i2c_state_t state)
{
	if (bus >= CORE_I2C_BUS_MAX || st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT == NULL) return;
	if(st_i2c_port[bus].en_i2c_state == state)
  {
    //相等，不切换
  }
  else
  {
    if(EN_HARDWARE_I2C == state)
    {
        __HAL_RCC_I2C1_CLK_DISABLE();
        HAL_GPIO_DeInit(st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PORT, st_i2c_port[bus].st_iic_bus_inst.I2C_SDA_PIN);
        HAL_GPIO_DeInit(st_i2c_port[bus].st_iic_bus_inst.I2C_SCL_PORT, st_i2c_port[bus].st_iic_bus_inst.I2C_SCL_PIN);
        //切换硬件I2C
        MX_I2C1_Init();
        st_i2c_port[bus].st_I2C_HandleTypeDef = &hi2c1;
		st_i2c_port[bus].en_i2c_state = EN_HARDWARE_I2C;
    }
    else
    {
      /*切换软件I2C
      需要先把硬件I2C停掉
      停用I2C外设*/
      HAL_I2C_DeInit(st_i2c_port[bus].st_I2C_HandleTypeDef);
      // 禁用I2C时钟
      __HAL_RCC_I2C1_CLK_DISABLE();

      core_i2c_software_init(bus);

	  st_i2c_port[bus].en_i2c_state = EN_SOFTWARE_I2C;
    }
  }
}
