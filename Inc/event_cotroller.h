#ifndef EVENT_COTROLLER_H_
#define EVENT_COTROLLER_H_

#include <stdbool.h>
#include "cmsis_os.h"
#include "../libs/collections/include/lbq.h"

#define MAX_WAIT_ACK 10
#define RETR_DELAY 5000

typedef struct {
	bool confirmed;
	uint32_t timestamp;
	uint32_t code;
	uint32_t size;
	uint8_t *data;
} PendingEvent;

typedef struct {
	QueueHandle_t pendingEventStream;
	QueueHandle_t ackEventStream;
	LinkedBlockingQueue *transmitterQueue;
} EventsControllerThreadArgs;


void EventController_thread(EventsControllerThreadArgs *args);

#endif
