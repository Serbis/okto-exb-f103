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
} Config;

Config* Config_Load(uint16_t address);
bool Config_Write(uint16_t address, Config *config);
void Config_Reset(uint16_t address);

#endif /* CONFIG_H_ */
