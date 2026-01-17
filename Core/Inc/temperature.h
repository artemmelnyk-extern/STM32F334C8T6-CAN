/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : temperature.h
  * @brief          : Header for temperature.c file.
  *                   This file contains functions for reading the internal
  *                   temperature sensor with factory calibration.
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
#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"
#include "adc.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* STM32F334 Temperature Sensor Calibration Addresses */
#define TEMP30_CAL_ADDR   ((uint16_t*) ((uint32_t) 0x1FFFF7B8))
#define TEMP110_CAL_ADDR  ((uint16_t*) ((uint32_t) 0x1FFFF7C2))

/* Calibration temperature values */
#define TEMP30_CAL_TEMP   30.0f
#define TEMP110_CAL_TEMP  110.0f

/* ADC reference voltage (V) */
#define VREFINT_CAL_VREF  3.3f

/* Exported functions prototypes ---------------------------------------------*/
uint16_t Temperature_ReadADC(void);
float Temperature_GetCelsius(void);
int16_t Temperature_GetCelsiusInt(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEMPERATURE_H */
