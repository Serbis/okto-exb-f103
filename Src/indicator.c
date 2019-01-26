#include <stdbool.h>
#include "stm32f1xx_it.h"
#include "../Inc/indicator.h"
#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim1;
extern bool led_indic;
//extern bool led_light = true;
extern uint8_t led_mode;

void Indicator_cmdIn() {
	led_indic = true;
	led_mode = LED_MODE_RIPPLE;
	HAL_TIM_Base_Start_IT(&htim1);
}

void Indicator_cmdOut() {
	led_indic = false;
}
