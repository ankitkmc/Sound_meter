/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "lora.h"
#include "oled_display/SH1106.h"
#include "oled_display/fonts.h"
#include "oled_display/bitmap.h"
#include "sound_meter/svan958a.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define TRANSMITTER		0
#define RECEIVER		1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

uint8_t init_cplt_flag = ERROR;
uint8_t over_flag = ERROR;
uint8_t packet_ready = ERROR;
uint8_t rx_buff;
uint8_t rx_count = 0;
uint8_t buffer[MAX_BYTES] = {0};

uint8_t rxIndex = 0;
uint8_t dataReceived = 0;
uint8_t rxBuffer[15] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#if RECEIVER
	if ((huart == &huart1) && (init_cplt_flag == SUCCESS))
	{
		buffer[rx_count++] = rx_buff;

		if (rx_count == PACKET_LENGTH)
		{
			packet_ready = SUCCESS;
		}

		/* Continue receiving */
		HAL_UART_Receive_IT(&huart1,(uint8_t *)&rx_buff, 1);
	}
#elif TRANSMITTER
	if (huart == &SVAN_UART_HANDLE)
	{
		if (rxBuffer[rxIndex] == '\n')
		{
			dataReceived = 1;
		}
		else
		{
			rxIndex++;
			if (rxIndex >= 12)
			{
				rxIndex = 0;
			}
		}

		HAL_UART_Receive_IT(&huart2, (uint8_t *)&rxBuffer[rxIndex], 1);
	}
#endif
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

#if TRANSMITTER
	uint8_t command[] = "LPO?\r\n";
	static uint32_t previousCount;
	static uint32_t deviceTimeoutCount;
	char noiseData[6];
#endif

#if RECEIVER
	static bool clearScreen = false;
	static float noiseMaxValue = 0.0;
	uint8_t bufferMaxVal[50];
#endif

#define TRASNSPARENT	1
#define RELAY			0
#define WOR				0

#if TRASNSPARENT
//	uint8_t transparent_string[] = "This is a transparent message\r\n";
#elif RELAY
	uint8_t relay_string[] = "This is a relay message\r\n";
#elif WOR
	uint8_t wor_string[] = "This is a wor message\r\n";
#endif

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
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_Delay(1000); //don't delete wait for lora reset

#if TRASNSPARENT
  while(ERROR == lora_cfg_mode(TRANSPARENT_MODE))
  {
	  HAL_Delay(500);
  }
#elif RELAY
  while(ERROR == lora_cfg_mode(RELAY_MODE))
  {
	  HAL_Delay(500);
  }
#elif WOR
  while(ERROR == lora_cfg_mode(WOR_TRANSMISSION_MODE))
  {
	  HAL_Delay(500);
  }
#endif

  	HAL_Delay(100);

#if RECEIVER
  	// Initialize LoRa for reception
  	HAL_UART_Receive_IT(&huart1, (uint8_t *)&rx_buff, 1);

  	// Initialize display driver
  	SH1106_Init();

  	// Startup display
  	SH1106_GotoXY (25, 25);
	SH1106_Puts((char *)"ESCORTS", &Font_11x18, 1);
	SH1106_UpdateScreen();
	HAL_Delay(3000);
	SH1106_Clear();

  	// GUI
  	SH1106_GotoXY (10, 5);
	SH1106_Puts((char *)"Noise (dB)", &Font_7x10, 1);
  	SH1106_GotoXY (20, 20);
  	SH1106_Puts((char *)"0.00", &Font_16x26, 1);
	SH1106_DrawLine(7, 45, 121, 45, 1);
	SH1106_GotoXY (10, 52);
	SH1106_Puts((char *)"MAX", &Font_7x10, 1);
	SH1106_GotoXY (87, 52);
	SH1106_Puts((char *)"0.00", &Font_7x10, 1);

	SH1106_UpdateScreen();

#elif TRANSMITTER
	// Start driver
//	svan_init();
	HAL_UART_Receive_IT(&SVAN_UART_HANDLE, &rxBuffer[rxIndex], 1);

	// Get initial count
	previousCount = HAL_GetTick();
	deviceTimeoutCount = HAL_GetTick();

	// Configure session and START the SVAN
//	svan_config_and_start();
#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

#if TRASNSPARENT
//	  if(delay++ > 18000000)
//	  {
//		  lora_send((int8_t *)transparent_string);
//		  delay = 0;
//	  }

#elif RELAY
	  if(delay++ > 18000000)
	  {
		  lora_send((int8_t *)relay_string);
		  delay = 0;
	  }
#elif WOR
	  if(delay++ > 18000000)
	  {
		  delay = 0;
		  lora_send((int8_t *)wor_string);
		  LED_ON();
		  while(delay++ < 30000);
		  LED_OFF();
		  delay = 0;
	  }
#endif

#if TRANSMITTER
	  if(HAL_GetTick() - previousCount > 1000)
	  {
		  HAL_UART_Transmit(&huart2, (uint8_t *)command, strlen((char *)command), HAL_MAX_DELAY);
		  previousCount = HAL_GetTick();
		  LED_Toggle();
	  }

	  if (dataReceived)
	  {
		  dataReceived = 0;

		  // Parse response
		  memcpy((char *)noiseData, (char *)&rxBuffer[1], 6);
		  rxIndex = 0;

		  lora_send((uint8_t *)noiseData);

		  // Reset timeout anchor (we had activity)
		  deviceTimeoutCount = HAL_GetTick();

		  HAL_UART_Receive_IT(&huart2, (uint8_t *)&rxBuffer[rxIndex], 1);

		  memset((char *)rxBuffer, 0x00, strlen((char *)rxBuffer));
	  }
	  else
	  {
		  // If no data received for 5 seconds, send fallback and re-arm timer
		  if ((HAL_GetTick() - deviceTimeoutCount) >= 3000U)
		  {
			  lora_send((uint8_t *)"00.00 ");   		// or "0.0" if that is the required format
			  deviceTimeoutCount = HAL_GetTick(); 		// next fallback in another 5 sec if still idle
		  }
	  }

#endif

#if RECEIVER
	  /* Loop back the received packet (*only for testing purpose*) */
	  if(packet_ready == SUCCESS)
	  {
		  packet_ready = ERROR;

		  if(!clearScreen)
		  {
			  clearScreen = true;
			  SH1106_Clear();
		  }

		  double NoiseValue = atof((char *)buffer);

		  if(NoiseValue > noiseMaxValue)
		  {
			  noiseMaxValue = NoiseValue;
		  }

		  sprintf((char *)buffer, (char *)"%.2f ", NoiseValue);
		  sprintf((char *)bufferMaxVal, (char *)"%.2f ", noiseMaxValue);

		  SH1106_GotoXY (10, 5);
		  SH1106_Puts((char *)"Noise (dB)", &Font_7x10, 1);
		  SH1106_GotoXY (20, 20);
		  SH1106_Puts((char *)buffer, &Font_16x26, 1);
		  SH1106_DrawLine(7, 45, 121, 45, 1);
		  SH1106_GotoXY (10, 52);
		  SH1106_Puts((char *)"MAX", &Font_7x10, 1);
		  SH1106_GotoXY (87, 52);
		  SH1106_Puts((char *)bufferMaxVal, &Font_7x10, 1);

		  SH1106_UpdateScreen();

		  // reset for next packet
		  rx_count = 0;
		  memset((char *)buffer, 0x00, strlen((char *)buffer));
	  }
#endif

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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_Pin|M0_Pin|M1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_Pin M0_Pin M1_Pin */
  GPIO_InitStruct.Pin = LED_Pin|M0_Pin|M1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
