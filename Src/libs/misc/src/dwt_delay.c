/*
 * Simple microseconds delay routine, utilizingARM's DWT
 * (Data Watchpoint and Trace Unit) and HAL library.
 * Intended to use with gcc compiler, but can be easily edited
 * for any other C compiler.
 * Max K
 *
 *
 * This file is part of DWT_Delay package.
 * DWT_Delay is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * us_delay is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 * http://www.gnu.org/licenses/.
 */

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "../include/dwt_delay.h"

#define    DWT_CYCCNT    *(volatile unsigned long *)(DWT_BASE + 0x004UL)
#define    DWT_CONTROL   *(volatile unsigned long *)DWT_BASE
#define    SCB_DEMCR     *(volatile unsigned long *)(CoreDebug_BASE + 0x00CUL)

void DWT_Init(void)
{
    //if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        //CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        //DWT->CYCCNT = 0;
        //DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    //}

	//разрешаем использовать счётчик
	SCB_DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	//обнуляем значение счётного регистра
	DWT_CYCCNT  = 0;
	//запускаем счётчик
	DWT_CONTROL |= DWT_CTRL_CYCCNTENA_Msk;
}

static __inline uint32_t delta(uint32_t t0, uint32_t t1)
{
    return (t1 - t0);
}

/**
 * Delay routine itself.
 * Time is in microseconds (1/1000000th of a second), not to be
 * confused with millisecond (1/1000th).
 *
 * @param uint32_t us  Number of microseconds to delay for
 */
void DWT_Delay(uint32_t us) // microseconds
{
  //int32_t targetTick = DWT_CYCCNT + us * (SystemCoreClock/1000000);
  //while (DWT_CYCCNT <= targetTick);
	uint32_t t0 =  DWT_CYCCNT;
	uint32_t us_count_tic =  us * (SystemCoreClock/1000000);
	while (delta(t0, DWT_CYCCNT) < us_count_tic) ;
}
