#include "../Inc/uart_receiver.h"
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "libs/collections/include/rings.h"

extern RingBufferDef *inBuf;
extern LinkedBlockingQueue *cmdQueue;

void UartReceiver_thread(void *args) {
	uint16_t offset = 0;


	while (true) {
		if (RINGS_dataLenght(inBuf) > 0) {
			taskENTER_CRITICAL();
			uint16_t pos = inBuf->writer;
			uint8_t b = RINGS_read(inBuf);
			taskEXIT_CRITICAL();
			offset++;
			if (b == '\r') {
				uint16_t len = offset;
				char *str = pmalloc(len + 1);
				str[len] = 0;

				RINGS_extractData(pos - len, len, (uint8_t *) str, inBuf);
				cmdQueue->enqueue(cmdQueue, str);
				offset = 0;
			}
		}
	}
}
