#include <config.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "stm32f1xx_hal.h"
#include "../Inc/cmd_processor.h"
#include "../Inc/adc.h"
#include "../libs/collections/include/list.h"
#include "../libs/oscl/include/data.h"
#include "../libs/oscl/include/harware.h"


extern LinkedBlockingQueue *cmdQueue;
extern Config *config;
bool CmdProcessor_threadAlive;

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
struct GpioMap* CmdProcessor_map_gpio(char *pin) {
	struct GpioMap *map = pmalloc(sizeof(struct GpioMap));
	if (strcmp(pin, "0") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_2;
	} else if (strcmp(pin, "1") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_10;
	} else if (strcmp(pin, "2") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_11;
	} else if (strcmp(pin, "3") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_12;
	} else if (strcmp(pin, "4") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_13;
	} else if (strcmp(pin, "5") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_14;
	} else if (strcmp(pin, "6") == 0) {
		map->port = GPIOB;
		map->pin = GPIO_PIN_15;
	} else if (strcmp(pin, "7") == 0) {
		map->port = GPIOA;
		map->pin = GPIO_PIN_8;
	} else {
		pfree(map);
		return NULL;
	}

	return map;
}

/** Create new OTPP packet */
char* CmdProcessor_createResponse(const char *type, uint32_t msgId, const char *content) {
    char *msgIdStr = itoa2(msgId);
    size_t size = strlen(type) + 2 + strlen(msgIdStr) + 2 + strlen(content);
    char *buf = pmalloc(size);
    sprintf(buf, "%s\t%s\t%s\r", type, msgIdStr, content);

    pfree(msgIdStr);

    return buf;
}

/** Create OTPP packet and send it to uart */
void CmdProcessor_sendResponse(const char *type, uint32_t msgId, const char *content, ...) {
	size_t len = strlen(content);
	size_t lineSize = len + 100;

	char *msg = (char*) pmalloc(lineSize);
	msg[lineSize - 1] = 0;

	va_list args;
	va_start(args, content);
	vsprintf(msg, content, args);
	va_end(args);

	char *msgIdStr = itoa2(msgId);
	size_t size = strlen(type) + 2 + strlen(msgIdStr) + 2 + strlen(msg);
	char *buf = pmalloc(size);
	sprintf(buf, "%s\t%s\t%s\r", type, msgIdStr, msg);

	pfree(msgIdStr);

	Hardware_writeToUart(buf);
	pfree(buf);
	pfree(msg);
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
void CmdProcessor_cmd_adc_r(uint32_t packetId, char *id) {
	if (id != NULL) {
		uint8_t result = 0;
		uint8_t id_n = strtol2(id, NULL, 10, 0, ADC_COUNT, &result);
		if (result == 0) {
			int32_t value = ADC_readFilteredVaule(id_n, config->adc_filtration_count, config->adc_filtration_delay);
			CmdProcessor_sendResponse("r", packetId, "%d", value);
		} else {
			CmdProcessor_sendResponse("e", packetId, "2");
		}
	} else {
		CmdProcessor_sendResponse("e", packetId, "1");
	}
}

/** Set the gpio pin to the speceified binary state. It is accept virtual pin number which is maps in the physical values
 *  of the port pin through the function CmdProcessor_map_gpio.
 *
 *  Args: pin - virtual pin number
 *        state - 0 or number > 0
 *
 *  Return: 0(r) - gpio state changed
 *  		1(e) - not enough arguments
 *          2(e) - some arg(s) not presented, not a number or pin is not defined
 *
 **/
void CmdProcessor_cmd_gpio_w(uint32_t packetId, char *pin, char *state) {
	if (pin != NULL || state != NULL) {
		struct GpioMap *gd = CmdProcessor_map_gpio(pin);
		if (gd != NULL) {
			if (strcmp(state, "0") == 0)
				HAL_GPIO_WritePin(gd->port, gd->pin, GPIO_PIN_RESET);
			else
				HAL_GPIO_WritePin(gd->port, gd->pin, GPIO_PIN_SET);
			CmdProcessor_sendResponse("r", packetId, "0");
			pfree(gd);
		} else {
			CmdProcessor_sendResponse("e", packetId, "2");
		}
	} else {
		CmdProcessor_sendResponse("e", packetId, "1");
	}
}

/** Returns the current state of the specified gpio pin. It is accept virtual pin number which is maps in the physical values
 *  of the port pin through the function CmdProcessor_map_gpio.
 *
 *  Args: pin - virtual pin number (number in system range)
 *
 *  Return: 0/1(r) - gpio state
 *  		1(e) - not enough arguments
 *          2(e) - interations arg not a number or not in range
 *
 * */
void CmdProcessor_cmd_gpio_r(uint32_t packetId, char *pin) {
	if (pin != NULL) {
		struct GpioMap *gd = CmdProcessor_map_gpio(pin);
		if (gd != NULL) {
			char *v = itoa2(HAL_GPIO_ReadPin(gd->port, gd->pin));
			CmdProcessor_sendResponse("r", packetId, "%s", v);
			pfree(v);
			pfree(gd);
		} else {
			CmdProcessor_sendResponse("e", packetId, "2");
		}
	} else {
		CmdProcessor_sendResponse("e", packetId, "1");
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
void CmdProcessor_cmd_adc_filter(uint32_t packetId, char *interations, char *delay) {
	if (interations != NULL && delay != NULL) {
		uint8_t result = 0;
		uint16_t iterations_n = strtol2(interations, NULL, 10, 1, 65535, &result);
		if (result == NULL) {
			uint16_t delay_n = strtol2(delay, NULL, 10, 0, 65535, &result);
			if (result == NULL) {
				config->adc_filtration_count = (uint16_t) iterations_n;
				config->adc_filtration_delay = (uint16_t) delay_n;
				if (Config_Write(CONFIG_ADDRESS, config))
					CmdProcessor_sendResponse("r", packetId, "0");
				else
					CmdProcessor_sendResponse("e", packetId, "3");
			} else {
				CmdProcessor_sendResponse("e", packetId, "2");
			}
		} else {
			CmdProcessor_sendResponse("e", packetId, "1");
		}
	} else {
		char *iterations_s = itoa2(config->adc_filtration_count);
		char *delay_s = itoa2(config->adc_filtration_delay);
		CmdProcessor_sendResponse("r", packetId, "%s:%s", iterations_s, delay_s);
		pfree(iterations_s);
		pfree(delay_s);
	}
}

/** Return current free heap size in bytes
 *
 *  Args: -
 *
 *  Return: x(r) - free heap in bytes
 *
 **/
void CmdProcessor_cmd_heap(uint32_t packetId) {
	CmdProcessor_sendResponse("r", packetId, "%d", xPortGetFreeHeapSize());
}


//=========================================== THREAD FUNCTION =============================================

void CmdProcessor_thread(void *args) {
    while (CmdProcessor_threadAlive) {
        if (cmdQueue->size(cmdQueue) > 0) {
            char *cmd = cmdQueue->dequeue(cmdQueue);
            char *spl = strtok(cmd, "\t");
            List *fields = new_List();

            while (spl != NULL)  {
                fields->prepend(fields, strcpy2(spl));
                spl = strtok (NULL, "\t");
            }

            //----------------------------------------------------------------------------

            if (fields->size < 3) {
            	pfree(cmd);
            	pfree(spl);
            	ListIterator *iterator = fields->iterator(fields);
            	while(iterator->hasNext(iterator)) {
            		void *v = iterator->next(iterator);
            	    iterator->remove(iterator);
            	   	pfree(v);
            	}
            	pfree(iterator);
            	pfree(fields);
                continue;
            }

            char *packetType = fields->get(fields, 2); // Тип пакета
            uint32_t packetId = (uint32_t) strtol(fields->get(fields, 1), NULL, 10); // Идентификатор пакета
            List *packetElements = new_List(); // Элементы

            if (packetId == 0) {
            	pfree(cmd);
            	pfree(spl);
            	ListIterator *iterator = fields->iterator(fields);
            	while(iterator->hasNext(iterator)) {
            		void *v = iterator->next(iterator);
            		iterator->remove(iterator);
            		pfree(v);
            	}
            	pfree(iterator);
            	pfree(fields);

            	ListIterator *iterator2 = packetElements->iterator(packetElements);
            	while(iterator2->hasNext(iterator2)) {
            		void *v = iterator2->next(iterator2);
            	   	iterator2->remove(iterator2);
            	 	pfree(v);
            	}
            	pfree(iterator2);
            	pfree(packetElements);
                continue;
            }

            char *inCmdSpl = strtok(fields->get(fields, 0), " ");
            while (inCmdSpl != NULL)  {
                if (strchr(inCmdSpl, '\r') != NULL) {
                	size_t strLen = strlen(inCmdSpl);
                    char *nInCmdSpl = pmalloc(strLen);
                    nInCmdSpl[strLen - 1] = 0;

                    strncpy(nInCmdSpl, inCmdSpl, strLen - 1);
                    if (strlen(inCmdSpl) != 0)
                        packetElements->prepend(packetElements, strcpy2(nInCmdSpl));
                    pfree(nInCmdSpl);

                } else {
                    packetElements->prepend(packetElements, strcpy2(inCmdSpl));
                }
                inCmdSpl = strtok (NULL, " ");
            }

            if (packetElements->size == 0) {
            	pfree(cmd);
            	pfree(spl);
            	ListIterator *iterator = fields->iterator(fields);
            	while(iterator->hasNext(iterator)) {
            		void *v = iterator->next(iterator);
            		iterator->remove(iterator);
            		pfree(v);
            	}
            	pfree(iterator);
            	pfree(fields);

            	ListIterator *iterator2 = packetElements->iterator(packetElements);
            	while(iterator2->hasNext(iterator2)) {
            		void *v = iterator2->next(iterator2);
            	 	iterator2->remove(iterator2);
            	  	pfree(v);
            	}
            	pfree(iterator2);
            	pfree(packetElements);
                continue;
            }

            //----------------------------------------------

            uint16_t cmdSize = packetElements->size;

            char *cmdName = packetElements->get(packetElements, (uint16_t) (cmdSize - 1));

            if (strcmp(cmdName, "adc_r") == 0) {
            	char *arg1 = packetElements->get(packetElements, (uint16_t) (cmdSize - 2));
            	CmdProcessor_cmd_adc_r(packetId, arg1);
            } else if (strcmp(cmdName, "gpio_w") == 0) {
            	char *arg1 = packetElements->get(packetElements, (uint16_t) (cmdSize - 2));
            	char *arg2 = packetElements->get(packetElements, (uint16_t) (cmdSize - 3));
            	CmdProcessor_cmd_gpio_w(packetId, arg1, arg2);
            } else if (strcmp(cmdName, "gpio_r") == 0) {
            	char *arg1 = packetElements->get(packetElements, (uint16_t) (cmdSize - 2));
            	CmdProcessor_cmd_gpio_r(packetId, arg1);
            } else if (strcmp(cmdName, "adc_filter") == 0) {
            	char *arg1 = packetElements->get(packetElements, (uint16_t) (cmdSize - 2));
            	char *arg2 = packetElements->get(packetElements, (uint16_t) (cmdSize - 3));
            	CmdProcessor_cmd_adc_filter(packetId, arg1, arg2);
            } else if (strcmp(cmdName, "heap") == 0) {
            	CmdProcessor_cmd_heap(packetId);
            } else if (strcmp(cmdName, "gpio_m") == 0) {

            } else {
                char *resp = CmdProcessor_createResponse("e", packetId, "Unknown command");
                Hardware_writeToUart(resp);
                pfree(resp);
            }

            pfree(cmd);
            if (cmd != NULL)
            	pfree(spl);
            if (inCmdSpl != NULL)
            	pfree(inCmdSpl);
            ListIterator *iterator = fields->iterator(fields);
            while(iterator->hasNext(iterator)) {
            	void *v = iterator->next(iterator);
            	iterator->remove(iterator);
            	pfree(v);
            }
            pfree(iterator);
            pfree(fields);

            ListIterator *iterator2 = packetElements->iterator(packetElements);
            while(iterator2->hasNext(iterator2)) {
            	void *v = iterator2->next(iterator2);
            	iterator2->remove(iterator2);
            	pfree(v);
            }
            pfree(iterator2);
            pfree(packetElements);

        }
    }
}
