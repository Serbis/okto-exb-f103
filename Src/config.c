#include "../Inc/config.h"
#include "../Inc/eeprom.h"
#include "cmsis_os.h"

/* Reads the configuration structure from flash memory at a given offset from the address of the configuration page
 * (set in eeprom_config.h). To determine the presence of the actual configuration, uses a magic number. If the correct
 * magic number appears in the read structure, it returns unchanged. If the last statement is incorrect, the structure
 * is filled with standard values and returned.*/
Config* Config_Load(uint16_t address) {
	Config *cfg = pmalloc(sizeof(Config));

	taskENTER_CRITICAL();
	bool r = EE_Reads(address, sizeof(Config) / 4, cfg);
	taskEXIT_CRITICAL();

	if (cfg->magic != CONFIG_MAGIC || !r) {
		cfg->magic = CONFIG_MAGIC;
		cfg->adc_filtration_count = 2048;
		cfg->adc_filtration_delay = 0;
		cfg->a_self = 0x11111101;
		cfg->a_master = 0x111111A1;
		for (uint8_t i = 0; i < 23; i++) { //def = _o0
			cfg->gpiom[i] = 0;
		}

		for (uint8_t i = 0; i < MAX_EVENTS; i++) {
			cfg->events[i] = 0;
		}

		for (uint8_t i = 0; i < MAX_EVENT_ARGS; i++) {
			cfg->eventArgs[i] = 0;
		}
	}

	return cfg;
}

/** Writes the configuration structure to flash memory at the specified offset from the address of the configuration page
 * (set in eeprom_config.h) */
bool Config_Write(uint16_t address, Config *config) {
	taskENTER_CRITICAL();
	bool r = EE_Writes(address, sizeof(Config) / 4, config);
	taskEXIT_CRITICAL();

	return r;
}

/** Weret configuration be erease magic */
void Config_Reset(uint16_t address) {
	uint64_t nol = 0UL;
	taskENTER_CRITICAL();
	EE_Writes(address, 2, &nol);
	taskEXIT_CRITICAL();
}
