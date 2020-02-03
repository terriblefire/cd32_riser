/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#ifndef EVALBOARD
	#define KBD_CLOCK_Pin GPIO_PIN_2
	#define KBD_CLOCK_GPIO_Port GPIOC
	#define KBD_DATA_Pin GPIO_PIN_3
	#define KBD_DATA_GPIO_Port GPIOC
	#define KBD_RESET_Pin GPIO_PIN_1
	#define KBD_RESET_GPIO_Port GPIOC
	#define TP1_Pin GPIO_PIN_0
	#define TP1_GPIO_Port GPIOA
	#define TP2_Pin GPIO_PIN_1
	#define TP2_GPIO_Port GPIOA
#else
	#define KBD_CLOCK_Pin GPIO_PIN_0
	#define KBD_CLOCK_GPIO_Port GPIOC
	#define KBD_DATA_Pin GPIO_PIN_1
	#define KBD_DATA_GPIO_Port GPIOC
	#define KBD_RESET_Pin GPIO_PIN_2
	#define KBD_RESET_GPIO_Port GPIOC
	#define TP1_Pin GPIO_PIN_10
	#define TP1_GPIO_Port GPIOB
	#define TP2_Pin GPIO_PIN_11
	#define TP2_GPIO_Port GPIOB
#endif
#define DEBUG_TX_Pin GPIO_PIN_2
#define DEBUG_TX_GPIO_Port GPIOA
#define DEBUG_RX_Pin GPIO_PIN_3
#define DEBUG_RX_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA



/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
