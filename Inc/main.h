/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define G0_o_Pin GPIO_PIN_13
#define G0_o_GPIO_Port GPIOC
#define G1_o_Pin GPIO_PIN_14
#define G1_o_GPIO_Port GPIOC
#define G2_o_Pin GPIO_PIN_15
#define G2_o_GPIO_Port GPIOC
#define G3_a_o_Pin GPIO_PIN_0
#define G3_a_o_GPIO_Port GPIOA
#define G4_a_o_Pin GPIO_PIN_1
#define G4_a_o_GPIO_Port GPIOA
#define G5_a_o_Pin GPIO_PIN_2
#define G5_a_o_GPIO_Port GPIOA
#define G6_a_o_Pin GPIO_PIN_3
#define G6_a_o_GPIO_Port GPIOA
#define G7_a_o_Pin GPIO_PIN_4
#define G7_a_o_GPIO_Port GPIOA
#define G8_a_o_Pin GPIO_PIN_5
#define G8_a_o_GPIO_Port GPIOA
#define G9_a_o_Pin GPIO_PIN_6
#define G9_a_o_GPIO_Port GPIOA
#define G10_a_o_Pin GPIO_PIN_7
#define G10_a_o_GPIO_Port GPIOA
#define G11_a_o_Pin GPIO_PIN_0
#define G11_a_o_GPIO_Port GPIOB
#define G12_a_o_Pin GPIO_PIN_1
#define G12_a_o_GPIO_Port GPIOB
#define G13_o_Pin GPIO_PIN_2
#define G13_o_GPIO_Port GPIOB
#define G14_o_Pin GPIO_PIN_10
#define G14_o_GPIO_Port GPIOB
#define G15_o_Pin GPIO_PIN_11
#define G15_o_GPIO_Port GPIOB
#define G16_o_Pin GPIO_PIN_12
#define G16_o_GPIO_Port GPIOB
#define G17_o_Pin GPIO_PIN_13
#define G17_o_GPIO_Port GPIOB
#define G18_o_Pin GPIO_PIN_14
#define G18_o_GPIO_Port GPIOB
#define G19_o_Pin GPIO_PIN_15
#define G19_o_GPIO_Port GPIOB
#define G20_o_Pin GPIO_PIN_8
#define G20_o_GPIO_Port GPIOA
#define UART_TX_Pin GPIO_PIN_9
#define UART_TX_GPIO_Port GPIOA
#define UART_RX_Pin GPIO_PIN_10
#define UART_RX_GPIO_Port GPIOA
#define G21_o_Pin GPIO_PIN_11
#define G21_o_GPIO_Port GPIOA
#define G22_o_Pin GPIO_PIN_12
#define G22_o_GPIO_Port GPIOA
#define BUTTON_Pin GPIO_PIN_15
#define BUTTON_GPIO_Port GPIOA
#define BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define NRF_SCK_Pin GPIO_PIN_3
#define NRF_SCK_GPIO_Port GPIOB
#define NRF_MISO_Pin GPIO_PIN_4
#define NRF_MISO_GPIO_Port GPIOB
#define NRF_MOSI_Pin GPIO_PIN_5
#define NRF_MOSI_GPIO_Port GPIOB
#define NRF_CS_Pin GPIO_PIN_6
#define NRF_CS_GPIO_Port GPIOB
#define NRF_IRQ_Pin GPIO_PIN_7
#define NRF_IRQ_GPIO_Port GPIOB
#define NRF_CE_Pin GPIO_PIN_8
#define NRF_CE_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_9
#define LED_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */



/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
