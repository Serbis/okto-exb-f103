#include <event_producer.h>
#include <stddef.h>
#include <string.h>
#include "../libs/oscl/include/data.h"
#include "stdlibo.h"
#include "event_cotroller.h"
#include "event_manager.h"
#include "stdlibo.h"

QueueHandle_t eventStream = NULL;

typedef struct {
	uint32_t code;
} BaseEventCtl;

typedef struct {
	uint32_t code;
	uint32_t counterl;
	uint32_t countern;
} Type1EventCtl;

// Array if pointers to a BaseEventCtl's struct
BaseEventCtl **regEvents;

uint8_t regEventsSize;

void EventsProducer_thread(EventsProducerThreadArgs *args) {
	eventStream = xQueueCreate(25, sizeof(EventDef));


	// init persisted events
	uint8_t size = 0;
	uint32_t *list = EventManager_list(&size);

	regEvents = (BaseEventCtl**) malloc(size * sizeof(ssize_t));

	for (uint8_t i = 0; i < size; i++) {
		uint32_t code = list[i];
		uint32_t type = EventManager_type(code);

		if (type == 0) {
			BaseEventCtl *ctl = (BaseEventCtl*) malloc(sizeof(BaseEventCtl));
			ctl->code = code;
			regEvents[i] = ctl;
		} else if (type == 1) {
			Type1EventCtl *ctl = (Type1EventCtl*) malloc(sizeof(Type1EventCtl));
			ctl->code = code;
			ctl->countern = 0;
			ctl->counterl = 0;
			regEvents[i] = (BaseEventCtl*) ctl;
		}
	}

	free(list);
	regEventsSize = size;

	while(1) {
		EventDef event;
		if(xQueueReceive(eventStream, &event, 1000) == pdPASS) {
			PendingEvent *pEvent = NULL;

			if (event.type == 0) { //EXTI
				for (uint8_t i = 0; i < regEventsSize; i++) {
					uint32_t code = regEvents[i]->code;
					uint8_t type = EventManager_type(code);
					if (type == 0) {
						uint32_t pin = 0;
						EventManager_getArg(code, 0, &pin);
						if (pin == event.data) {
							pEvent = (PendingEvent*) malloc(sizeof(PendingEvent));
							pEvent->code = code;
							pEvent->confirmed = EventManager_isConfirnmed(code);
							pEvent->size = 0;
							pEvent->data = NULL;
						}
					} else if (type == 1) {
						uint32_t pin = 0;
						EventManager_getArg(code, 0, &pin);
						uint32_t thr = 0;
						EventManager_getArg(code, 1, &thr);
						uint32_t countern =  ((Type1EventCtl*) regEvents[i])->countern;
						uint32_t counterl =  ((Type1EventCtl*) regEvents[i])->counterl;

						if (pin == event.data) {
							if (countern - counterl >= thr - 1) {
								char *counterStr = itoa2(countern + 1);
								pEvent = (PendingEvent*) malloc(sizeof(PendingEvent));
								pEvent->code = code;
								pEvent->confirmed = EventManager_isConfirnmed(code);
								pEvent->size = strlen(counterStr);
								pEvent->data = (uint8_t*) counterStr;


								((Type1EventCtl*) regEvents[i])->counterl = countern + 1;
								((Type1EventCtl*) regEvents[i])->countern = countern + 1;
							} else {
								((Type1EventCtl*) regEvents[i])->countern++;
							}
						}

					}
				}
			}

			if (pEvent != NULL)
				xQueueSend(args->pendingEventStream, &pEvent, portMAX_DELAY);

		}

		vTaskDelay(1);
	}
}

void EventsProducer_removeEvent(uint32_t code) {

}

void EventsProducer_addEvent(uint32_t code) {

}
