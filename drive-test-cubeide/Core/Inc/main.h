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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user_app.h"

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
#define CS2_Mouse_Pin GPIO_PIN_12
#define CS2_Mouse_GPIO_Port GPIOB
#define INT_Mouse_Pin GPIO_PIN_8
#define INT_Mouse_GPIO_Port GPIOD
#define LED_B1_Pin GPIO_PIN_10
#define LED_B1_GPIO_Port GPIOC
#define LED_nR0_Pin GPIO_PIN_2
#define LED_nR0_GPIO_Port GPIOD
#define LED_B0_Pin GPIO_PIN_6
#define LED_B0_GPIO_Port GPIOD
#define LED_LG0_Pin GPIO_PIN_6
#define LED_LG0_GPIO_Port GPIOB
#define LED_Y0_Pin GPIO_PIN_7
#define LED_Y0_GPIO_Port GPIOB
#define LED_nR0E0_Pin GPIO_PIN_0
#define LED_nR0E0_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
