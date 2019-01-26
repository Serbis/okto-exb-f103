/*
 * NRF24.h
 *
 *  Created on: 5 янв. 2019 г.
 *      Author: serbis
 */

#ifndef NRF24_H_
#define NRF24_H_

#define HW_STM32
//#define HW_PI
//#define HW_PI_DEBUG

//------------------------------------------------

#ifdef HW_STM32
	#include "stm32f1xx_hal.h"
#endif

#ifdef HW_PI
	#include "time.h"
	#include "spi.h"
	#include "gpio.h"
	#include <stdint.h>
    #include <wiringPi.h>
#endif


#include <string.h>
#include <stdbool.h>


//------------------------------------------------

#ifdef HW_STM32
    SPI_HandleTypeDef NRF_spid;
#endif

#ifdef HW_PI
    int NRF_cePin;
    struct SpiDescriptor *NRF_spid;
#endif


//------------------------------------------------

#define CS_GPIO_PORT GPIOB

#define CS_PIN GPIO_PIN_6

#ifdef HW_STM32
    #define CS_ON HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET)
    #define CS_OFF HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET)
#endif

#ifdef HW_PI
#define CS_ON

#define CS_OFF

#endif

#define CE_GPIO_PORT GPIOB

#define CE_PIN GPIO_PIN_8

#ifdef HW_STM32
    #define CE_RESET HAL_GPIO_WritePin(CE_GPIO_PORT, CE_PIN, GPIO_PIN_RESET)
    #define CE_SET HAL_GPIO_WritePin(CE_GPIO_PORT, CE_PIN, GPIO_PIN_SET)
#endif

#ifdef HW_PI
    #define CE_RESET digitalWrite(NRF_cePin, LOW);
    #define CE_SET  digitalWrite(NRF_cePin, HIGH);
#endif


#define IRQ_GPIO_PORT GPIOB

#ifdef HW_STM32
    #define IRQ_PIN GPIO_PIN_7
#endif

#ifdef HW_PI
    #define IRQ_PIN 22
#endif


#define IRQ HAL_GPIO_ReadPin(IRQ_GPIO_PORT, IRQ_PIN)

#define LED_GPIO_PORT GPIOC

#define LED_PIN GPIO_PIN_13

#define LED_ON HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_RESET)

#define LED_OFF HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_SET)

#define LED_TGL HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN)


#define _BV(x) (1<<(x))

#ifdef HW_STM32

    #define delayMs(ms) DWT_Delay(ms)
    #define spi_transmit(dev, bf, size, timeout) HAL_SPI_TransmitReceive(&dev,bf,bf,size,timeout)
    #define read_irq_pin() HAL_GPIO_ReadPin(IRQ_GPIO_PORT, IRQ_PIN)
#endif

#ifdef HW_PI
    #define delayMs(ms) delayMicroseconds(ms)
    #define spi_transmit(dev, bf, size, timeout) SPI_transmit(dev,bf,size)
    #define read_irq_pin() digitalRead(IRQ_PIN)
#endif

//------------------------------------------------

#define ACTIVATE 0x50 //

#define RD_RX_PLOAD 0x61 // Define RX payload register address

#define WR_TX_PLOAD 0xA0 // Define TX payload register address

#define FLUSH_TX 0xE1

#define FLUSH_RX 0xE2

//------------------------------------------------

#define CONFIG 0x00 //'Config' register address

#define EN_AA 0x01 //'Enable Auto Acknowledgment' register address

#define EN_RXADDR 0x02 //'Enabled RX addresses' register address

#define SETUP_AW 0x03 //'Setup address width' register address

#define SETUP_RETR 0x04 //'Setup Auto. Retrans' register address

#define RF_CH 0x05 //'RF channel' register address

#define RF_SETUP 0x06 //'RF setup' register address

#define STATUS 0x07 //'Status' register address

#define OBSERVE_TX 0x08 //'Transmit observe' register

#define RX_ADDR_P0 0x0A //'RX address pipe0' register address

#define RX_ADDR_P1 0x0B //'RX address pipe1' register address

#define TX_ADDR 0x10 //'TX address' register address

#define RX_PW_P0 0x11 //'RX payload width, pipe0' register address

#define RX_PW_P1 0x12 //'RX payload width, pipe1' register address

#define FIFO_STATUS 0x17 //'FIFO Status Register' register address

#define DYNPD 0x1C

#define FEATURE 0x1D

//------------------------------------------------

#define PRIM_RX 0x00 //RX/TX control (1: PRX, 0: PTX)

#define PWR_UP 0x01 //1: POWER UP, 0:POWER DOWN

#define RX_DR 0x40 //Data Ready RX FIFO interrupt

#define TX_DS 0x20 //Data Sent TX FIFO interrupt

#define MAX_RT 0x10 //Maximum number of TX retransmits interrupt

//------------------------------------------------

#define W_REGISTER 0x20 //запись в регистр

//------------------------------------------------

#define TX_PLOAD_WIDTH 32

//------------------------------------------------

//Self transmitter net address
uint32_t a_self;

//Master pipe adress (node from packet will be expected in RX mode)
uint32_t a_master;

//------------------------------------------------

void NRF24_init();
//void NRF24_WriteReg(uint8_t addr, uint8_t dt);
//uint8_t NRF24_ReadReg(uint8_t addr);
uint8_t NRF24_init_check();
uint8_t NRF24_Send(uint8_t *pBuf);
int NRF24_Receive(uint8_t *payload);
bool NRF24_available();

//------------------------------------------------

#endif /* NRF24_H_ */

