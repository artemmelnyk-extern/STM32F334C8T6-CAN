/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "can.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "adc.h"
#include "temperature.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
CAN_RxHeaderTypeDef rxHeader; // CAN Bus Receive Header
CAN_TxHeaderTypeDef txHeaderA1; // CAN Bus Transmit Header for A1 (DEADBEEF)
CAN_TxHeaderTypeDef txHeaderT1; // CAN Bus Transmit Header for T1 (Temperature NMEA 2000)
uint32_t canMailbox; // CAN Bus Mail box variable
CAN_FilterTypeDef canfil; // CAN Bus Filter
uint32_t a1Counter = 0; // Counter for A1 transmission (30 Hz = every 3-4 cycles)
uint32_t t1Counter = 0; // Counter for T1 transmission (1 Hz = every 30 cycles)
uint8_t nmea2000_sid = 0; // NMEA 2000 Sequence ID
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_CAN_ConfigFilter(&hcan, &canfil); // Initialize CAN Filter
  HAL_CAN_Start(&hcan);//
  
  /* Setup A1 CAN message header (DEADBEEF pattern) */
  txHeaderA1.DLC = 8;
  txHeaderA1.IDE = CAN_ID_STD;
  txHeaderA1.RTR = CAN_RTR_DATA;
  txHeaderA1.StdId = 0x0A1; // A1 message ID
  txHeaderA1.TransmitGlobalTime = DISABLE;
  
  /* Setup T1 CAN message header (NMEA 2000 PGN 130312 - Temperature) */
  txHeaderT1.DLC = 8;
  txHeaderT1.IDE = CAN_ID_EXT; // NMEA 2000 uses extended CAN ID
  txHeaderT1.RTR = CAN_RTR_DATA;
  // NMEA 2000 format: Priority=6, PGN=130312 (0x1FD08), Source=0x01
  // 29-bit ID = (Priority << 26) | (PGN << 8) | Source
  txHeaderT1.ExtId = (6UL << 26) | (130312UL << 8) | 0x01; // 0x19FD0801
  txHeaderT1.TransmitGlobalTime = DISABLE;
  
  /* Calibrate ADC */
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
   // Main loop runs every ~33ms for 30Hz A1 transmission
   HAL_Delay(33U);
   (void) HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);

   // Send A1 message (DEADBEEF) at 30 Hz
   a1Counter++;
   uint8_t a1Data[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x00};
   HAL_CAN_AddTxMessage(&hcan, &txHeaderA1, a1Data, &canMailbox);

   // Send T1 message (Temperature NMEA 2000 PGN 130312) at 1 Hz
   t1Counter++;
   if (t1Counter >= 30) // 30 cycles * 33ms ≈ 1 second
   {
     t1Counter = 0;
     
     // Read temperature sensor
     float tempCelsius = Temperature_GetCelsius();
     
     // NMEA 2000 PGN 130312 - Temperature format
     // Temperature in Kelvin with 0.01K resolution: temp_value = (Celsius + 273.15) * 100
     uint16_t tempKelvin = (uint16_t)((tempCelsius + 273.15f) * 100.0f);
     
     uint8_t t1Data[8];
     t1Data[0] = nmea2000_sid++;              // SID (Sequence ID) - increment each message
     t1Data[1] = 0;                            // Temperature Instance (0 = single sensor)
     t1Data[2] = 1;                            // Temperature Source (1 = Inside Temperature)
     t1Data[3] = (uint8_t)(tempKelvin & 0xFF); // Temperature low byte
     t1Data[4] = (uint8_t)(tempKelvin >> 8);   // Temperature high byte
     t1Data[5] = 0xFF;                         // Reserved
     t1Data[6] = 0xFF;                         // Reserved
     t1Data[7] = 0xFF;                         // Reserved
     
     // Send NMEA 2000 temperature message
     HAL_CAN_AddTxMessage(&hcan, &txHeaderT1, t1Data, &canMailbox);
   }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
