#ifndef EXB_GATE_H_
#define EXB_GATE_H_

#include "../libs/collections/include/lbq.h"
#include "../libs/collections/include/lbq8.h"
#include "../libs/collections/include/rings.h"

typedef struct ExbGateThreadArgs {
	RingBufferDef *upRing;
	LinkedBlockingQueue *downQueue;
	uint8_t marker;
} ExbGateThreadArgs;

void ExbGate_thread(ExbGateThreadArgs *args);

#endif
