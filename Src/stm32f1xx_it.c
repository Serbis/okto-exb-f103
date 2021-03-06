/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <event_producer.h>
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include "stm32f1xx_it.h"
#include "cmsis_os.h"

/* USER CODE BEGIN 0 */

#include <stdbool.h>
#include "../Inc/executor.h"
#include "libs/collections/include/lbq.h"
#include "libs/oscl/include/threads.h"
#include "libs/collections/include/rings.h"
#include "../Inc/adc.h"

extern UART_HandleTypeDef huart1;
extern RingBufferDef *uartRing;
extern uint32_t *ADC1ConvertedValues;
UART_HandleTypeDef *huartx = &huart1;

//---- Filtration mechanics (for more details see DMA1_Channel1_IRQHandler) ----

extern uint32_t filterSum;
extern uint8_t filrerChannel;
extern uint16_t filterIteration;
extern bool filterCompleted;
extern bool filterNw;
extern ADC_HandleTypeDef hadc1;

//---- Indicator logic

bool led_indic = false;
bool led_light = true;
uint8_t led_mode = LED_MODE_RIPPLE;

// ---- Button logic

TickType_t btn_hold = 0;

// ---- Reset mechanics

extern bool soft_reset;
extern bool man_reset;

// --- Interrupt cuts

InterruptCut interruptCuts[23];

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart1;

/******************************************************************************/
/*            Cortex-M3 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
* @brief This function handles Hard fault interrupt.
*/
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
  /* USER CODE BEGIN HardFault_IRQn 1 */

  /* USER CODE END HardFault_IRQn 1 */
}

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
  /* USER CODE BEGIN MemoryManagement_IRQn 1 */

  /* USER CODE END MemoryManagement_IRQn 1 */
}

/**
* @brief This function handles Prefetch fault, memory access fault.
*/
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
  /* USER CODE BEGIN BusFault_IRQn 1 */

  /* USER CODE END BusFault_IRQn 1 */
}

/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
  /* USER CODE BEGIN UsageFault_IRQn 1 */

  /* USER CODE END UsageFault_IRQn 1 */
}

/**
* @brief This function handles Debug monitor.
*/
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  osSystickHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles DMA1 channel1 global interrupt.
*/
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* Filtration mechanic. In this code, data is collected to filter the values of ADCs. The essence of the code is reduced
   * to the summation of the value from the target channel of the ADC to a special variable until the number of iterations
   * is zero. Initially, the iteration number is a positive number equal to the width of the filtering array - 1. For each
   * value entered, the iteration counter decreases by one and a new ADC conversion is launched. When the counter reaches
   * zero, the re-conversion no longer starts, and the filter completion flag is set to true. */

  if (filterIteration > 0) {
	  filterIteration--;
	  if (!filterNw)
		  filterSum = filterSum + ADC1ConvertedValues[filrerChannel];
	  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) ADC1ConvertedValues, ADC_COUNT);
  } else {
	  filterCompleted = true;
  }

  /* End of filteration mechanic */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
* @brief This function handles TIM1 update interrupt.
*/
void TIM1_UP_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_IRQn 0 */
	if (led_indic) {
		if (led_mode == LED_MODE_RIPPLE) {
			if (led_light == true) {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
				led_light = false;
			} else {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
				led_light = true;
			}
		}
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
		led_light = true;
		HAL_TIM_Base_Stop_IT(&htim1);
	}

  /* USER CODE END TIM1_UP_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  /* USER CODE BEGIN TIM1_UP_IRQn 1 */

  /* USER CODE END TIM1_UP_IRQn 1 */
}

/**
* @brief This function handles USART1 global interrupt.
*/
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

	uint8_t value = (uint8_t)(huartx->Instance->DR & (uint8_t)0x00FF);
	/*if (counter > 2) {
		inBuf->enqueue(inBuf, value);
	}*/
	/*if (value == '\r') {
		RINGS_write(value, inBuf);
		uint16_t len = RINGS_dataLenght(inBuf);
		char *str = pmalloc(len + 1);
		str[len] = 0;
		RINGS_extractData(inBuf->writer - len, len, (uint8_t *) str, inBuf);
		RINGS_dataClear(inBuf);
		cmdQueue->enqueue(cmdQueue, str);
	} else {*/
		RINGS_write(value, uartRing);
	//}


	//if (cmdQueue != NULL)
			//RINGS_write((uint8_t)(huart1->Instance->DR & (uint8_t)0x00FF), inputFNPRbd);
  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

uint16_t EXTI_detectPhisicalPin() {
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0)) return 0;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1)) return GPIO_PIN_1;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2)) return GPIO_PIN_2;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3)) return GPIO_PIN_3;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4)) return GPIO_PIN_4;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5)) return GPIO_PIN_5;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6)) return GPIO_PIN_6;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_7)) return GPIO_PIN_7;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_8)) return GPIO_PIN_8;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_9)) return GPIO_PIN_9;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10)) return GPIO_PIN_10;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_11)) return GPIO_PIN_11;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12)) return GPIO_PIN_12;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13)) return GPIO_PIN_13;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_14)) return GPIO_PIN_14;
	else if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15)) return GPIO_PIN_15;

	return 0;
}

void EXTI_handleEventsLogic(uint16_t pin) {
	uint8_t exbPin = Executor_pinToGpio(pin);

	time_t curt = xTaskGetTickCountFromISR();
	if (curt > interruptCuts[exbPin].last + interruptCuts[exbPin].threshold) {
		if (eventStream != NULL) {
			EventDef event = { 0, exbPin };
			xQueueSendFromISR(eventStream, &event, 0);
		}

		interruptCuts[exbPin].last = curt;
	}

}

void EXTI15_10_IRQHandler(void)
{
	uint16_t pin = EXTI_detectPhisicalPin();

	if (pin == GPIO_PIN_15) { // Button
		if (btn_hold == 0) { // Btn pushed
			btn_hold = xTaskGetTickCountFromISR();
		} else { //Btn up
			TickType_t diff = xTaskGetTickCountFromISR() - btn_hold;
			btn_hold = 0;
			if (diff > 100 && diff < 8000) { // If click is not a noise error and it hold is less that man reset - soft reset
				soft_reset = true;
			} else { // If hold > 8 sec - man reset
				man_reset = true;
			}
		}
	} else { // External interrupts
		EXTI_handleEventsLogic(pin);
	}

	HAL_GPIO_EXTI_IRQHandler(pin);
}

void EXTI9_5_IRQHandler(void) {

}

void EXTI4_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}
void EXTI3_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}
void EXTI2_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}
void EXTI1_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}
void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
