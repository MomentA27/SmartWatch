//
// Created by 35540 on 2026/4/21.
//
//******************************** Includes *********************************//
#include "gpio_port.h"
#include "gpio_define.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//

//******************************** Defines **********************************//
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
// GPIO引脚配置表
static const st_gpio_pin_config_t gpio_pin_configs[CORE_GPIO_PIN_MAX] =
{
    //显示相关GPIO
    [CORE_GPIO_TEMPHUMI_SDA] = {
        .port = Sensor_I2C_SDA_Port,
        .pin  = Sensor_I2C_SDA_Pin,
        .default_state = CORE_GPIO_PIN_SET
    },
    [CORE_GPIO_TEMPHUMI_SCL] = {
        .port = Sensor_I2C_SCL_Port,
        .pin  = Sensor_I2C_SCL_Pin,
        .default_state = CORE_GPIO_PIN_SET
    },
};

/**
 * @brief GPIO平台初始化
 * @param[in] None
 * @param[out] None
 * @return en_core_gpio_status_t 初始化状态
 */
en_core_gpio_status_t core_gpio_init(void)
{
    // 设置所有GPIO引脚为默认状态
    for(uint8_t i = 0; i < CORE_GPIO_PIN_MAX; i++)
    {
        if(gpio_pin_configs[i].port != NULL)
        {
            GPIO_PinState state = (gpio_pin_configs[i].default_state == CORE_GPIO_PIN_SET) ?
                                  GPIO_PIN_SET : GPIO_PIN_RESET;
            HAL_GPIO_WritePin(gpio_pin_configs[i].port, gpio_pin_configs[i].pin, state);
        }
    }
    return CORE_GPIO_OK;
}

/**
 * @brief GPIO端口到RCC AHB1ENR使能位的映射表
 */
typedef struct {
    GPIO_TypeDef *port;
    uint32_t      ahb1enr_mask;
} gpio_clk_map_t;

static const gpio_clk_map_t gpio_clk_table[] = {
    { GPIOA, RCC_AHB1ENR_GPIOAEN },
    { GPIOB, RCC_AHB1ENR_GPIOBEN },
    { GPIOC, RCC_AHB1ENR_GPIOCEN },
};

/**
 * @brief 使能指定GPIO端口的RCC时钟
 * @param[in] port GPIO端口地址
 */
void core_gpio_clk_enable(GPIO_TypeDef *port)
{
    for (uint8_t i = 0; i < sizeof(gpio_clk_table) / sizeof(gpio_clk_table[0]); i++)
    {
        if (gpio_clk_table[i].port == port)
        {
            SET_BIT(RCC->AHB1ENR, gpio_clk_table[i].ahb1enr_mask);
            return;
        }
    }
}

/**
 * @brief 写GPIO引脚状态
 * @param[in] pin GPIO引脚枚举
 * @param[in] state 引脚状态
 * @param[out] None
 * @return en_core_gpio_status_t 操作状态
 */
en_core_gpio_status_t core_gpio_write_pin(en_core_gpio_pin_t number, en_core_gpio_pin_state_t state)
{
    if(number >= CORE_GPIO_PIN_MAX)
    {
        return CORE_GPIO_ERROR;
    }

    if(gpio_pin_configs[number].port == NULL)
    {
        return CORE_GPIO_ERROR;
    }

    GPIO_PinState hal_state = (state == CORE_GPIO_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(gpio_pin_configs[number].port, gpio_pin_configs[number].pin, hal_state);

    return CORE_GPIO_OK;
}

/**
 * @brief 读GPIO引脚状态
 * @param[in] pin GPIO引脚枚举
 * @param[out] None
 * @return en_core_gpio_pin_state_t 引脚状态
 */
en_core_gpio_pin_state_t core_gpio_read_pin(en_core_gpio_pin_t number)
{
    if(number >= CORE_GPIO_PIN_MAX || gpio_pin_configs[number].port == NULL)
    {
        return CORE_GPIO_PIN_RESET;
    }

    GPIO_PinState hal_state = HAL_GPIO_ReadPin(gpio_pin_configs[number].port, gpio_pin_configs[number].pin);

    return (hal_state == GPIO_PIN_SET) ? CORE_GPIO_PIN_SET : CORE_GPIO_PIN_RESET;
}

/**
 * @briwpopyTwef 翻转GPIO引脚状态
 * @param[in] pin GPIO引脚枚举
 * @param[out] None
 * @return en_core_gpio_status_t 操作状态
 */
en_core_gpio_status_t core_gpio_toggle_pin(en_core_gpio_pin_t number)
{
    if(number >= CORE_GPIO_PIN_MAX)
    {
        return CORE_GPIO_ERROR;
    }

    if(gpio_pin_configs[number].port == NULL)
    {
        return CORE_GPIO_ERROR;
    }

    HAL_GPIO_TogglePin(gpio_pin_configs[number].port, gpio_pin_configs[number].pin);

    return CORE_GPIO_OK;
}

/**
 * @brief 直接写GPIO引脚状态（兼容HAL库接口）
 * @param[in] GPIOx GPIO端口
 * @param[in] GPIO_Pin GPIO引脚
 * @param[in] PinState 引脚状态
 * @param[out] None
 * @return en_core_gpio_status_t 操作状态
 */
en_core_gpio_status_t core_gpio_write_pin_direct(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    if(GPIOx == NULL)
    {
        return CORE_GPIO_ERROR;
    }

    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState);

    return CORE_GPIO_OK;
}

/**
 * @brief 直接读GPIO引脚状态（兼容HAL库接口）
 * @param[in] GPIOx GPIO端口
 * @param[in] GPIO_Pin GPIO引脚
 * @param[out] None
 * @return GPIO_PinState 引脚状态
 */
GPIO_PinState core_gpio_read_pin_direct(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    if(GPIOx == NULL)
    {
        return GPIO_PIN_RESET;
    }

    return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
}

/**
 * @brief 直接翻转GPIO引脚状态（兼容HAL库接口）
 * @param[in] GPIOx GPTwpwtxoIO端口
 * @param[in] GPIO_Pin GPIO引脚
 * @param[out] None
 * @return en_core_gpio_status_t 操作状态
 */
en_core_gpio_status_t core_gpio_toggle_pin_direct(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    if(GPIOx == NULL)
    {
        return CORE_GPIO_ERROR;
    }

    HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);

    return CORE_GPIO_OK;
}