#include "../include/malloc.h"
#include "cmsis_os.h"

void *pmalloc (size_t __size) {
	return pvPortMalloc(__size);
}

void pfree(void *__ptr) {
	vPortFree(__ptr);
}

