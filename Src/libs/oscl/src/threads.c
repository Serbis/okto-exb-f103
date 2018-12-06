#include <stdint.h>
#include "../include/threads.h"
#include "../include/malloc.h"
#include "cmsis_os.h"


thread_t NewThread(void (*run)(void *), void *args, uint16_t stackSize, char *name, uint64_t priority) {
	TaskHandle_t xHandle = NULL;
	BaseType_t result = xTaskCreate(run, name, stackSize, args, priority, &xHandle );

    if (result == pdPASS)
    	return xHandle;
    else
    	return NULL;
}

mutex_t* NewMutex() {
	//SemaphoreHandle_t *mutex = pmalloc(sizeof(SemaphoreHandle_t));
	//*mutex = xSemaphoreCreateMutex();
	//return mutex;
	return xSemaphoreCreateMutex();
}

void MutexLock(mutex_t *mutex) {
	xSemaphoreTake(mutex, portMAX_DELAY);
}

int MutexTryLock(mutex_t *mutex) {
	BaseType_t result = xSemaphoreTake(mutex, 1);
	if (result == pdTRUE)
		return 0;
	else
		return 1;
}

void MutexUnlock(mutex_t *mutex) {
	xSemaphoreGive(mutex);
}
