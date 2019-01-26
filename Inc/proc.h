#ifndef PROC_H_
#define PROC_H_

#include <stdint.h>
#include "../libs/collections/include/lbq.h"

/** Unknown command */
#define PROCERR_UNC 100

/** Iscufficient args */
#define PROCERR_INSA 101

/** Bad arg type */
#define PROCERR_WTA 102

/** Strange error */
#define PROCERR_STRE 103

/** Action received from rf */
#define FROM_RF 0

/** Action received from uart */
#define FROM_UART 1

typedef struct ProcThreadArgs {
	LinkedBlockingQueue *upQueue;
	LinkedBlockingQueue *downQueue;
} ProcThreadArgs;

typedef struct ProcQueueElem {
	uint32_t id;
	uint8_t action;
	uint8_t size;
	uint8_t *data;
	uint8_t from;
} ProcQueueElem;

void Proc_thread(void const * argument);

#endif
