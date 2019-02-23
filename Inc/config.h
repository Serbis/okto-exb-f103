/*
 * runtime_config.h
 *
 *  Created on: 24 окт. 2018 г.
 *      Author: serbis
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>
#include <stdint.h>

#define MAX_EVENTS 25
#define MAX_EVENT_ARGS 50

/** Default config addrees. Use in address variable of the config functions */
#define CONFIG_ADDRESS 1

/** Config struct magic. See the Config_Load function */
#define CONFIG_MAGIC 12447744073739555619UL

typedef struct Config {
	/** This number is intended to detect the existence of a config in the memory area. See the Config_Load function */
	uint64_t magic;

	/* Count of ADC iterations. A value of 1 indicates direct channel reads without filtering */
	uint16_t adc_filtration_count;

	/** The time delay in ticks between iterations of adc filtering. A value of 0 indicates continuous mode */
	uint16_t adc_filtration_delay;

	/** Self net address */
	uint32_t a_self;

	/** Master net address */
	uint32_t a_master;

	/** Gpio configuration. This is a bit mask array, where each elemnt represend configration of one gpio. Elements
	 * position in the array direct corresponds to the exb gpio numbering system. Element has next bit mask:
	 *
	 * | type |             args            |
	 * | 0-3  |             4-15            |
	 *
	 * -----------------------------------------------------
	 * Type 0 - gpio out
	 *
	 * | 0000 | a |        not used         |
	 * | 0-3  | 4 |          5-15           |
	 *
	 * a - initial out state 0 or 1
	 *
	 * -----------------------------------------------------
	 * Type 1 - gpio in
	 *
	 * | 0001 |  a  |        not used       |
	 * | 0-3  | 4-5 |          6-15         |
	 *
	 * a - pull resitor mode: 0=up, 1=down, 3=nopull
	 *
	 * -----------------------------------------------------
	 * Type 2 - gpio irq
	 *
	 * | 0010 |  a  |  b  |  c  |  not used    |
	 * | 0-3  | 4-5 | 6-7 | 8-10|   11-15      |
	 *
	 * a - interrupt front: 0=rising, 1=falling, 2=all
	 * b - pull resitor mode: 0=up, 1=down, 2=nopull
	 * c - interrupt cutoff time: 0=0ms, 1=1ms, 2=10ms, 3=50ms, 4=100ms, 5=250ms, 6=500ms, 7=1000ms #1
	 *
	 * #1 - This mechanism is designed to control contact bounce when processing EXTI in case of events.
	 *  	Its essence boils down to the fact that after triggering an interrupt by pin and forming an
	 *  	event, some time must pass before a repeated interruption can create a new event
	 * */
	uint16_t gpiom[23];

	/** Events configuration. This is a bit mask array, where each elemnt respresend an one event configuration.
	 * Element shas nex bit mask:
	 *
	 * | e | c |       type      | ... code ... | ... a2 ... | ... a1 ... | ... a0 ... |
	 * | 0 | 1 |       2-7       |     8-39     |   40-47    |   48-55    |   55-64    |
	 *
	 * e - cell enabled (used) flag: 1=used, 2=not used
	 * c - confirmed event flag : 1 = confirmed, 2 = not confirmed
	 * type - event type: 0: single exti, 1: counting exti, 2: adc threshold, 3: test type
	 * code - event code
	 * a1-a3 - event args, containt index in eventArgs array
	 *
	 * Events args mask by types:
	 * type=0 -> args not used
	 * type=1 -> a0=counter threshold
	 * type=3 -> a0=bit mask 1
	 * type=5 -> a0=test arg, a1=test arg, a2=test arg,
	 *
	 **/
	uint64_t events[MAX_EVENTS];

	/** Events arguments array. Contain values used as argunets in events. This fields is a bit mask:
	 *
	 * | e |   ... value ...   |
	 * | 0 |       1-31        |
	 *
	 * e - cell enabled (used)
	 * value - 31 bit data field
	 *
	 */
	uint32_t eventArgs[MAX_EVENT_ARGS];
} Config;

Config* Config_Load(uint16_t address);
bool Config_Write(uint16_t address, Config *config);
void Config_Reset(uint16_t address);

#endif /* CONFIG_H_ */
