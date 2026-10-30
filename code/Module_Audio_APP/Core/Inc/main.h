/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32g0xx_hal.h"

#include "stm32g0xx_ll_i2c.h"
#include "stm32g0xx_ll_iwdg.h"
#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_utils.h"
#include "stm32g0xx_ll_pwr.h"
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_dma.h"

#include "stm32g0xx_ll_exti.h"

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
#define LIN_MIC_PU_EN_Pin GPIO_PIN_7
#define LIN_MIC_PU_EN_GPIO_Port GPIOB
#define HP_MIC_EN_Pin GPIO_PIN_0
#define HP_MIC_EN_GPIO_Port GPIOA
#define HP_DET_Pin GPIO_PIN_1
#define HP_DET_GPIO_Port GPIOA
#define HP_MODE_SET_Pin GPIO_PIN_2
#define HP_MODE_SET_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
typedef enum {
  RGB_LED_1 = 0x00, // RGB1
  RGB_LED_2 = 0x01, // RGB2
  RGB_LED_3 = 0x02  // RGB3
} rgb_led_t;        // RGB类型

typedef enum{
	HEADPHONE_UNPLUG = 0x00, // 耳机拔掉
	HEADPHONE_INSERTION = 0x01 // 耳机插入
}headphone_detection_t; // 耳机插入状态

typedef enum{
	MICROPHONE_OFF = 0x00, // 关闭
	MICROPHONE_ON = 0x01   // 打开
}mic_t; // 麦克风状态

typedef enum{
	AMERICA_STANDARD = 0x00, // 美标
  NATIONAL_STANDARD = 0x01 // 国标
} headphone_mode_t; // 模式类型
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
