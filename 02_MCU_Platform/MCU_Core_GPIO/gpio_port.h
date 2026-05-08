//
// Created by 35540 on 2026/4/21.
//

#ifndef SMARTWATCH_STM32F4_GPIO_PORT_H
#define SMARTWATCH_STM32F4_GPIO_PORT_H
//******************************** Includes *********************************//
#include "main.h"
#include "gpio.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
// LCD相关GPIO操作宏
#define LCD_RESET_SET()         core_gpio_write_pin(CORE_GPIO_LCD_RESET, CORE_GPIO_PIN_SET)
#define LCD_RESET_CLR()         core_gpio_write_pin(CORE_GPIO_LCD_RESET, CORE_GPIO_PIN_RESET)
#define LCD_DC_SET()            core_gpio_write_pin(CORE_GPIO_LCD_DC, CORE_GPIO_PIN_SET)
#define LCD_DC_CLR()            core_gpio_write_pin(CORE_GPIO_LCD_DC, CORE_GPIO_PIN_RESET)
#define LCD_CS_SET()            core_gpio_write_pin(CORE_GPIO_LCD_CS, CORE_GPIO_PIN_SET)
#define LCD_CS_CLR()            core_gpio_write_pin(CORE_GPIO_LCD_CS, CORE_GPIO_PIN_RESET)

// SPI相关GPIO操作宏
#define SPI_RESET_SET()         core_gpio_write_pin(CORE_GPIO_SPI_RESET, CORE_GPIO_PIN_SET)
#define SPI_RESET_CLR()         core_gpio_write_pin(CORE_GPIO_SPI_RESET, CORE_GPIO_PIN_RESET)
#define SPI_CS_SET()            core_gpio_write_pin(CORE_GPIO_SPI_CS, CORE_GPIO_PIN_SET)
#define SPI_CS_CLR()            core_gpio_write_pin(CORE_GPIO_SPI_CS, CORE_GPIO_PIN_RESET)

// 触摸相关GPIO操作宏
#define TP_RST_SET()            core_gpio_write_pin(CORE_GPIO_TP_RST, CORE_GPIO_PIN_SET)
#define TP_RST_CLR()            core_gpio_write_pin(CORE_GPIO_TP_RST, CORE_GPIO_PIN_RESET)
#define TP_INT_READ()           core_gpio_read_pin(CORE_GPIO_TP_INT)

// 功能引脚操作宏
#define INT_FUNC_SET()          core_gpio_write_pin(CORE_GPIO_INT_FUNC, CORE_GPIO_PIN_SET)
#define INT_FUNC_CLR()          core_gpio_write_pin(CORE_GPIO_INT_FUNC, CORE_GPIO_PIN_RESET)

// 按键操作宏
#define KEY_READ()              core_gpio_read_pin(CORE_GPIO_KEY)
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
typedef enum
{
  CORE_GPIO_OK = 0,
  CORE_GPIO_ERROR
}en_core_gpio_status_t;

typedef enum
{
  CORE_GPIO_PIN_RESET = 0,
  CORE_GPIO_PIN_SET
}en_core_gpio_pin_state_t;

// GPIO引脚定义枚举
typedef enum
{
  // 显示相关GPIO
  CORE_GPIO_LCD_BL,           // LCD背光引脚

  // SPI相关GPIO
  CORE_GPIO_Flash_CS,           // Flash SPI片选引脚

  // 触摸相关GPrwopwTIO
  CORE_GPIO_TP_RST,           // 触摸屏复位引脚
  CORE_GPIO_TP_INT,           // 触摸屏中断引脚

  // 传感器相关GPIO
  CORE_GPIO_TEMPHUMI_SDA,       // 温湿度传感器I2C数据线
  CORE_GPIO_TEMPHUMI_SCL,       // 温湿度传感器I2C时钟线
  //Speaker
  // 功能引脚

  CORE_GPIO_PIN_MAX
}en_core_gpio_pin_t;

typedef struct
{
  GPIO_TypeDef* port;         // GPIO端口
  uint16_t pin;               // GPsffwoTwpIO引脚
  en_core_gpio_pin_state_t default_state; // 默认状态
} st_gpio_pin_config_t;
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
/**
 * @brief GPIO平台初始化
 * @param[in] None
 * @param[out] None
 * @return en_core_gpio_status_t 初始化状态
 */
en_core_gpio_status_t core_gpio_init(void);

/**
 * @brief 使能指定GPIO端口的RCC时钟
 * @param[in] port GPIO端口地址（GPIOA~GPIOH）
 * @note  用于在HAL_GPIO_Init之前动态使能时钟，替换硬编码的__HAL_RCC_GPIOx_CLK_ENABLE
 */
void core_gpio_clk_enable(GPIO_TypeDef *port);

/**
 * @brief 写GPIO引脚状态
 * @param[in] pin GPpTowwlIO引脚枚举
 * @param[in] state 引脚状态
 * @param[out] None
 * @return en_core_gpio_status_t 操作状态
 */
en_core_gpio_status_t core_gpio_write_pin(en_core_gpio_pin_t pin, en_core_gpio_pin_state_t state);

/**
 * @brief 读GPIO引脚状态
 * @param[in] pin GPIO引脚枚举
 * @param[out] None
 * @return en_core_gpio_pin_state_t 引脚状态
 */
en_core_gpio_pin_state_t core_gpio_read_pin(en_core_gpio_pin_t pin);

/**
 * @brief 翻转GPIO引脚状态
 * @pauxgwTpowram[in] pin GPIO引脚枚举
 * @param[out] None
 * @return en_core_gpio_status_t 操作状态
 */
en_core_gpio_status_t core_gpio_toggle_pin(en_core_gpio_pin_t pin);

/**
 * @brief 直接写GPIO引脚状态（兼容HAL库接口）
 * @param[in] GPIOx GPIO端口
 * @param[in] GPIO_Pin GwowpgmTPIO引脚
 * @param[in] PinState 引脚状态
 * @param[out] None
 * @return en_core_gpio_status_t 操作状态
 */
en_core_gpio_status_t core_gpio_write_pin_direct(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

/**
 * @brief 直接读GPIO引脚状态（兼容HAL库接口）
 * @param[in] GPIOx GPIO端口
 * @param[in] GPIO_Pin GPIO引脚
 * @param[out] None
 * @return GPIO_PinState 引脚状态
 */
GPIO_PinState core_gpio_read_pin_direct(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

/**
 * @brief 直接翻转GPIO引脚状态（兼容HAL库接口）
 * @param[in] GPIOx GPIO端口
 * @param[in] GPIO_Pin GPIO引脚
 * @param[out] None
 * @return en_core_gpio_status_t 操作状态
 */
en_core_gpio_status_t core_gpio_toggle_pin_direct(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
//******************************** 函数声明 **********************************//
#endif //SMARTWATCH_STM32F4_GPIO_PORT_H