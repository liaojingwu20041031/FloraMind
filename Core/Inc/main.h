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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//头文件
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
//全局变量
extern uint8_t sb_num_yy,bg_num_yy,fs_num_yy,sb_num_yy2,bg_num_yy2,fs_num_yy2,yy_num2;
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
#define DHT11_Pin GPIO_PIN_1
#define DHT11_GPIO_Port GPIOA
#define tr_sd_adc_IN2_Pin GPIO_PIN_2
#define tr_sd_adc_IN2_GPIO_Port GPIOA
#define gq_adc_IN3_Pin GPIO_PIN_3
#define gq_adc_IN3_GPIO_Port GPIOA
#define SGP30_SCL_Pin GPIO_PIN_10
#define SGP30_SCL_GPIO_Port GPIOB
#define SGP30_SDA_Pin GPIO_PIN_11
#define SGP30_SDA_GPIO_Port GPIOB
#define tw_TX_Pin GPIO_PIN_8
#define tw_TX_GPIO_Port GPIOD
#define tw_RX_Pin GPIO_PIN_9
#define tw_RX_GPIO_Port GPIOD
#define xsp_TX_Pin GPIO_PIN_9
#define xsp_TX_GPIO_Port GPIOA
#define xsp_RX_Pin GPIO_PIN_10
#define xsp_RX_GPIO_Port GPIOA
#define LED_GPIO_Pin GPIO_PIN_0
#define LED_GPIO_GPIO_Port GPIOD
#define csj_GPIO_Pin GPIO_PIN_1
#define csj_GPIO_GPIO_Port GPIOD
#define fs_GPIO_Pin GPIO_PIN_3
#define fs_GPIO_GPIO_Port GPIOD
#define wifi_TX_Pin GPIO_PIN_5
#define wifi_TX_GPIO_Port GPIOD
#define wifi_RX_Pin GPIO_PIN_6
#define wifi_RX_GPIO_Port GPIOD
#define gymk_GPIO_Pin GPIO_PIN_0
#define gymk_GPIO_GPIO_Port GPIOE
#define zdmk_GPIO_Pin GPIO_PIN_1
#define zdmk_GPIO_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
