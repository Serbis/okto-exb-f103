#include "../Inc/adc.h"
#include "../libs/oscl/include/malloc.h"
#include "../libs/oscl/include/time.h"
#include "stm32f1xx_hal.h"

extern ADC_HandleTypeDef hadc1;

/** Filter summator */
uint32_t filterSum;

/* Filtered ADC Channel */
uint8_t filrerChannel;

/* Current filtering iteration */
uint16_t filterIteration;

/* Flag closure filtering ADC value  */
bool filterCompleted;

/* Not write data to summator (used by dma interrupt in delay mode) */
bool filterNw;

/* Ring buffer for adc dma conversions */
uint32_t *ADC1ConvertedValues;


bool ADC_init() {
	ADC1ConvertedValues = (uint32_t*) pmalloc(4 * ADC_COUNT);
	return true;
}

/* Returns the arithmetic average value of the ADC channel for the specified number of iterations. The function can work
 * either in the direct mode, when the values ​​are read in the scanning mode without delay, or in the deceleration mode,
 * when between each subsequent reading of the ADC, a short time delay occurs. The delay variable is responsible for this
 * mode. If it is 0, it will be read in scan mode. If there is some positive number in it, then it is taken as the number
 * of ticks that will pass between repeated readings. This function can be used to directly read the ADC channel without
 * filtering, if set in the number of iterations to 1. */
int32_t ADC_readFilteredVaule(uint8_t channel, uint16_t iterations, uint16_t delay) {
	//Reset summator
	filterSum = 0;

	//Set filtered adc channel
	filrerChannel = channel;

	//Set start filter iteration
	if (delay != 0) {
		filterNw = true; //Prevent column write in interrupt
		filterIteration = 0;
	} else {
		filterNw = false;
		filterIteration = iterations - 1;
	}

	//This varible count iterations in delay mode
	uint16_t interIterations = iterations - 1;

	//Reset filtration complete flag
	filterCompleted = false;

	//Start first adc conversion
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) ADC1ConvertedValues, ADC_COUNT);

	while(true) {
		while(!filterCompleted) {}
		if (delay == 0)
			break;
		//If in delay mode
		filterSum = filterSum + ADC1ConvertedValues[filrerChannel];
		if (interIterations == 0)
			break;
		DelayMillis(delay);
		interIterations--;
		filterCompleted = false;
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) ADC1ConvertedValues, ADC_COUNT);
	}

	int32_t x = filterSum / iterations;

	return x;
}
