#include <stdarg.h>
#include <stdlibo.h>
#include "event_manager.h"
#include "config.h"

extern Config *config;

/** Set arg value to the first free event arg. Return > 0 if success or 0 if not free cells in
 *  events arg array */
uint8_t _EventManager_putArg(uint32_t arg) {
	// Find free arg cell
	int16_t cell = 0;
	for (uint16_t i = 1; i < MAX_EVENT_ARGS; i++) {
		if ((config->eventArgs[i] & 0x80000000) == 0) {
			cell = i;
			break;
		}
	}

	if (cell > 0) {
		arg |= 0x80000000; // reserved
		config->eventArgs[cell] = arg;
		return cell;
	}

	return 0;
}

/** Write argument value with number (arg) from event (code) to the buffer (buf). Return 0 if succes or
 *  positive if events with passed code does not registered */
uint8_t EventManager_getArg(uint32_t code, uint8_t arg, uint32_t *buf) {
	// Find target cell
	int16_t cell = -1;
	for (uint8_t i = 0; i < MAX_EVENTS; i++) {
		uint64_t v = config->events[i];
		if (v << 24 >> 32 == code) {
			cell = i;
			break;
		}
	}

	if (cell == -1)
		return 1;

	uint64_t event = config->events[cell];
	uint64_t argn = event << (8 * arg) >> 56;
	if (argn > 0) {
		uint32_t ar = config->eventArgs[event << (8 * arg) >> 56];
		*buf = ar & 0x7FFFFFFF; //remove reserved flag
	} else {
		*buf = 0;
	}

	return 0;
}

/** Register event in the configuration. Return 0 if success or:
 * 		1 - no free event cell
 * 		2 - type is not supported
 * 		3 - no free event arg cell
 * 		4 - error while saving config
 * 		5 - event with speicified code already registered
 */
uint8_t EventManager_register(uint8_t type, bool conf, uint32_t code, uint32_t args, ...) {
	// Find exist code
	int16_t exist = -1;
	for (uint8_t i = 0; i < MAX_EVENTS; i++) {
		uint64_t v = config->events[i];
		if (v << 24 >> 32 == code) {
			exist = i;
			break;
		}
	}

	if (exist >= 0)
		return 5;

	// Find free cell
	int16_t cell = -1;
	for (uint8_t i = 0; i < MAX_EVENTS; i++) {
		if ((config->events[i] & 0x0000000000000080) == 0) {
			cell = i;
			break;
		}
	}

	if (cell < 0)
		return 1; //No free event cell

	// Create  bit blob
	uint8_t blob[8] = { 0 };
	blob[0] |= 0x80; // enabled
	if (conf)
		blob[0] |= 0x40; // confirmed
	memcpy(&blob[1], &code, 4); // code



	if (type == 0) {
		uint8_t pin_el = _EventManager_putArg(args);
		if (pin_el == 0)
			return 3; //No free event args
		memcpy(&blob[7], &pin_el, 1);
	} else if (type == 1) {
		blob[0] |= 0x01; // type
		va_list argptr;
		va_start (argptr, args);
		uint32_t a2 = va_arg(argptr, uint32_t);
		uint8_t pin_el = _EventManager_putArg(args);
		if (pin_el == 0)
			return 3; //No free event args
		uint8_t th_el = _EventManager_putArg(a2);
		if (th_el == 0)
			return 3; //No free event args
		memcpy(&blob[7], &pin_el, 1);
		memcpy(&blob[6], &th_el, 1);
	} else if (type == 2) {
		return 0;
	} else if (type == 3) { //Test type with 3 args
		blob[0] |= 0x03; // type
		va_list argptr;
		va_start (argptr, args);
		uint32_t a0 = args;
		uint32_t a1 = va_arg(argptr, uint32_t);
		uint32_t a2 = va_arg(argptr, uint32_t);
		va_end(argptr);
		uint16_t la0 = _EventManager_putArg(a0);
		uint16_t la1 = _EventManager_putArg(a1);
		uint16_t la2 = _EventManager_putArg(a2);
		if (la0 == 0)
			return 3; //No free event args
		if (la1 == 0) {
			EventManager_unregister(la0);
			return 3; //No free event args
		}
		if (la2 == 0) {
			EventManager_unregister(la0);
			EventManager_unregister(la1);
			return 3; //No free event args
		}

		memcpy(&blob[7], &la0, 1);
		memcpy(&blob[6], &la1, 1);
		memcpy(&blob[5], &la2, 1);
	} else {
		return 2; //Not supported type
	}

	// Write blob to the cell
	uint64_t _blob;
	memcpy(&_blob, &blob, 8);
	config->events[cell] = (uint64_t) _blob;

	// Write config
	if (!Config_Write(CONFIG_ADDRESS, config))
		return 4;

	return 0;
}

/** Unregister event with specified code. Reutrn 0 if event unregisterd or if event is not be
 * registered before or 2 if saving config goes wrong */
uint8_t EventManager_unregister(uint32_t code) {
	// Find target cell
	int16_t cell = -1;
	for (uint8_t i = 0; i < MAX_EVENTS; i++) {
		uint64_t v = config->events[i];
		if (v << 24 >> 32 == code) {
			cell = i;
			break;
		}
	}

	if (cell >= 0) {
		uint64_t event = config->events[cell];
		config->eventArgs[event << 16 >> 56] = 0;
		config->eventArgs[event << 8 >> 56] = 0;
		config->eventArgs[event >> 56] = 0;
		config->events[cell] = 0;

		// Write config
		if (!Config_Write(CONFIG_ADDRESS, config))
			return 4;

		return 0;
	} else {
		return 1;
	}
}

/** Return cell number of event or -1. This function used for performing event exist checking */
int32_t EventManager_cell(uint32_t code) {
	// Find target cell
	int16_t cell = -1;
	for (uint8_t i = 0; i < MAX_EVENTS; i++) {
		uint64_t v = config->events[i];
		if (v << 24 >> 32 == code) {
			cell = i;
			break;
		}
	}

	return cell;
}

/** Return type of an event with specified code */
int8_t EventManager_type(uint32_t code) {
	// Find target cell
	int16_t cell = -1;
	for (uint8_t i = 0; i < MAX_EVENTS; i++) {
		uint64_t v = config->events[i];
		if (v << 24 >> 32 == code) {
			cell = i;
			break;
		}
	}

	if (cell == -1)
		return -1;
	uint8_t fb = *((uint8_t*) &(config->events[cell]));
	uint8_t type = (fb << 2);
	type = type >> 2;

	return type;
}

/** Return array with all registered event codes */
uint32_t* EventManager_list(uint8_t *size) {
	uint32_t *cds = (uint32_t*) malloc(MAX_EVENTS * 4);

	uint8_t rcounter = 0;

	for (uint8_t i = 0; i < MAX_EVENTS; i++) {
		uint64_t v = config->events[i];

		if (v != 0) {
			uint64_t code = v << 24 >> 32;

			cds[rcounter] = code;
			rcounter++;
		}
	}

	if (rcounter > 0) {
		uint32_t *cdsr = (uint32_t*) malloc(rcounter * 4);
		memcpy(cdsr, cds, rcounter * 4);
		free(cds);
		*size = rcounter;
		return cdsr;
	}

	*size = 0;

	return NULL;
}

/** Check if an event is confirmed or not. If event deos not registered return false */
bool EventManager_isConfirnmed(uint32_t code) {
	// Find target cell
	int16_t cell = -1;
	for (uint8_t i = 0; i < MAX_EVENTS; i++) {
		uint64_t v = config->events[i];
		if (v << 24 >> 32 == code) {
			cell = i;
			break;
		}
	}

	if (cell == -1)
		return false;
	uint8_t fb = *((uint8_t*) &(config->events[cell]));
	uint8_t conf = fb & 0x40;

	return conf;
}
