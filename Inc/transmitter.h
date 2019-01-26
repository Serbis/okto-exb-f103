#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

typedef struct TransmitterQueueElem {
	uint32_t id;
	uint8_t action;
	uint8_t size;
	uint8_t *data;
	uint8_t from;
} TransmitterQueueElem;

void Transmitter_thread(void const * argument);

#endif
