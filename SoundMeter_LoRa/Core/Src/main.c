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

#include <stdbool.h>
#include "lora.h"
#include "oled_display/SH1106.h"
#include "oled_display/fonts.h"
#include "oled_display/bitmap.h"

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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

uint8_t init_cplt_flag = ERROR;
uint8_t over_flag = ERROR;
uint8_t packet_ready = ERROR;
uint8_t rx_buff;
uint8_t rx_count = 0;
uint8_t buffer[MAX_BYTES] = {0};
uint8_t txBuff[25] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if ((huart == &huart1) && (init_cplt_flag == SUCCESS))
	{
		buffer[rx_count++] = rx_buff;

		if (rx_count >= PACKET_LENGTH)
		{
			packet_ready = SUCCESS;
			rx_count = 0; // reset for next packet
			memcpy(txBuff, buffer, strlen((const char *)buffer));
		}

		/* Continue receiving */
		HAL_UART_Receive_IT(&huart1,(uint8_t *)&rx_buff, 1);
	}
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

	uint32_t delay = 0;
//	char noiseStr[4] = "41.8";
//	char vibratStr[4] = "23.5";

#define TRASNSPARENT
//#define RELAY
//#define WOR

#ifdef TRASNSPARENT
	uint8_t transparent_string[] = "This is a transparent message\r\n";
#elif defined RELAY
	uint8_t relay_string[] = "This is a relay message\r\n";
#elif defined WOR
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
  /* USER CODE BEGIN 2 */

  HAL_Delay(1000); //don't delete wait for lora reset

#ifdef TRASNSPARENT
  while(ERROR == lora_cfg_mode(TRANSPARENT_MODE))
  {
	  HAL_Delay(500);
  }
#elif defined RELAY
  while(ERROR == lora_cfg_mode(RELAY_MODE))
  {
	  HAL_Delay(500);
  }
#elif defined WOR
  while(ERROR == lora_cfg_mode(WOR_TRANSMISSION_MODE))
  {
	  HAL_Delay(500);
  }
#endif

  	HAL_Delay(100);
  	HAL_UART_Receive_IT(&huart1, (uint8_t *)&rx_buff, 1);

  	SH1106_Init();
  	SH1106_GotoXY (25, 25);
  	SH1106_Puts((char *)"ESCORTS", &Font_11x18, 1);
//	SH1106_DrawBitmap(2, 0, logo, 128, 64, 1);  // 132x64, so leave 2 from both sides, draw from 0,0
//  	SH1106_Clear();
//  	SH1106_DrawRectangle(2, 0, 128, 64, 1);
//  	SH1106_DrawLine(64, 0, 64, 64, 1);
//  	SH1106_DrawLine(0, 32, 128, 32, 1);
//  	SH1106_GotoXY (15, 12);
//  	SH1106_Puts((char *)"Noise", &Font_7x10, 1);
//  	SH1106_GotoXY (10, 44);
//	SH1106_Puts((char *)"Vibrat.", &Font_7x10, 1);
	SH1106_UpdateScreen();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

#ifdef TRASNSPARENT
	  if(delay++ > 18000000)
	  {
		  lora_send((uint8_t *)transparent_string);
		  delay = 0;
		  LED_Toggle();
	  }
#elif defined	RELAY
	  if(delay++ > 18000000)
	  {
		  lora_send((int8_t *)relay_string);
		  delay = 0;
	  }
#elif defined WOR
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

	  /* Loop back the received packet (*only for testing purpose*) */
	  if(packet_ready == SUCCESS)
	  {
		  packet_ready = ERROR;
		  lora_send((uint8_t *)buffer);
		  memset(txBuff, 0x00, strlen((const char *)txBuff));

//		  memcpy(noiseStr, &buffer[0], 4);
//		  memcpy(vibratStr, &buffer[4], 4);

		  SH1106_Clear();
		  SH1106_Puts((char *)buffer, &Font_7x10, 1);
//		  SH1106_DrawRectangle(2, 0, 128, 64, 1);
//		  SH1106_DrawLine(64, 0, 64, 64, 1);
//		  SH1106_DrawLine(0, 32, 128, 32, 1);
//		  SH1106_GotoXY (15, 12);
//		  SH1106_Puts((char *)"Noise", &Font_7x10, 1);
//		  SH1106_GotoXY (79, 12);
//		  SH1106_Puts((char *)noiseStr, &Font_7x10, 1);
//		  SH1106_GotoXY (10, 44);
//		  SH1106_Puts((char *)"Vibrat.", &Font_7x10, 1);
//		  SH1106_GotoXY (79, 44);
//		  SH1106_Puts((char *)vibratStr, &Font_7x10, 1);

		  SH1106_UpdateScreen();
	  }
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, M0_Pin|M1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : M0_Pin M1_Pin */
  GPIO_InitStruct.Pin = M0_Pin|M1_Pin;
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
