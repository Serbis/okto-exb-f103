/** This thread is used to build exb packets from a raw byte stream. From the generated packages, operations are created for the proc.c thread */

#include <stdbool.h>
#include <stddef.h>
#include "../Inc/exb_gate.h"
#include "../Inc/proc.h"
#include "../libs/oscl/include/malloc.h"
#include "../libs/collections/include/lbq8.h"
#include "../libs/collections/include/rings.h"
#include "../Inc/exb_packet.h"
#include "event_producer.h"

#define MODE_PREAMBLE 0
#define MODE_HEADER 1
#define MODE_BODY 2

void ExbGate_thread(ExbGateThreadArgs *args) {
	uint64_t prbits = EXB_PREAMBLE_R; //Revesed to big-endian preable
	uint8_t mode = MODE_PREAMBLE;
	ExbPacket *packet = NULL;
	uint16_t sbody = 0;

	//Up ring from rf or uart receiver
	RingBufferDef *upRing = args->upRing;

	//Proc operations queue
	LinkedBlockingQueue *downQueue = args->downQueue;

	//Internal packet construction buffer
	RingBufferDef *inBuf  = RINGS_createRingBuffer(150, RINGS_OVERFLOW_SHIFT, true);
	inBuf->reader = 4;
	inBuf->writer = 3;

	while(1) {
		if (RINGS_dataLenght(upRing) > 0) {
			uint8_t ch = RINGS_read(upRing);


			RINGS_write((uint8_t) ch, inBuf);
			uint16_t dlen = RINGS_dataLenght(inBuf);
			if (mode == MODE_PREAMBLE) { // If expected preamble form stream
				if (dlen >= EXB_PREAMBLE_SIZE) { // Buffer contain data with size of preamble or more
					int r = RINGS_cmpData(dlen - EXB_PREAMBLE_SIZE, (uint8_t*) &prbits, EXB_PREAMBLE_SIZE, inBuf);
			    	if (r == 0) {
			        	RINGS_dataClear(inBuf);
			            mode = MODE_HEADER;
			        }
			    }
			} else if (mode == MODE_HEADER) { // If collected packet header
				if (dlen >= EXB_HEADER_SIZE) { // Buffer contain data with size of header or more
			    	uint8_t *header = (uint8_t*) pmalloc(EXB_HEADER_SIZE);
			    	packet = (ExbPacket*) pmalloc(sizeof(ExbPacket));
			        RINGS_readAll((uint8_t*)header, inBuf);
			        ExbPacket_parsePacketHeader(packet, header, 0);
			        sbody = packet->length;
			        if (sbody > 128)
			        	mode = MODE_PREAMBLE;
			        mode = MODE_BODY;
			        pfree(header);
			     }
			 } else { // If collected packet body
			 	if (dlen >= sbody) {  // Buffer contain data with size of body or more
			    	uint8_t *body = (uint8_t*) pmalloc(sbody);

			        RINGS_readAll(body, inBuf);

			        Indicator_cmdIn();

			        // Ok, packet if fully completed, detect action and emit it
			        if (packet->type == EXB_TYPE_EVENT_ACK) {
			        	xQueueSend(args->ackEventStream, &(packet->tid), portMAX_DELAY);
			        	pfree(body);
			        } else {
			        	ProcQueueElem *elem = (ProcQueueElem*) pmalloc(sizeof(ProcQueueElem));
			        	elem->id = packet->tid;
			        	elem->action = packet->type;
			        	elem->size = packet->length;
			        	elem->from = args->marker;
			        	elem->data = body;
			        	downQueue->enqueue(downQueue, elem);
			        }

			        pfree(packet);

			        // Expect next packet from the data stream
			        mode = MODE_PREAMBLE;
			     }
			 }
		}

		// Do not remove this delay, because it will broken all mutexes in the everywhere
		vTaskDelay(1);
	}
}
