/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PC13_LED_EN_Pin GPIO_PIN_13
#define PC13_LED_EN_GPIO_Port GPIOC
#define PA0_USER_KEY3_Pin GPIO_PIN_0
#define PA0_USER_KEY3_GPIO_Port GPIOA
#define PA0_USER_KEY3_EXTI_IRQn EXTI0_IRQn
#define PA1_LCD_BLACK_Pin GPIO_PIN_1
#define PA1_LCD_BLACK_GPIO_Port GPIOA
#define PA2_Light_ADC_Pin GPIO_PIN_2
#define PA2_Light_ADC_GPIO_Port GPIOA
#define PA3_Heart_Sensor_INT_Pin GPIO_PIN_3
#define PA3_Heart_Sensor_INT_GPIO_Port GPIOA
#define PA4_LCD_CS_Pin GPIO_PIN_4
#define PA4_LCD_CS_GPIO_Port GPIOA
#define PA5_LCD_CLK_Pin GPIO_PIN_5
#define PA5_LCD_CLK_GPIO_Port GPIOA
#define PA6_LCD_DC_Pin GPIO_PIN_6
#define PA6_LCD_DC_GPIO_Port GPIOA
#define PA7_LCD_MOSI_Pin GPIO_PIN_7
#define PA7_LCD_MOSI_GPIO_Port GPIOA
#define PB0_BAT_ADC_Pin GPIO_PIN_0
#define PB0_BAT_ADC_GPIO_Port GPIOB
#define PB1_USER_KEY1_Pin GPIO_PIN_1
#define PB1_USER_KEY1_GPIO_Port GPIOB
#define PB2_TP_TINT_Pin GPIO_PIN_2
#define PB2_TP_TINT_GPIO_Port GPIOB
#define PB2_TP_TINT_EXTI_IRQn EXTI2_IRQn
#define PB10_LCD_RESET_Pin GPIO_PIN_10
#define PB10_LCD_RESET_GPIO_Port GPIOB
#define PB12_FLASH_SPI_NSS_M_Pin GPIO_PIN_12
#define PB12_FLASH_SPI_NSS_M_GPIO_Port GPIOB
#define PB13_FLASH_SPI_CLK_M_Pin GPIO_PIN_13
#define PB13_FLASH_SPI_CLK_M_GPIO_Port GPIOB
#define PB14_FLASH_SPI_MISO_M_Pin GPIO_PIN_14
#define PB14_FLASH_SPI_MISO_M_GPIO_Port GPIOB
#define PB15_FLASH_SPI_MOSI_M_Pin GPIO_PIN_15
#define PB15_FLASH_SPI_MOSI_M_GPIO_Port GPIOB
#define PA8_TP_SCL_Pin GPIO_PIN_8
#define PA8_TP_SCL_GPIO_Port GPIOA
#define PA9_BT_UART_TX_Pin GPIO_PIN_9
#define PA9_BT_UART_TX_GPIO_Port GPIOA
#define PA10_BT_UART_RX_Pin GPIO_PIN_10
#define PA10_BT_UART_RX_GPIO_Port GPIOA
#define PA15_TP_RST_Pin GPIO_PIN_15
#define PA15_TP_RST_GPIO_Port GPIOA
#define PB3_MOTOR_EN_Pin GPIO_PIN_3
#define PB3_MOTOR_EN_GPIO_Port GPIOB
#define PB4_TP_SDA_Pin GPIO_PIN_4
#define PB4_TP_SDA_GPIO_Port GPIOB
#define PB5_MPU6050_INT_Pin GPIO_PIN_5
#define PB5_MPU6050_INT_GPIO_Port GPIOB
#define PB5_MPU6050_INT_EXTI_IRQn EXTI9_5_IRQn
#define PB6_COMMON_SCL_Pin GPIO_PIN_6
#define PB6_COMMON_SCL_GPIO_Port GPIOB
#define PB7_COMMON_SDA_Pin GPIO_PIN_7
#define PB7_COMMON_SDA_GPIO_Port GPIOB
#define PB8_Speaker_Signal_Pin GPIO_PIN_8
#define PB8_Speaker_Signal_GPIO_Port GPIOB
#define PB9_Speaker_Status_Pin GPIO_PIN_9
#define PB9_Speaker_Status_GPIO_Port GPIOB

#define MYDEBUG
#define SUCCESS 0
#define ERROR   1
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
