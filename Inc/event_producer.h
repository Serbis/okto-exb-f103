#ifndef EVENT_PRODUCER_H_
#define EVENT_PRODUCER_H_

#include "cmsis_os.h"

typedef struct {
	QueueHandle_t pendingEventStream;
} EventsProducerThreadArgs;

typedef struct {
	uint8_t type;
	uint8_t data;
} EventDef;

QueueHandle_t eventStream;

void EventsProducer_thread(EventsProducerThreadArgs *args);
void EventsProducer_removeEvent(uint32_t code);
void EventsProducer_addEvent(uint32_t code);

#endif
