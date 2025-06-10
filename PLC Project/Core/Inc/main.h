/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32g4xx_hal.h"

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
#define BTN1_Pin GPIO_PIN_13
#define BTN1_GPIO_Port GPIOC
#define AIN1_Pin GPIO_PIN_0
#define AIN1_GPIO_Port GPIOA
#define AIN2_Pin GPIO_PIN_1
#define AIN2_GPIO_Port GPIOA
#define DIN1_Pin GPIO_PIN_2
#define DIN1_GPIO_Port GPIOA
#define DIN2_Pin GPIO_PIN_3
#define DIN2_GPIO_Port GPIOA
#define AOUT1_Pin GPIO_PIN_4
#define AOUT1_GPIO_Port GPIOA
#define AOUT2_Pin GPIO_PIN_5
#define AOUT2_GPIO_Port GPIOA
#define DOUT1_Pin GPIO_PIN_4
#define DOUT1_GPIO_Port GPIOC
#define DOUT2_Pin GPIO_PIN_0
#define DOUT2_GPIO_Port GPIOB
#define DOUT3_Pin GPIO_PIN_1
#define DOUT3_GPIO_Port GPIOB
#define DOUT4_Pin GPIO_PIN_2
#define DOUT4_GPIO_Port GPIOB
#define AIN4_Pin GPIO_PIN_11
#define AIN4_GPIO_Port GPIOB
#define AIN3_Pin GPIO_PIN_12
#define AIN3_GPIO_Port GPIOB
#define DIN3_Pin GPIO_PIN_13
#define DIN3_GPIO_Port GPIOB
#define DIN4_Pin GPIO_PIN_14
#define DIN4_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_6
#define LED_GPIO_Port GPIOC
#define RS485_DIR_Pin GPIO_PIN_8
#define RS485_DIR_GPIO_Port GPIOA
#define RS485_TX_Pin GPIO_PIN_9
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RX_Pin GPIO_PIN_10
#define RS485_RX_GPIO_Port GPIOA
#define BTN2_Pin GPIO_PIN_13
#define BTN2_GPIO_Port GPIOA
#define SD_CS_Pin GPIO_PIN_14
#define SD_CS_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
