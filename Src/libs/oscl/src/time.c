#include "../include/time.h"
#include "cmsis_os.h"

//Return current system time in millis
//WARNING: Not ISR safe
uint64_t SystemTime() {
    return (uint64_t) xTaskGetTickCount();
}

//Delay current thread for some millis
void DelayMillis(uint64_t millis) {
	vTaskDelay(millis);
}
