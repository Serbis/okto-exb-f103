/*
 * adc.h
 *
 *  Created on: 26 окт. 2018 г.
 *      Author: serbis
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdbool.h>
#include <stdint.h>

#define ADC_COUNT 10

bool ADC_init();
int32_t ADC_readFilteredVaule(uint8_t channel, uint16_t iterations, uint16_t delay);

#endif /* ADC_H_ */
