/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : temperature.c
  * @brief          : Internal temperature sensor driver with factory calibration
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

/* Includes ------------------------------------------------------------------*/
#include "temperature.h"

/**
  * @brief  Reads raw ADC value from internal temperature sensor
  * @retval 12-bit ADC reading (0-4095)
  */
uint16_t Temperature_ReadADC(void)
{
  uint16_t adcValue = 0;
  
  /* Start ADC conversion */
  if (HAL_ADC_Start(&hadc1) == HAL_OK)
  {
    /* Wait for conversion to complete (timeout 100ms) */
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
      /* Read the converted value */
      adcValue = (uint16_t)HAL_ADC_GetValue(&hadc1);
    }
    
    /* Stop ADC */
    HAL_ADC_Stop(&hadc1);
  }
  
  return adcValue;
}

/**
  * @brief  Calculates temperature in Celsius using factory calibration
  * @retval Temperature in degrees Celsius (float)
  * 
  * Formula: Temperature = ((110 - 30) / (CAL_110 - CAL_30)) * (ADC_reading - CAL_30) + 30
  */
float Temperature_GetCelsius(void)
{
  uint16_t adcValue;
  uint16_t cal30;
  uint16_t cal110;
  float temperature;
  
  /* Read current ADC value */
  adcValue = Temperature_ReadADC();
  
  /* Read factory calibration values from system memory */
  cal30 = *TEMP30_CAL_ADDR;
  cal110 = *TEMP110_CAL_ADDR;
  
  /* Calculate temperature using two-point calibration
   * 
   * The formula derives from linear interpolation:
   * temp = temp30 + (temp110 - temp30) * (adc - cal30) / (cal110 - cal30)
   * 
   * This accounts for device-specific variations in the temperature sensor
   */
  if (cal110 != cal30)  // Avoid division by zero
  {
    temperature = ((TEMP110_CAL_TEMP - TEMP30_CAL_TEMP) / (float)(cal110 - cal30)) 
                  * (float)(adcValue - cal30) + TEMP30_CAL_TEMP;
  }
  else
  {
    temperature = 25.0f;  // Fallback to room temperature if calibration is invalid
  }
  
  return temperature;
}

/**
  * @brief  Gets temperature in Celsius as signed integer (for CAN transmission)
  * @retval Temperature in degrees Celsius * 10 (e.g., 255 = 25.5°C)
  * 
  * This format allows 0.1°C resolution while using integer arithmetic
  * Range: -3276.8°C to +3276.7°C (suitable for all practical temperatures)
  */
int16_t Temperature_GetCelsiusInt(void)
{
  float tempFloat = Temperature_GetCelsius();
  int16_t tempInt = (int16_t)(tempFloat * 10.0f);
  
  return tempInt;
}
