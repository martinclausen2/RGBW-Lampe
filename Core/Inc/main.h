/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "stm32l1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "serialLogger.h"
#include "mainmenu.h"
#include "eeprom.h"
#include "encoder.h"
#include "keys.h"
#include "RC5.h"
#include "clock.h"
#include "settings.h"
#include "extbrightness.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define AMP_SHTDN_Pin GPIO_PIN_13
#define AMP_SHTDN_GPIO_Port GPIOC
#define ENC_SW_Pin GPIO_PIN_0
#define ENC_SW_GPIO_Port GPIOA
#define LED_2_PWM_Pin GPIO_PIN_1
#define LED_2_PWM_GPIO_Port GPIOA
#define IR_IN_Pin GPIO_PIN_2
#define IR_IN_GPIO_Port GPIOA
#define Audio_Pin GPIO_PIN_4
#define Audio_GPIO_Port GPIOA
#define Volume_Pin GPIO_PIN_5
#define Volume_GPIO_Port GPIOA
#define STATUS_LED_R_PWM_Pin GPIO_PIN_6
#define STATUS_LED_R_PWM_GPIO_Port GPIOA
#define STATUS_LED_G_PWM_Pin GPIO_PIN_7
#define STATUS_LED_G_PWM_GPIO_Port GPIOA
#define STATUS_LED_B_PWM_Pin GPIO_PIN_0
#define STATUS_LED_B_PWM_GPIO_Port GPIOB
#define LCD_BACKLIGHT_PWM_Pin GPIO_PIN_1
#define LCD_BACKLIGHT_PWM_GPIO_Port GPIOB
#define ESP_RST_Pin GPIO_PIN_2
#define ESP_RST_GPIO_Port GPIOB
#define LED_3_PWM_SCL2_Pin GPIO_PIN_10
#define LED_3_PWM_SCL2_GPIO_Port GPIOB
#define LED_4_PWM_SDA2_Pin GPIO_PIN_11
#define LED_4_PWM_SDA2_GPIO_Port GPIOB
#define IR_OUT_Pin GPIO_PIN_12
#define IR_OUT_GPIO_Port GPIOB
#define LCD_SCK_Pin GPIO_PIN_13
#define LCD_SCK_GPIO_Port GPIOB
#define BRIGHT_IN_Pin GPIO_PIN_14
#define BRIGHT_IN_GPIO_Port GPIOB
#define LCD_MOSI_Pin GPIO_PIN_15
#define LCD_MOSI_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_8
#define LCD_RST_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_11
#define LCD_CS_GPIO_Port GPIOA
#define LCD_A0_Pin GPIO_PIN_12
#define LCD_A0_GPIO_Port GPIOA
#define LED_1_PWM_Pin GPIO_PIN_15
#define LED_1_PWM_GPIO_Port GPIOA
#define BRIGHT_HIGH_Pin GPIO_PIN_3
#define BRIGHT_HIGH_GPIO_Port GPIOB
#define BRIGHT_LOW_Pin GPIO_PIN_4
#define BRIGHT_LOW_GPIO_Port GPIOB
#define ENC_B_Pin GPIO_PIN_6
#define ENC_B_GPIO_Port GPIOB
#define ENC_A_Pin GPIO_PIN_7
#define ENC_A_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
