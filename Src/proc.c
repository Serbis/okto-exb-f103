#include <string.h>
#include "proc.h"
#include "../Inc/transmitter.h"
#include "../libs/oscl/include/malloc.h"
#include "../libs/collections/include/lbq.h"
#include "splitter.h"
#include "stdlibo.h"
#include "executor.h"
#include "exb_packet.h"


ExecutorResult* Proc_complete(StringSplitter *splitter, uint8_t type, StringCmpResult *cmpr) {
	uint8_t result = cmpr->result;
	free(cmpr);
	if (result == CMPR_SUCCESS) {
		if (type == CMD_ADC_R) return Executor_cmd_adc_r(Splitter_getStringElem(splitter, 1));
		else if (type == CMD_GPIO_W) return Executor_cmd_gpio_w(Splitter_getStringElem(splitter, 1), Splitter_getStringElem(splitter, 2));
		else if (type == CMD_GPIO_R) return Executor_cmd_gpio_r(Splitter_getStringElem(splitter, 1));
		else if (type == CMD_ADC_FILTER) return Executor_cmd_adc_filter(Splitter_getStringElem(splitter, 1), Splitter_getStringElem(splitter, 2));
		else if (type == CMD_ADC_HEAP) return Executor_cmd_heap();
		else if (type == CMD_ADC_UPTIME) return Executor_cmd_uptime();
		else if (type == CMD_PIPE_M) return Executor_cmd_pipe_m(Splitter_getStringElem(splitter, 1), Splitter_getStringElem(splitter, 2));
		else if (type == CMD_PING) return Executor_cmd_ping(Splitter_getStringElem(splitter, 1));
		else if (type == CMD_XYZ) return Executor_error(PROCERR_UNC, "-");
	} else if (result == CMPR_ERR_MATCH) {
		return Executor_error(PROCERR_WTA, "-");
	} else if (result == CMPR_ERR_SPLLEN) {
		return Executor_error(PROCERR_INSA, "-");
	}

	return Executor_error(PROCERR_STRE, "-");
}

void Proc_thread(void const * args) {
	ProcThreadArgs *parg = (ProcThreadArgs*) args;
	LinkedBlockingQueue *upQueue = parg->upQueue;
	LinkedBlockingQueue *downQueue = parg->downQueue;


	while(1) {
		if (upQueue->size(upQueue) > 0) {
			ProcQueueElem *elem = (ProcQueueElem*) upQueue->dequeue(upQueue);

			char *cmd = (char*) malloc(elem->size + 1);
			memcpy(cmd, elem->data, elem->size);
			cmd[elem->size] = 0;
			StringSplitter *splitter = Splitter_splitString(cmd, " ", 50, 10);
			ExecutorResult *er;


			char *ncmd = splitter->elems[0];

			if (strcmp(ncmd, "adc_r") == 0)
				er = Proc_complete(splitter, CMD_ADC_R, Splitter_compareString(splitter, 2, 2, "adc_r", "*n"));
			else if (strcmp(ncmd, "gpio_w") == 0)
				er = Proc_complete(splitter, CMD_GPIO_W, Splitter_compareString(splitter, 3, 3, "gpio_w", "*n", "*n"));
			else if (strcmp(ncmd, "gpio_r") == 0)
				er = Proc_complete(splitter, CMD_GPIO_R, Splitter_compareString(splitter, 2, 2, "gpio_r", "*n"));
			else if (strcmp(ncmd, "adc_filter") == 0)
				er = Proc_complete(splitter, CMD_ADC_FILTER, Splitter_compareString(splitter, 1, 3, "adc_filter", "*n", "*n"));
			else if (strcmp(ncmd, "heap") == 0)
				er = Proc_complete(splitter, CMD_ADC_HEAP, Splitter_compareString(splitter, 1, 1, "heap"));
			else if (strcmp(ncmd, "uptime") == 0)
				er = Proc_complete(splitter, CMD_ADC_UPTIME, Splitter_compareString(splitter, 1, 1, "uptime"));
			else if (strcmp(ncmd, "pipe_m") == 0)
				er = Proc_complete(splitter, CMD_PIPE_M, Splitter_compareString(splitter, 1, 3, "pipe_m", "*h", "*h"));
			else if (strcmp(ncmd, "ping") == 0)
				er = Proc_complete(splitter, CMD_PING, Splitter_compareString(splitter, 2, 2, "ping", "*a"));
			else
				er = Proc_complete(splitter, CMD_XYZ, Splitter_compareString(splitter, 1, 1, "xyz"));


			TransmitterQueueElem *tqm = (TransmitterQueueElem*) pmalloc(sizeof(TransmitterQueueElem));
			tqm->id = elem->id;
			tqm->action = er->type;
			tqm->size = er->size;
			tqm->data = er->data;
			tqm->from = elem->from;


			for (uint8_t i = 0; i < splitter->size; i++) {
				free(splitter->elems[i]);
			}
			free(splitter->elems);
			free(splitter);
			free(elem->data);
			free(elem);
			free(er);
			free(cmd);


			downQueue->enqueue(downQueue, tqm);
		}
		vTaskDelay(1);
	}
}
