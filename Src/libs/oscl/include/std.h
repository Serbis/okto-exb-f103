/*
 * std.h
 *
 *  Created on: 28 июл. 2018 г.
 *      Author: serbis
 */

#ifndef LIBS_OSCL_INCLUDE_STD_H_
#define LIBS_OSCL_INCLUDE_STD_H_

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdarg.h>

/** UART Handle*/
extern UART_HandleTypeDef huart1;

void _printf(char *str, ...);

#endif /* LIBS_OSCL_INCLUDE_STD_H_ */
