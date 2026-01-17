/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : adc.h
  * @brief          : Header for adc.c file.
  *                   This file contains the ADC initialization for internal
  *                   temperature sensor.
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
#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* Exported types ------------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;

/* Exported functions prototypes ---------------------------------------------*/
void MX_ADC1_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H */
