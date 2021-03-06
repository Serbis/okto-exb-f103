
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <event_manager.h>
#include <event_producer.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */

#include "stm32f1xx_hal_iwdg.h"
#include "stm32f1xx_it.h"
#include "../Inc/eeprom.h"
#include <stdbool.h>
#include "../Inc/uart_receiver.h"
#include "../Inc/rf_receiver.h"
#include "../Inc/exb_gate.h"
#include "../Inc/proc.h"
#include "../Inc/transmitter.h"
#include "libs/oscl/include/threads.h"
#include "libs/oscl/include/data.h"
#include "libs/oscl/include/harware.h"
#include "libs/collections/include/lbq.h"
#include "libs/collections/include/lbq8.h"
#include "libs/collections/include/rings.h"
#include "libs/misc/include/dwt_delay.h"
#include "../Inc/defines.h"
#include "../Inc/adc.h"
#include "../Inc/executor.h"
#include "../Inc/config.h"
#include "../Inc/NRF24.h"
#include "../Inc/event_cotroller.h"
#include "../Inc/proc.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;

osThreadId defaultTaskHandle;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

RingBufferDef *uartRing = NULL;
Config *config;
char str1[20] = {0};
uint8_t buf1[20] = {0};
uint8_t dt_reg=0;
extern mutex_t *rf_mutex;
extern InterruptCut interruptCuts[23];

// This variables used for sort/man reset operations, see freertos default task
bool soft_reset = false;
bool man_reset = false;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
static void MX_IWDG_Init(void);
static void MX_SPI1_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  uartRing = RINGS_createRingBuffer(256, RINGS_OVERFLOW_SHIFT, 0);
  RingBufferDef *rfGateRing =	RINGS_createRingBuffer(256, RINGS_OVERFLOW_SHIFT, 1);
  RingBufferDef *uartGateRing =	RINGS_createRingBuffer(256, RINGS_OVERFLOW_SHIFT, 1);
  LinkedBlockingQueue *procQueue = new_LQB(15);
  LinkedBlockingQueue *transmitterQueue = new_LQB(15);
  QueueHandle_t pendingEventStream = xQueueCreate(5, 4);
  QueueHandle_t ackEventStream = xQueueCreate(5, 4);

  //Config_Reset(CONFIG_ADDRESS);
  config = Config_Load(CONFIG_ADDRESS);

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  DWT_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  //Start indicator timer
  HAL_TIM_Base_Start_IT(&htim1);

  a_self = config->a_self;
  a_master = config->a_master;

  rf_mutex = NewMutex();
  NRF_spid = hspi1;
  NRF24_init();
  NRF24_init_check();

  Harware_initUart();
  ADC_init();

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */

  osThreadDef(rfReceiverTask, RfReceiver_thread, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(rfReceiverTask), rfGateRing);

  UartReceiverThreadArgs *uartReceiverArgs = (UartReceiverThreadArgs*) pmalloc(sizeof(UartReceiverThreadArgs));
  uartReceiverArgs->uartRing = uartRing;
  uartReceiverArgs->gateRing = uartGateRing;
  osThreadDef(uartReceiverTask, UartReceiver_thread, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(uartReceiverTask), uartReceiverArgs);

  ExbGateThreadArgs *exbGateArgs1 = (ExbGateThreadArgs*) pmalloc(sizeof(ExbGateThreadArgs));
  exbGateArgs1->upRing = rfGateRing;
  exbGateArgs1->downQueue = procQueue;
  exbGateArgs1->marker = FROM_RF;
  exbGateArgs1->ackEventStream = ackEventStream;
  osThreadDef(exbRfGate, ExbGate_thread, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(exbRfGate), exbGateArgs1);

  ExbGateThreadArgs *exbGateArgs2 = (ExbGateThreadArgs*) pmalloc(sizeof(ExbGateThreadArgs));
  exbGateArgs2->upRing = uartGateRing;
  exbGateArgs2->downQueue = procQueue;
  exbGateArgs2->marker = FROM_UART;
  exbGateArgs2->ackEventStream = ackEventStream;
  osThreadDef(exbUartGate, ExbGate_thread, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(exbUartGate), exbGateArgs2);

  ProcThreadArgs *procArgs = (ProcThreadArgs*) pmalloc(sizeof(ProcThreadArgs));
  procArgs->upQueue = procQueue;
  procArgs->downQueue = transmitterQueue;
  osThreadDef(proc, Proc_thread, osPriorityNormal, 0, 200);
  defaultTaskHandle = osThreadCreate(osThread(proc), procArgs);

  osThreadDef(transmitter, Transmitter_thread, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(transmitter), transmitterQueue);

  EventsProducerThreadArgs *evpThreadArgs = (EventsProducerThreadArgs*) pmalloc(sizeof(EventsProducerThreadArgs));
  evpThreadArgs->pendingEventStream = pendingEventStream;
  osThreadDef(eventProducer, EventsProducer_thread, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(eventProducer), evpThreadArgs);

  EventsControllerThreadArgs *evcThreadArgs = (EventsControllerThreadArgs*) pmalloc(sizeof(EventsControllerThreadArgs));
  evcThreadArgs->pendingEventStream = pendingEventStream;
  evcThreadArgs->ackEventStream = ackEventStream;
  evcThreadArgs->transmitterQueue = transmitterQueue;
  osThreadDef(eventController, EventController_thread, osPriorityNormal, 0, 64);
  defaultTaskHandle = osThreadCreate(osThread(eventController), evcThreadArgs);

  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
 

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 10;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime =  ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_9;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_10;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* IWDG init function */
static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 1000;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 36000;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();


  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, G0_o_Pin|G1_o_Pin|G2_o_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, G13_o_Pin|G14_o_Pin|G15_o_Pin|G16_o_Pin 
                          |G17_o_Pin|G18_o_Pin|G19_o_Pin|NRF_CE_Pin 
                          |LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, G20_o_Pin|G21_o_Pin|G22_o_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(NRF_CS_GPIO_Port, NRF_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : G0_o_Pin G1_o_Pin G2_o_Pin */
  GPIO_InitStruct.Pin = G0_o_Pin|G1_o_Pin|G2_o_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : G13_o_Pin G14_o_Pin G15_o_Pin G16_o_Pin 
                           G17_o_Pin G18_o_Pin G19_o_Pin */
  GPIO_InitStruct.Pin = G13_o_Pin|G14_o_Pin|G15_o_Pin|G16_o_Pin 
                          |G17_o_Pin|G18_o_Pin|G19_o_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : G20_o_Pin */
  GPIO_InitStruct.Pin = G20_o_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(G20_o_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : G21_o_Pin G22_o_Pin */
  GPIO_InitStruct.Pin = G21_o_Pin|G22_o_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NRF_CS_Pin NRF_CE_Pin LED_Pin */
  GPIO_InitStruct.Pin = NRF_CS_Pin|NRF_CE_Pin|LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : NRF_IRQ_Pin */
  GPIO_InitStruct.Pin = NRF_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(NRF_IRQ_GPIO_Port, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument) {


	GPIO_InitTypeDef GPIO_InitStruct;

	// ----------------- exb gpio configuration -----------------------

	for (uint8_t i = 0; i <= 22; i++) { //Configure exb gpio system
	  if (i < 2 || i > 12) { // For all not adc and not pin 2 (button irq pair)
		  char *pin_s = itoa2(i);
		  GpioMap *gp = Executor_map_gpio(pin_s);
		  uint16_t set = 0;
		  memcpy(&set, &(config->gpiom[i]), 2);
		  uint16_t mode = (set & 0b1111000000000000) >> 12;
		  if (mode == 0) { // OUT
			  uint16_t state = (set & 0b0000100000000000) >> 11;

			  GPIO_InitStruct.Pin = gp->pin;
			  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
			  GPIO_InitStruct.Pull = GPIO_NOPULL;
			  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			  HAL_GPIO_Init(gp->port, &GPIO_InitStruct);

			  if (state == 0)
				  HAL_GPIO_WritePin(gp->port, gp->pin, GPIO_PIN_RESET);
			  else
				  HAL_GPIO_WritePin(gp->port, gp->pin, GPIO_PIN_SET);

		  } else if (mode == 1) { // IN
			  uint16_t pull = (set & 0b0000110000000000) >> 10;

			  GPIO_InitStruct.Pin = gp->pin;
			  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

			  if (pull == 0)
				  GPIO_InitStruct.Pull = GPIO_PULLUP;
			  else if (pull == 1)
				  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
			  else
				  GPIO_InitStruct.Pull = GPIO_NOPULL;

			  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			  HAL_GPIO_Init(gp->port, &GPIO_InitStruct);


		  } else if (mode == 2) { // IRQ
			  uint16_t mode = (set & 0b0000110000000000) >> 10;
			  uint16_t pull = (set & 0b0000001100000000) >> 8;
			  uint16_t cutoff = (set & 0b0000000011100000) >> 5;

			  GPIO_InitStruct.Pin = gp->pin;

			  if (mode == 0) {
				  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
			  } else if (mode == 1) {
				  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
			  } else {
				  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
			  }

			  if (pull == 0)
				  GPIO_InitStruct.Pull = GPIO_PULLUP;
			  else if (pull == 1)
			  	  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
			  else
				  GPIO_InitStruct.Pull = GPIO_NOPULL;

			  HAL_GPIO_Init(gp->port, &GPIO_InitStruct);

			  uint16_t c = 0;
			  if (cutoff == 1)
				  c = 1;
			  else if (cutoff == 2)
				  c = 10;
			  else if (cutoff == 3)
				  c = 50;
			  else if (cutoff == 4)
				  c = 100;
			  else if (cutoff == 5)
				  c = 250;
			  else if (cutoff == 6)
				  c = 500;
			  else if (cutoff == 7)
				  c = 1000;


			  InterruptCut cut = {0, c};
			  uint16_t complPin = Executor_pinToGpio(gp->pin);
			  interruptCuts[complPin] = cut;
			  interruptCuts[i] = cut;
		  }

		  pfree(pin_s);
		  pfree(gp);
	  }
	}

	  /* EXTI interrupt init*/
	  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 6, 0);
	  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	  HAL_NVIC_SetPriority(EXTI4_IRQn, 7, 0);
	  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	  HAL_NVIC_SetPriority(EXTI3_IRQn, 8, 0);
	  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	  HAL_NVIC_SetPriority(EXTI2_IRQn, 9, 0);
	  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

	  HAL_NVIC_SetPriority(EXTI1_IRQn, 10, 0);
	  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

	  HAL_NVIC_SetPriority(EXTI0_IRQn, 11, 0);
	  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	// -------------------------------------------------------------

	 MX_IWDG_Init();

	 for(;;) {
		  // reset_mode set by button interrupt handler
		  if (soft_reset) { //soft reset mode
			; //do not refresh iwdg, mcu will be reset at iwdg overflow ( <= 1000 ms )
		  } else if (man_reset) { //man reset mode
			  Config_Reset(CONFIG_ADDRESS); //clear config magic, it will fully reset firmware configuration
			  //do not refresh iwdg, mcu will be reset at iwdg overflow ( <= 1000 ms )
		  } else {
			  HAL_IWDG_Refresh(&hiwdg);
		  }

		  vTaskDelay(200);
	 }
  /* USER CODE END 5 */ 
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
