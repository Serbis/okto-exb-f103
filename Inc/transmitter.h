#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

/** Action received from rf */
#define FROM_RF 0

/** Action received from uart */
#define FROM_UART 1

/* Actrion from/to uart and rf (used with events)*/
#define FROM_ALL 2


typedef struct TransmitterQueueElem {
	uint32_t id;
	uint8_t action;
	uint8_t size;
	uint8_t *data;
	uint8_t from;
} TransmitterQueueElem;

void Transmitter_thread(void const * argument);

#endif
