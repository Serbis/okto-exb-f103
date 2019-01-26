//
// Created by serbis on 13.09.18.
//

#ifndef FMB_UART_RECEIVER_H
#define FMB_UART_RECEIVER_H

#include "../libs/collections/include/rings.h"

typedef struct {
	RingBufferDef *uartRing;
	RingBufferDef *gateRing;
} UartReceiverThreadArgs;

void UartReceiver_thread(UartReceiverThreadArgs *args);

#endif //FMB_CMD_PROCESSOR_H
