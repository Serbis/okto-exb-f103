/** This file contains exb command handlers. Any command handler is a function that takes as
 *  input arguments as strings and returns an ExecutorResult structure that describes the result
 *  of terminating the handler */

#include <stdarg.h>
#include "stm32f1xx_hal.h"
#include "adc.h"
#include "executor.h"
#include "config.h"
#include "stdlibo.h"
#include "exb_packet.h"
#include "NRF24.h"
#include "../libs/oscl/include/data.h"

extern Config *config;

//=============================================== UTILITES ================================================

/** Struct for map_gpio function. Read it`s desciption fot details */
struct GpioMap {
	/** Phisical gpio port */
	GPIO_TypeDef *port;
	/** Phisical pin number on the gpio port */
	uint16_t pin;
};

/** Map the gpio virtual identifier to the physical value of the port and pin number. Return null if pin indenitifer does
 *  not exist */
struct GpioMap* Executor_map_gpio(char *pin) {
	struct GpioMap *map = pmalloc(sizeof(struct GpioMap));
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
	} else if (strcmp(pin, "4") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_1;
	} else if (strcmp(pin, "5") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_2;
	} else if (strcmp(pin, "6") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_3;
	} else if (strcmp(pin, "7") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_4;
	} else if (strcmp(pin, "8") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_5;
	} else if (strcmp(pin, "9") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_6;
	} else if (strcmp(pin, "10") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_7;
	} else if (strcmp(pin, "11") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_0;
	} else if (strcmp(pin, "12") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_1;
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
		struct GpioMap *gd = Executor_map_gpio(pin);
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
		struct GpioMap *gd = Executor_map_gpio(pin);
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
