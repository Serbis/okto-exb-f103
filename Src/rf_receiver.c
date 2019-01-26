/** This thread is designed to receive raw data from nrf24 and redirect them to the exb_gate.c thread. */

#include "indicator.h"
#include "../Inc/rf_receiver.h"
#include "../Inc/NRF24.h"
#include "../libs/oscl/include/malloc.h"
#include "../libs/oscl/include/threads.h"
#include "../libs/collections/include/lbq8.h"
#include "../libs/collections/include/rings.h"

// rf operation blocking mutext, used with nrf library for exclude simultaneous rx and tx opertaions
mutex_t *rf_mutex;

void RfReceiver_thread(void const * args) {

	// Downstream data buffer. This buffer is used in conjunction with the exb_gate stream, which receives
	// data from it
	RingBufferDef *ring = (RingBufferDef*) args;

	while(1) {
		MutexLock(rf_mutex);
		TickType_t taw = xTaskGetTickCount();
		if (NRF24_available()) { // If nrf rx buffer have new data
			uint8_t *payload = (uint8_t*) pmalloc(TX_PLOAD_WIDTH);
			NRF24_Receive(payload); // Receive packet from nrf
			for (int i = 0; i < TX_PLOAD_WIDTH; i++) { //Write packet payload to the downstream buffer
				RINGS_write(payload[i], ring);
			}
			pfree(payload);
		}
		MutexUnlock(rf_mutex);
		if (xTaskGetTickCount() != taw)
			taskYIELD();
		vTaskDelay(1);
	}
}
