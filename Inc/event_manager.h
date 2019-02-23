#ifndef EVENT_MANAGER_H_
#define EVENT_MANAGER_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t type;
} EventCtlPrimitive;

typedef struct {
	uint8_t type;
	uint32_t code;
} EventCtlType0;

typedef struct {
	uint8_t type;
	uint32_t code;
	uint32_t counter;
} EventCtlType1;

uint8_t EventManager_register(uint8_t type, bool conf, uint32_t code, uint32_t args, ...);
uint8_t EventManager_unregister(uint32_t code);
int32_t EventManager_cell(uint32_t code);
int8_t EventManager_type(uint32_t code);
bool EventManager_isConfirnmed(uint32_t code);
uint8_t EventManager_getArg(uint32_t code, uint8_t arg, uint32_t *buf);
uint32_t* EventManager_list(uint8_t *size);

#endif
