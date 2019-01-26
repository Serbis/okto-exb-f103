#include <string.h>
#include <math.h>
#include "indicator.h"
#include "stdlibo.h"
#include "../libs/oscl/include/malloc.h"
#include "../libs/collections/include/lbq.h"
#include "../Inc/exb_packet.h"
#include "../Inc/transmitter.h"
#include "../Inc/NRF24.h"
#include "../libs/oscl/include/std.h"
#include "proc.h"

extern UART_HandleTypeDef huart1;

void Transmitter_thread(void const * argument) {
	LinkedBlockingQueue *queue = (LinkedBlockingQueue*) argument;
	UART_HandleTypeDef *huart = &huart1;

	while(1) {
		if (queue->size(queue) > 0) {
			TransmitterQueueElem *elem = queue->dequeue(queue);

			ExbPacket *packet = (ExbPacket*) pmalloc(sizeof(ExbPacket));
			packet->preamble = EXB_PREAMBLE;
			packet->tid = elem->id;
			packet->type = elem->action;
			packet->length = elem->size;
			packet->body = elem->data;
			uint16_t sbin = 0;
			uint8_t *bin = ExbPacket_toBinary(packet, &sbin);

			if (elem->from == FROM_RF) {
				uint16_t frags = ceil((double) sbin / 32);

				for (uint16_t i = 0; i < frags; i++) {
					uint16_t start = i * 32;
					uint8_t len = 32;
					if (start + 32 > sbin)
						len =  sbin - i * 32;

					uint8_t *payload = (uint8_t*) malloc(32);
					memcpy(payload, bin + start, len);

					NRF24_Send(payload);
					free(payload);

				}
			} else {
				taskENTER_CRITICAL();
				huart->gState = HAL_UART_STATE_READY;
				HAL_UART_Transmit(&huart1, bin, sbin, 1000);
				taskEXIT_CRITICAL();
			}

			Indicator_cmdOut();

			pfree(packet);
			pfree(bin);
			pfree(elem->data);
			pfree(elem);
		}
		vTaskDelay(1);
	}
}