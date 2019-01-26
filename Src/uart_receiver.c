/** This thread is designed to receive raw data from uart and redirect them to the exb_gate.c thread. */

#include "../Inc/uart_receiver.h"
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "stdlibo.h"


void UartReceiver_thread(UartReceiverThreadArgs *args) {

	// Ring buffer to stream data from uart. The data buffer is shared by the interrupt handler upon the
	// arrival of data from uart
	RingBufferDef *uartRing = args->uartRing;

	// Downstream data buffer. This buffer is used in conjunction with the exb_gate stream, which receives
	// data from it
	RingBufferDef *gateRing = args->gateRing;

	while (true) {

		// If data is available in the buffer, the buffer is read with a global interrupt disable. The
		// received data block is sent to the downstream ring buffer
		if (RINGS_dataLenght(uartRing) > 0) {
			taskENTER_CRITICAL();
			uint16_t size = RINGS_dataLenght(uartRing);
			uint8_t *data = (uint8_t*) malloc(size);
			RINGS_readAll(data, uartRing);
			taskEXIT_CRITICAL();

			for (uint16_t i = 0; i < size; i++) {
				RINGS_write(data[i], gateRing);
			}

			free(data);
		}

		// Buffer read iteration delay Due to the fact that the read operation of the buffer prohibits
		// interrupts, a sufficiently large time period is needed here so that there would be no conflicts
		// with the interrupt system. It is experimentally established that 10ms does not affect the
		// operation of interrupts
		vTaskDelay(10);
	}
}
