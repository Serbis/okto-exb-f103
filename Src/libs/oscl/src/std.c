#include <string.h>
#include "../include/std.h"
#include "cmsis_os.h"

extern UART_HandleTypeDef huart1;

/*
 * Выводит форматированную строку в UART3
 *
 * TODO функция не является потокобезопасной!
 */
void _printf(char *str, ...) {
	UART_HandleTypeDef *huart = &huart1;
	size_t len = strlen(str);
	size_t lineSize = len + 200;

	char *msg = (char*) pvPortMalloc(lineSize);

	va_list args;
	va_start(args, str);
	vsprintf(msg, str, args);
	va_end(args);

	 taskENTER_CRITICAL();
	 huart->gState = HAL_UART_STATE_READY;
	 HAL_UART_Transmit(&huart1, msg, strlen(msg), 1000);
	 taskEXIT_CRITICAL();

	vPortFree(msg);
}
