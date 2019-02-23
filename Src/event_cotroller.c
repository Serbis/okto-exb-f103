#include <string.h>
#include "../libs/collections/include/map2.h"
#include "stdlibo.h"
#include "event_cotroller.h"
#include "transmitter.h"
#include "exb_packet.h"


void EventController_handleEvent(PendingEvent *pEvent, uint32_t *idCounter, Map *ackMap, bool resend, LinkedBlockingQueue *queue) {
	uint16_t blobSize = 4 + pEvent->size;
	uint8_t *blob = (uint8_t*) malloc(blobSize);

	// Set event code
	blob[0] = (pEvent->code & 0xff000000) >> 24;
	blob[1] = (pEvent->code & 0x00ff0000) >> 16;
	blob[2] = (pEvent->code & 0x0000ff00) >> 8;
	blob[3] = (pEvent->code & 0x000000ff);

	//Set event data
	if (pEvent->size > 0)
		memcpy(blob + 4, pEvent->data, pEvent->size);

	//Register in the confirmation table if event is confirmed
	pEvent->timestamp = xTaskGetTickCount();
	if (ackMap->inner->size <= MAX_WAIT_ACK && !resend) {
		char *tid = itoa2(*idCounter);
		MAP_add(tid, pEvent, ackMap);
		free(tid);
	}


	//Send event packet to transmition
	TransmitterQueueElem *tqm = (TransmitterQueueElem*) malloc(sizeof(TransmitterQueueElem));
	tqm->id = *idCounter;
	if (pEvent->confirmed)
		tqm->action = EXB_TYPE_EVENTC;
	else
		tqm->action = EXB_TYPE_EVENT;
	tqm->size = blobSize;
	tqm->data = blob;
	tqm->from = FROM_ALL;

	queue->enqueue(queue, tqm);

	(*idCounter)++;
	if (!pEvent->confirmed) {
		if (pEvent->data != NULL)
			free(pEvent->data);
		free(pEvent);
	}
}

void EventController_thread(EventsControllerThreadArgs *args) {
	Map *ackMap = MAP_new();
	uint32_t idCounter = 1;

	PendingEvent *pEvent;
	uint32_t ackId;


	while(1) {

		//Check for resend unconfirmed events
		MapIterator *it = MAP_ITERATOR_new(ackMap);
		time_t current = xTaskGetTickCount();
		while(MAP_ITERATOR_hasNext(it)) {
			pEvent = (PendingEvent*) MAP_ITERATOR_next(it);
			if (current - pEvent->timestamp > RETR_DELAY) {
				EventController_handleEvent(pEvent, &idCounter, ackMap, true, args->transmitterQueue);
			}
		}

		free(it);

		if (xQueueReceive(args->pendingEventStream, &pEvent, 100) == pdPASS) {
			EventController_handleEvent(pEvent, &idCounter, ackMap, false, args->transmitterQueue);
		}

		if (xQueueReceive(args->ackEventStream, &ackId, 100) == pdPASS) {
			char *tid = itoa2(ackId);
			PendingEvent *pe = MAP_remove(tid, ackMap);
			free(tid);
			if (pe != NULL) {
				if (pEvent->data != NULL)
					free(pEvent->data);
				free(pEvent);
			}
		}

		vTaskDelay(1);
	}
}
