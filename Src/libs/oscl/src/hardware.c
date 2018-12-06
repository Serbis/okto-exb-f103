#include <stdlib.h>
#include <stdbool.h>
#include "../include/harware.h"
#include "../include/malloc.h"
#include <string.h>
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

extern UART_HandleTypeDef huart1;
extern ADC_HandleTypeDef hadc1;
UART_HandleTypeDef *huart = &huart1;
extern uint32_t ADC1ConvertedValues[10];

void Harware_initUart() {
	//Hardware_writeToUart(strcpy2("xxx"));
	SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE);
	// Hardware_writeToUart(strcpy2("yyy"));
}

int32_t Hardware_readAdc(uint8_t number) {
	if (number <= 10 && number != 0) {
		int32_t value = (int32_t) ADC1ConvertedValues[number - 1];
		return value;
	} else {
		return -1;
	}
    /*if (number == 1) {
    	 HAL_ADC_Start(&hadc1);
    	 HAL_ADC_PollForConversion(&hadc1, 100);
    	 uint32_t adc = HAL_ADC_GetValue(&hadc1);
    	 HAL_ADC_Stop(&hadc1);

    	 return adc;
    } else {
    	return  - 1;
    }*/
}

bool Hardware_writeToUart(char *pData) {
    //int fd = open(rxPath, O_RDWR | O_CREAT, 0644);
    //lseek(fd, 0L, SEEK_END);
    size_t len = strlen(pData);
    uint8_t *bf = pmalloc(len);

    memcpy(bf, pData, len);
    taskENTER_CRITICAL();
    huart->gState = HAL_UART_STATE_READY;
    HAL_UART_Transmit(&huart1, bf, len, 1000);
    taskEXIT_CRITICAL();

    //memcpy(bf, pData, len);
    //while (HAL_UART_Transmit(&huart1, bf, len, 0xFFFF) != HAL_OK) {}
    //write(fd, bf, len);
    pfree(bf);
    //close(fd);
    //printf("\nWrited");
    //fflush(stdout);

    return true;
}
