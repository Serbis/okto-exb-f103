#ifndef EXECUTOR_H_
#define EXECUTOR_H_

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
#define CMD_XYZ 6


typedef struct {
	uint8_t type;
	uint16_t size;
	uint8_t *data;
} ExecutorResult;

ExecutorResult* Executor_error(uint32_t code, char *data, ...);
ExecutorResult* Executor_cmd_adc_r(char *id);
ExecutorResult* Executor_cmd_gpio_w(char *pin, char *state);
ExecutorResult* Executor_cmd_gpio_r(char *pin);
ExecutorResult* Executor_cmd_adc_filter(char *interations, char *delay);
ExecutorResult* Executor_cmd_heap();
ExecutorResult* Executor_cmd_uptime();
ExecutorResult* Executor_cmd_pipe_m(char *self, char *mast);
ExecutorResult* Executor_cmd_ping(char *pld);

#endif
