/** This file contains exb command handlers. Any command handler is a function that takes as
 *  input arguments as strings and returns an ExecutorResult structure that describes the result
 *  of terminating the handler */

#include <stdarg.h>
#include "stm32f1xx_it.h"
#include "adc.h"
#include "executor.h"
#include "config.h"
#include "stdlibo.h"
#include "exb_packet.h"
#include "NRF24.h"
#include "event_manager.h"
#include "event_producer.h"
#include "../libs/oscl/include/data.h"

extern Config *config;
extern InterruptCut interruptCuts[23];



//=============================================== UTILITES ================================================

/** Map the gpio virtual identifier to the physical value of the port and pin number. Return null if pin indenitifer does
 *  not exist */
GpioMap* Executor_map_gpio(char *pin) {
	GpioMap *map = pmalloc(sizeof(GpioMap));
	map->adc = false;
	if (strcmp(pin, "0") == 0) {
		map->port = GPIOC;
		map->pin = GPIO_PIN_13;
	} else if (strcmp(pin, "1") == 0) {
		map->port = GPIOC;
		map->pin = GPIO_PIN_14;
	} else if (strcmp(pin, "2") == 0) {
		map->port = GPIOC;
		map->pin = GPIO_PIN_15;
	} else if (strcmp(pin, "3") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_0;
		map->adc = true;
	} else if (strcmp(pin, "4") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_1;
		map->adc = true;
	} else if (strcmp(pin, "5") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_2;
		map->adc = true;
	} else if (strcmp(pin, "6") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_3;
		map->adc = true;
	} else if (strcmp(pin, "7") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_4;
		map->adc = true;
	} else if (strcmp(pin, "8") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_5;
		map->adc = true;
	} else if (strcmp(pin, "9") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_6;
		map->adc = true;
	} else if (strcmp(pin, "10") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_7;
		map->adc = true;
	} else if (strcmp(pin, "11") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_0;
		map->adc = true;
	} else if (strcmp(pin, "12") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_1;
		map->adc = true;
	} else if (strcmp(pin, "13") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_2;
	} else if (strcmp(pin, "14") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_10;
	} else if (strcmp(pin, "15") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_11;
	} else if (strcmp(pin, "16") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_12;
	} else if (strcmp(pin, "17") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_13;
	} else if (strcmp(pin, "18") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_14;
	} else if (strcmp(pin, "19") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_15;
	}  else if (strcmp(pin, "20") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_8;
	} else if (strcmp(pin, "21") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_11;
	} else if (strcmp(pin, "22") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_12;
	} else {
		pfree(map);
		return NULL;
	}

	return map;
}

/**
 * Performs mapping of the hardware value of pin to pin exb. This function is used by interrupt
 * handlers to determine by which gpio number it occurred. Since interrupts are multiplexed,
 * although interruptions can be assigned to each individual gpio pin, there are pins that have
 * one interrupt handler. For this reason, this function will always return the smallest possible
 * gpio number. For example, physical pins PC13 and PB13 correspond to Exb pins 0 and 17. When
 * assigning an interrupt to both of these pins, they will fall into the same interrupt handler, in
 * which it will be impossible to determine the port of the physical pin, and therefore it cannot
 * be said what the Exb pin is 0 or 17. In this case, the function will return the smallest value
 * of 0. This fact is very important to understand when working with events, in that part, that
 * event handlers should always be assigned to the smallest number of composite pins, regardless
 * of the actual position of the handler in the Exb gpio map.
 */
uint8_t Executor_pinToGpio(uint16_t gpioPin) {
	if (gpioPin == 13) return 0;
	else if (gpioPin == GPIO_PIN_14) return 1;
	else if (gpioPin == GPIO_PIN_0) return 3;
	else if (gpioPin == GPIO_PIN_1) return 4;
	else if (gpioPin == GPIO_PIN_2) return 5;
	else if (gpioPin == GPIO_PIN_3) return 6;
	else if (gpioPin == GPIO_PIN_4) return 7;
	else if (gpioPin == GPIO_PIN_5) return 8;
	else if (gpioPin == GPIO_PIN_6) return 9;
	else if (gpioPin == GPIO_PIN_7) return 10;
	else if (gpioPin == GPIO_PIN_10) return 14;
	else if (gpioPin == GPIO_PIN_11) return 15;
	else if (gpioPin == GPIO_PIN_12) return 16;
	else if (gpioPin == GPIO_PIN_15) return 19;
	else if (gpioPin == GPIO_PIN_8) return 20;
	else return 0;
}

void Executor_disable_adc(GpioMap *pin) {

}

/** Create new ExecutorResult struct as exb response, receive repsonse string and format. Work as printf
 * function */
ExecutorResult* Executor_response(char *data, ...) {
	size_t size = strlen(data) + MAX_RET_PAYLOAD + 1;
	char *buf = malloc(size);

	va_list args;
	va_start(args, data);
	vsprintf(buf, data, args);
	va_end(args);

	ExecutorResult *r = (ExecutorResult*) malloc(sizeof(ExecutorResult));
	r->type = EXB_TYPE_RESPONSE;
	r->size = strlen(buf);
	r->data = buf;

	return r;
}

/** Create new ExecutorResult struct as exb error, receive error code, message string and format. Work as printf
 * function */
ExecutorResult* Executor_error(uint32_t code, char *data, ...) {
	size_t size = strlen(data) + MAX_RET_PAYLOAD + 1;
	char *msg = malloc(size);

	code = (code & 0x00FF00FF) <<  8 | (code & 0xFF00FF00) >>  8;
	code = (code & 0x0000FFFF) << 16 | (code & 0xFFFF0000) >> 16;

	va_list args;
	va_start(args, data);
	vsprintf(msg, data, args);
	va_end(args);

	size_t msgl = strlen(msg);
	char *buf = malloc(msgl + 4);

	memcpy(buf, &code, 4);
	memcpy(buf + 4, msg, msgl);

	ExecutorResult *r = (ExecutorResult*) malloc(sizeof(ExecutorResult));
	r->type = EXB_TYPE_ERR;
	r->size = msgl + 4;
	r->data = buf;

	free(msg);

	return r;
}

//============================================== COMMANDS =================================================

/** Read value from specified adc channel
 *
 *  Args: id - adc phisical channel number (number in range 0 to ADC_COUNT)
 *
 *  Return: x(r) - adc value
 *  		1(e) - not enough arguments
 *          2(e) - id not a number or channel is not defined
 *
 **/
ExecutorResult* Executor_cmd_adc_r(char *id) {
	if (id != NULL) {
		uint8_t result = 0;
		uint8_t id_n = strtol2(id, NULL, 10, 0, ADC_COUNT + 3, &result);
		if (result == 0) {
			int32_t value = ADC_readFilteredVaule(id_n - 3, config->adc_filtration_count, config->adc_filtration_delay);
			return Executor_response("%d", value);
		} else {
			return Executor_error(2, "-");
		}
	} else {
		return Executor_error(1, "-");
	}
}

/** Set the gpio pin to the speceified binary state. It is accept virtual pin number which is maps in the physical values
 *  of the port pin through the function Executor_map_gpio.
 *
 *  Args: pin - virtual pin number
 *        state - 0 or number > 0
 *
 *  Return: 0(r) - gpio state changed
 *  		1(e) - not enough arguments
 *          2(e) - some arg(s) not presented, not a number or pin is not defined
 *
 **/
ExecutorResult* Executor_cmd_gpio_w(char *pin, char *state) {
	if (pin != NULL || state != NULL) {
		GpioMap *gd = Executor_map_gpio(pin);
		if (gd != NULL) {
			if (strcmp(state, "0") == 0)
				HAL_GPIO_WritePin(gd->port, gd->pin, GPIO_PIN_RESET);
			else
				HAL_GPIO_WritePin(gd->port, gd->pin, GPIO_PIN_SET);
			pfree(gd);
			return Executor_response("0");
		} else {
			return Executor_error(2, "-");
		}
	} else {
		return Executor_error(1, "-");
	}
}

/** Returns the current state of the specified gpio pin. It is accept virtual pin number which is maps in the physical values
 *  of the port pin through the function Executor_map_gpio.
 *
 *  Args: pin - virtual pin number (number in system range)
 *
 *  Return: 0/1(r) - gpio state
 *  		1(e) - not enough arguments
 *          2(e) - interations arg not a number or not in range
 *
 * */
ExecutorResult* Executor_cmd_gpio_r(char *pin) {
	if (pin != NULL) {
		GpioMap *gd = Executor_map_gpio(pin);
		if (gd != NULL) {
			char *v = itoa2(HAL_GPIO_ReadPin(gd->port, gd->pin));
			pfree(v);
			pfree(gd);
			return Executor_response("%s", v);
		} else {
			return Executor_error(2, "-");
		}
	} else {
		return Executor_error(1, "-");
	}
}


/** Set exb pin (pin) to some supported mode. If after pin arg is no args specified, it return current gpio mode
 *
 * Args: pin -   virtual pin number (number in system range)
 * 		 mode -  pin mode, may by:
 * 		 		   i - digital input
 * 				   o - digital output
 * 				   r - external interrupt
 * 		 a1-a3 - mode specify arguments. For i mode:
 * 		 		   a1 - pull resistor mode: u=PULLUP, d=PULLDOWN, n=NOPULL
 * 		 		 For o mode:
 * 		 		   a1 - default out value 1 or 0
 * 		 		 For r mode:
 * 		 		   a1 - interrupt front: f=FALLING, r=RISING, a=FALLING+RISING
 * 		 		   a2 - pull resistor mode: u=PULLUP, d=PULLDOWN, n=NOPULL
 * 		 		   a3 - events cutoff timeout (event will not be gereated if new interrupt was raised early then last
 * 		 		        current time < last interrupt time + cutoff time): 0=0ms, 1=1ms, 2=10ms, 3=50ms, 4=100ms,
 * 		 		        5=250ms, 6=500ms, 7=1000ms. This value must be mandatory set to > 1 if to the gpio conntected
 * 		 		        any electomechanical devaice, to prevent duplicate events due to false interrupts caused by
 * 		 		        contact bounce
 * Return: 0/mode(r) - 0 text or current gpio mode
 *         1(e)		 - unkown pin number
 *         3(e)		 - configuration write error
 *         4(e)      - internal configuration is broken
 *         5(e)		 - mode does not supported
 *         6(e)		 - insufficient mode specific args
 *         7(e)      - some mode specific arg have bad value
 *
 *
 *
 */
ExecutorResult* Executor_cmd_gpio_m(char *pin, char *mode, char *a1, char *a2, char *a3) {
	uint8_t pinn = atoi(pin);

	if (pinn > 23)
		return Executor_error(1, "PIN_UNK");

	// Set gpio mode

	GpioMap *gp = Executor_map_gpio(pin);
	if (gp->adc)
		return Executor_error(5, "OP_UNS");

	if (gp != NULL) {
		Executor_disable_adc(gp);
		if (strcmp(mode, "i") == 0) {
			if (a1 == NULL)
				return Executor_error(6, "ARGS_INS");

			GPIO_InitTypeDef GPIO_InitStruct;
			GPIO_InitStruct.Pin = gp->pin;
			GPIO_InitStruct.Mode = GPIO_MODE_INPUT;


			if (strcmp(a1, "u") == 0) {
				GPIO_InitStruct.Pull = GPIO_PULLUP;
				config->gpiom[pinn] = 0b0001000000000000;
			} else if (strcmp(a1, "d") == 0) {
				GPIO_InitStruct.Pull = GPIO_PULLDOWN;
				config->gpiom[pinn] = 0b0001010000000000;
			} else {
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				config->gpiom[pinn] = 0b0001100000000000;
			}

			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			HAL_GPIO_Init(gp->port, &GPIO_InitStruct);

			if (!Config_Write(CONFIG_ADDRESS, config))
				return Executor_error(3, "CFG_ERR");

		} else if (strcmp(mode, "o") == 0) {
			if (a1 == NULL)
				return Executor_error(6, "ARGS_INS");

			GPIO_InitTypeDef GPIO_InitStruct;
			GPIO_InitStruct.Pin = gp->pin;
			GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			HAL_GPIO_Init(gp->port, &GPIO_InitStruct);

			if (strcmp(a1, "0") == 0) {
				HAL_GPIO_WritePin(gp->port, gp->pin, GPIO_PIN_RESET);
				config->gpiom[pinn] = 0;
			} else {
				HAL_GPIO_WritePin(gp->port, gp->pin, GPIO_PIN_SET);
				config->gpiom[pinn] = 0b0000100000000000;
			}

			if (!Config_Write(CONFIG_ADDRESS, config))
				return Executor_error(3, "CFG_ERR");

		} else if (strcmp(mode, "a") == 0) {
			return Executor_error(5, "OP_UNS");
		} else if (strcmp(mode, "r") == 0) {
			if (a1 == NULL || a2 == NULL || a3 == NULL)
				return Executor_error(6, "ARGS_INS");

			GPIO_InitTypeDef GPIO_InitStruct;
			GPIO_InitStruct.Pin = gp->pin;

			if (strcmp(a1, "r") == 0) {
				GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
				config->gpiom[pinn] = 0b0010000000000000;
			} else if (strcmp(a1, "f") == 0) {
				GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
				config->gpiom[pinn] = 0b0010010000000000;
			} else {
				GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
				config->gpiom[pinn] = 0b0010100000000000;
			}

			if (strcmp(a2, "u") == 0) {
				GPIO_InitStruct.Pull = GPIO_PULLUP;
			} else if (strcmp(a2, "d") == 0) {
				GPIO_InitStruct.Pull = GPIO_PULLDOWN;
				config->gpiom[pinn] = config->gpiom[pinn] | 0b0000000100000000;
			} else {
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				config->gpiom[pinn] = config->gpiom[pinn] | 0b0000001000000000;
			}

			uint32_t cutoff = atoi(a3);
			uint16_t thr;
			if (cutoff == 0) {
				thr = 0;
			} else if (cutoff == 1) {
				thr = 1;
				config->gpiom[pinn] = config->gpiom[pinn] | 0b0000000000100000;
			} else if (cutoff == 2) {
				thr = 10;
				config->gpiom[pinn] = config->gpiom[pinn] | 0b0000000001000000;
			} else if (cutoff == 3) {
				thr = 50;
				config->gpiom[pinn] = config->gpiom[pinn] | 0b0000000001100000;
			} else if (cutoff == 4) {
				thr = 100;
				config->gpiom[pinn] = config->gpiom[pinn] | 0b0000000010000000;
			} else if (cutoff == 5) {
				thr = 250;
				config->gpiom[pinn] = config->gpiom[pinn] | 0b0000000010100000;
			} else if (cutoff == 6) {
				thr = 500;
				config->gpiom[pinn] = config->gpiom[pinn] | 0b0000000011000000;
			} else if (cutoff == 7) {
				thr = 1000;
				config->gpiom[pinn] = config->gpiom[pinn] | 0b0000000011100000;
			} else {
				return Executor_error(7, "ARG_BAD");
			}

			//Update interrupt cutoff map
			uint16_t complPin = Executor_pinToGpio(gp->pin);
			interruptCuts[complPin].threshold = thr;
			interruptCuts[pinn].threshold = thr;

			HAL_GPIO_Init(gp->port, &GPIO_InitStruct);

			if (!Config_Write(CONFIG_ADDRESS, config))
				return Executor_error(3, "CFG_ERR");
		} else {
			uint16_t set;
			memcpy(&set, &(config->gpiom[pinn]), 2);
			uint16_t ms = (set & 0b1111000000000000) >> 12;

			if (ms == 0) { // OUT
				char *r = (char*) pmalloc(3);
				uint8_t state = (set & 0b0000100000000000) >> 11;
				r[0] = 'o';
				if (state == 0)
					r[1] = '0';
				else
					r[1] = '1';
				r[2] = 0;
				ExecutorResult *result = Executor_response(r);
				pfree(r);

				return result;
			}  else if (ms == 1) { // IN
				char *r = (char*) pmalloc(3);
				uint16_t x = (set & 0b0000110000000000);
				uint16_t pull = x >> 10;
				r[0] = 'i';
				if (pull == 0)
					r[1] = 'u';
				else if (pull == 1)
					r[1] = 'd';
				else
					r[1] = 'n';

				r[2] = 0;
				ExecutorResult *result = Executor_response(r);
				pfree(r);

				return result;
			}  else if (ms == 2) { // IRQ
				char *r = (char*) pmalloc(5);
				uint16_t mode = (set & 0b0000110000000000) >> 10;
				uint16_t pull = (set & 0b0000001100000000) >> 8;
				uint16_t cutoff = (set & 0b0000000011100000) >> 5;
				r[0] = 'r';

				if (mode == 0)
					r[1] = 'r';
				else if (mode == 1)
					r[1] = 'f';
				else
					r[1] = 'a';

				if (pull == 0)
					r[2] = 'u';
				else if (pull == 1)
					r[2] = 'd';
				else
					r[2] = 'n';

				char *cs = itoa2(cutoff);
				r[3] = cs[0];
				pfree(cs);

				r[4] = 0;
				ExecutorResult *result = Executor_response(r);
				pfree(r);

				return result;
			} else if (ms == 3) { // ADC
				return Executor_error(5, "OP_UNS");
			} else { //UNKNOWN
				return Executor_error(4, "CFG_BROKEN");
			}
		}

		return Executor_response("0");
	} else {
		return Executor_error(1, "NE_PIN");
	}
}

/** Works with ADP filter options. If arguments are provided, it is entered from the value into the configuration. If no
 *  arguments are provided, returns the current filtering values from the configuration.
 *
 *  Args: interations - filter iterations count (number in range 1-65535)
 *        delay - delay betwieen iterations (namber in range 0-65535)
 *
 *  Return: 0(r) - configuration changed
 *          1(e) - interations arg not presented, not a number or is not in range
 *          2(e) - delay arg not presented, not a number or is not in range
 *          3(e) - could not write confiugation to flash
 *
 **/
ExecutorResult* Executor_cmd_adc_filter(char *interations, char *delay) {
	if (interations != NULL && delay != NULL) {
		uint8_t result = 0;
		uint16_t iterations_n = strtol2(interations, NULL, 10, 1, 65535, &result);
		if (result == 0) {
			uint16_t delay_n = strtol2(delay, NULL, 10, 0, 65535, &result);
			if (result == 0) {
				config->adc_filtration_count = (uint16_t) iterations_n;
				config->adc_filtration_delay = (uint16_t) delay_n;
				if (Config_Write(CONFIG_ADDRESS, config))
					return Executor_response("0");
				else
					return Executor_error(3, "-");
			} else {
				return Executor_error(2, "-");
			}
		} else {
			return Executor_error(1, "-");
		}
	} else {
		char *iterations_s = itoa2(config->adc_filtration_count);
		char *delay_s = itoa2(config->adc_filtration_delay);
		pfree(iterations_s);
		pfree(delay_s);
		return Executor_response("%s:%s", iterations_s, delay_s);
	}
}

/** Return current free heap size in bytes
 *
 *  Args: -
 *
 *  Return: x(r) - free heap in bytes
 *
 **/
ExecutorResult* Executor_cmd_heap() {
	return Executor_response("%d", xPortGetFreeHeapSize());
}

/** Return time in ticks past from os sheduler start
 *
 *  Attension: xTaskGetTickCount return uint32_t, with means that max uptime may be ~46 days, after that, counter will be
 *  cleared
 *
 *  Args: -
 *
 *  Return: x(r) - ticks count
 *
 **/
ExecutorResult* Executor_cmd_uptime() {
	return Executor_response("%d", xTaskGetTickCount());
}


ExecutorResult* Executor_cmd_pipe_m(char *self, char *mast) {
	if (self == NULL || mast == NULL) {
		return Executor_response("0x%04x:0x%04x", a_self, a_master);
	}
	uint8_t result = 0;
	uint32_t self_n = strtol2(self, NULL, 16, 0, (long) 4294967295UL, &result);

	uint8_t result2 = 0;
	uint32_t mast_n = strtol2(mast, NULL, 16, 0, 4294967295UL, &result2);

	if (result2 == 0 && result2 == 0) {
		a_self = self_n;
		a_master = mast_n;
		config->a_self = a_self;
		config->a_master = a_master;

		if (Config_Write(CONFIG_ADDRESS, config))
			return Executor_response("0");
		else
			return Executor_error(1, "-");

		return Executor_response("%d", 0);
	} else {
		return Executor_error(2, "-");
	}
}

/** Pong payload data to back as is
 *
 *  Args: pld - payload data
 *
 *  Return: input payload
 *
 */
ExecutorResult* Executor_cmd_ping(char *pld) {
	return Executor_response("%s", pld);
}

ExecutorResult* Executor_cmd_event_r(char *code, char *type, char *confirmed, char *arg1, char *arg2) {
	uint32_t iCode = atoi(code);
	uint32_t iType = atoi(type);
	bool iConfirmed = false;
	if (strcmp(confirmed, "c") == 0)
		iConfirmed = true;

	uint32_t iArg1 = atoi(arg1);
	uint32_t iArg2 = atoi(arg2);
	uint8_t r = EventManager_register(iType, iConfirmed, iCode, iArg1, iArg2);
	if (r == 0) {
		EventsProducer_addEvent(iCode);
		return Executor_response("0");
	} else {
		if (r == 1)
			return Executor_error(1, "CFG_ERR");
		else if (r == 2)
			return Executor_error(2, "OP_UNS");
		else if (r == 3)
			return Executor_error(3, "CFG_ERR");
		else if (r == 4)
			return Executor_error(4, "CFG_ERR");
		else
			return Executor_error(5, "EXIST");
	}
}

ExecutorResult* Executor_cmd_event_u(char *code) {
	uint32_t iCode = atoi(code);
	uint8_t r = EventManager_unregister(iCode);
	if (r == 0) {
		EventsProducer_removeEvent(iCode);
		return Executor_response("0");
	} else {
		if (r == 1)
			return Executor_error(1, "NOT_EXIST");
		else
			return Executor_error(4, "CFG_ERR");
	}
}

ExecutorResult* Executor_cmd_event_l() {
	uint8_t size = 0;
	uint32_t *list = EventManager_list(&size);

	if (size == 0)
		return Executor_response("-");

	char *str = (char*) pmalloc(50 * size);


	for (uint8_t i = 0; i < (50 * size); i++) {
		str[i] = ' ';
	}

	str[50 * size - 1] = 0;

	uint16_t rsize = 0;
	for(uint8_t i = 0; i < size; i++) {
		uint32_t code = list[i];
		char *codeStr = itoa2(code);
		uint8_t codeStrSize = strlen(codeStr);

		bool confirmed = EventManager_isConfirnmed(code);
		char *confirmedStr = (char*) pmalloc(2);
		confirmedStr[0] = 'n';
		if (confirmed)
			confirmedStr[0] = 'c';
		confirmedStr[1] = 0;
		uint8_t confirmedStrSize = 2;

		uint8_t type = EventManager_type(code);
		char *typeStr = itoa2(type);
		uint8_t typeStrSize = strlen(typeStr);

		uint32_t arg0;
		EventManager_getArg(code, 0, &arg0);
		char *arg0Str = itoa2(arg0);
		uint8_t arg0StrSize = strlen(arg0Str);

		uint32_t arg1;
		EventManager_getArg(code, 1, &arg1);
		char *arg1Str = itoa2(arg1);
		uint8_t arg1StrSize = strlen(arg1Str);

		uint32_t arg2;
		EventManager_getArg(code, 2, &arg2);
		char *arg2Str = itoa2(arg2);
		uint8_t arg2StrSize = strlen(arg2Str);

		uint8_t off0 = rsize + codeStrSize + 1;
		uint8_t off1 = off0 + confirmedStrSize;
		uint8_t off2 = off1 + typeStrSize + 1;
		uint8_t off3 = off2 + arg0StrSize + 1;
		uint8_t off4 = off3 + arg1StrSize + 1;
		uint8_t offt = off4 + arg2StrSize + 1;

		memcpy(str + rsize, codeStr, codeStrSize);
		str[codeStrSize] = ':';

		memcpy(str + off0, confirmedStr, confirmedStrSize);
		str[off0 + 1] = ':';


		memcpy(str + off1, typeStr, typeStrSize);
		str[off1 + 1] = ':';

		memcpy(str + off2, arg0Str, arg0StrSize);
		str[off2 + arg0StrSize] = ':';

		memcpy(str + off3, arg1Str, arg1StrSize);
		str[off3 + arg1StrSize] = ':';

		memcpy(str + off4,  arg2Str, arg2StrSize);
		str[off4 + arg2StrSize] = '\n';

		pfree(codeStr);
		pfree(confirmedStr);
		pfree(typeStr);
		pfree(arg0Str);
		pfree(arg1Str);
		pfree(arg2Str);

		rsize = offt;
	}


	return Executor_response(str);

		//"2000000000:255:c:2000000000:2000000000:2000000000\n"
}
