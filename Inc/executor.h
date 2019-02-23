#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include "stm32f1xx_hal.h"

/** Maximum sum data size witch may be passed as ... to Executor_result function */
#define MAX_RET_PAYLOAD 50

#define CMD_ADC_R 0
#define CMD_GPIO_W 1
#define CMD_GPIO_R 2
#define CMD_ADC_FILTER 3
#define CMD_ADC_HEAP 4
#define CMD_ADC_UPTIME 5
#define CMD_PIPE_M 7
#define CMD_PING 8
#define CMD_GPIO_M 9
#define CMD_EVENT_R 10
#define CMD_EVENT_U 11
#define CMD_EVENT_L 12
#define CMD_XYZ 6



typedef struct {
	uint8_t type;
	uint16_t size;
	uint8_t *data;
} ExecutorResult;

/** Struct for map_gpio function. Read it`s desciption fot details */
typedef struct {
	/** Phisical gpio port */
	GPIO_TypeDef *port;
	/** Phisical pin number on the gpio port */
	uint16_t pin;
	/** Adc pin marker */
	bool adc;
} GpioMap;

uint8_t Executor_pinToGpio(uint16_t gpioPin);
GpioMap* Executor_map_gpio(char *pin);
ExecutorResult* Executor_error(uint32_t code, char *data, ...);
ExecutorResult* Executor_cmd_adc_r(char *id);
ExecutorResult* Executor_cmd_gpio_w(char *pin, char *state);
ExecutorResult* Executor_cmd_gpio_r(char *pin);
ExecutorResult* Executor_cmd_gpio_m(char *pin, char *mode, char *a1, char *a2, char *a3);
ExecutorResult* Executor_cmd_adc_filter(char *interations, char *delay);
ExecutorResult* Executor_cmd_heap();
ExecutorResult* Executor_cmd_uptime();
ExecutorResult* Executor_cmd_pipe_m(char *self, char *mast);
ExecutorResult* Executor_cmd_ping(char *pld);
ExecutorResult* Executor_cmd_event_r(char *code, char *type, char *confirmed, char *arg1, char *arg2);
ExecutorResult* Executor_cmd_event_u(char *code);
ExecutorResult* Executor_cmd_event_l();


#endif
