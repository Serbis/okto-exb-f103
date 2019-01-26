/*
 * rings.c
 *
 *  Created on: 29 окт. 2017 г.
 *      Author: serbis
 */

/**
 * Реализация кольцевого буфера
 *
 * Версия: 10
 *
 */

#include <string.h>
#include "../include/rings.h"

#ifdef RINGS_FREERTOS

#include "stdlibo.h"

void RINGS_mutexLock(xSemaphoreHandle mutex) {
	xSemaphoreTake(mutex, portMAX_DELAY);
}

void RINGS_mutexUnLock(xSemaphoreHandle mutex) {
	xSemaphoreGive(mutex);
}

#endif

#ifdef RINGS_PTHREADS

void RINGS_mutexLock(pthread_mutex_t *mutex) {
	pthread_mutex_lock(mutex);
}

void RINGS_mutexUnLock(pthread_mutex_t *mutex) {
	pthread_mutex_unlock(mutex);
}

#endif




/**
 * Создает новый кольцевой буфер в динамической памяти
 *
 * @param size размер буфера в байтах
 * @param portMalloc 1 для использования менеджера памяти уровня ос
 * @return указатель на определение кольцевого буфера
 */
RingBufferDef* RINGS_createRingBuffer(uint16_t size, uint8_t overflowPolitics, bool lock) {
	RingBufferDef *rbd;
	uint8_t *buffer;

	rbd = (RingBufferDef*) malloc(sizeof(RingBufferDef));
	buffer = (uint8_t*) malloc(size);


	rbd->buffer = buffer;
	rbd->reader = 0;
	rbd->writer = 0;
    rbd->size = size;

	#ifdef RINGS_FREERTOS
    rbd->mutex = xSemaphoreCreateMutex();;
	#endif
	#ifdef RINGS_PTHREADS
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    rbd->mutex = mutex;
	#endif
    rbd->overflowPolitics = overflowPolitics;
    rbd->lock = lock;

	return rbd;
}

/**
 * Деструктор кольцевого буфера
 *
 * @param rbd определение кольца
 */
void RINGS_Free(RingBufferDef* rbd) {
	free(rbd->buffer);
}

/**
 * Считывает один байт из буфера
 *
 * @param rbd определение кольца
 * @return считанный байт
 */
uint8_t RINGS_read(RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	uint8_t b = *(rbd->buffer + rbd->reader);
    if (rbd->reader == rbd->size -1) {
        rbd->reader = 0;
    } else {
        if (rbd->reader != rbd->writer)
            rbd->reader++;
    }
    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }
	return b;
}

/**
 * Записывает дин байт в указанный буфер
 *
 * @param byte байт для записи
 * @param rbd определение кольца
 * @return количество записанных байт (в рамках функции всегда равно 1)
 */
uint8_t RINGS_write(uint8_t byte, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	*(rbd->buffer + rbd->writer) = byte;
    if (rbd->writer == rbd->size -1) {
        if (rbd->reader == 0) {
            switch (rbd->overflowPolitics) {
                case RINGS_OVERFLOW_SHIFT:
                    if (rbd->reader == rbd->size -1)
                        rbd->reader = 0;
                    else
                        rbd->reader++;
                    break;

                default:break;
            }
        }
        rbd->writer = 0;
    } else {
        if (rbd->writer == rbd->reader - 1) {
            switch (rbd->overflowPolitics) {
                case RINGS_OVERFLOW_SHIFT:
                    if (rbd->reader == rbd->size -1)
                        rbd->reader = 0;
                    else
                        rbd->reader++;
                    break;

                default:break;
            }
        }
        rbd->writer++;
    }

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }

	return 1;
}

/**
 * Записывает в буфер массив данных
 *
 * @param array указатель на массив данных
 * @param size размер массива данных
 * @param rbd представление кольца
 * @return количество записанных в буфер байт
 */
uint8_t RINGS_writeArray(uint8_t *array, uint16_t size, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	for (int i = 0; i < size; i++) {
		RINGS_write(*(array + i), rbd);
	}

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }

	return size;
}

/**
 * Считывает все имеющиеся в кольце в указанный буфер
 *
 * @param buff буфер в который происходит перенос данных
 * @param rbd определение кольца
 * @return количество перенесенных байт
 */
uint16_t RINGS_readAll(uint8_t *buff, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	uint16_t p = 0;

	while (rbd->reader != rbd->writer) {
		*(buff + p) = *(rbd->buffer + rbd->reader);
		p++;
		if (rbd->reader > rbd->size - 2) {
			rbd->reader = 0;
		} else {
			rbd->reader++;
		}
	}

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }

	return p;
}

/**
 * Возвращает данные из буфера как строку (с добавлением терминатора к конец
 * блока данных)
 *
 * @param lenght размер выборки данных
 * @param rbd определение кольца
 * @return указатель на созданную строку
 */
char* RINGS_readString(uint16_t lenght, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	uint16_t strSize = rbd->writer - rbd->reader + 1;
	char* str = (char*) malloc(strSize);
	memcpy(str, (uint8_t*) (rbd->buffer + rbd->reader), strSize - 1);
	*(str + strSize - 1) = 0;

	rbd->reader = rbd->writer;

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }

	return str;
}


/**
 * Возвращает данные из буфера как строку из заданного диапазона (с добавлением
 * терминатора к конец блока данных)
 *
 * @param start позиция начала чтения
 * @param lenght размер выборки данных
 * @param rbd определение кольца
 * @return указатель на созданную строку
 */
char* RINGS_readStringInRange(uint16_t start, uint16_t lenght, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	char* str = (char*) malloc(lenght + 1);
	uint16_t count = 0;
	uint16_t ptr = start;

	while(count < lenght) {
		*(str + count) = *(rbd->buffer + ptr);

		if (ptr == rbd->size -1) {
			ptr = 0;
		} else {
		    ptr++;
		}

		count++;
	}

	*(str + lenght) = 0;

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }

	return str;

}

/**
 * Возвращает количество данных в кольце
 *
 * @param rbd определение кольца
 * @return количество байт
 */
uint16_t RINGS_dataLenght(RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	if (rbd->writer < rbd->reader) {
	    if (rbd->lock) {
	    	RINGS_mutexUnLock(rbd->mutex);
	    	#ifdef RINGS_FREERTOS
	    	if (xTaskGetTickCount() != taw)
	    		taskYIELD();
			#endif
	    }

		return rbd->size - rbd->reader + rbd->writer;
	}

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }

	return rbd->writer - rbd->reader;
}

/**
 * Возвращает указатель на начало блока данных в кольце
 *
 * @param rbd определение кольца
 * @return указатель на начало блока данных
 */
uint8_t* RINGS_dataStart(RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	uint8_t *p = rbd->buffer + rbd->reader;

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }

    return p;
}



/**
 * Очищает блок данных в кольце (читай устанавливает редер на позицию райтера)
 *
 * @param rbd определение кольца
 */
void RINGS_dataClear(RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	rbd->reader = rbd->writer;

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }
}

/**
 * Очищает блок данных в кольце, заполняет весь буфер нулями, устанавливает
 * ридет и райтер в нулевую позицию
 *
 * @param rbd определение кольца
 */
void RINGS_dataClearFull(RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	rbd->reader = 0;
	rbd->writer = 0;

    for (int i = 0; i < rbd->size - 1; i++) {
    	*(rbd->buffer + i) = 0;
    }
    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }
}

/**
 * Очищает блок данных заданного размера в кольце
 *
 * @param size размер очищаемого блока от позиции ридера
 * @param rbd определение кольца
 */
void RINGS_dataClearBySize(uint16_t size, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	rbd->reader = rbd->reader + size;
    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }
}

/**
 * Возвращает байт по смещения от райтера. Не изменяет значение
 * райтера.
 *
 * Примечание: при использовании функции, следует помнить то, что
 * райтер всегда находится на один байт впереди последнего реально
 * записанного байта. Поэтому фактически первый байт по райтеру
 * находится на позиции по смещению -1.
 *
 * @param shift положительное или отрицательное смещение
 * @param rbd определение кольца
 */
uint8_t RINGS_getByShiftFromWriter(int32_t shift, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

    if (shift > 0) { //Если искомы байт находится впереди от райтера
        uint16_t ns = shift;
        uint16_t ptr = rbd->writer;
        while (ns > 0) {
            if (ptr == rbd->size -1) {
                ptr = 0;
            } else {
                ptr++;
            }
            ns--;
        }

        if (rbd->lock) {
        	RINGS_mutexUnLock(rbd->mutex);
        	#ifdef RINGS_FREERTOS
        	if (xTaskGetTickCount() != taw)
        		taskYIELD();
    		#endif
        }

        return *(rbd->buffer + ptr);

    } else if (shift < 0) { //Если искомый байт находится сзади от райтера
        int32_t ns = shift;
        uint16_t ptr = rbd->writer;
        while (ns < 0) {
            if (ptr == 0) {
                ptr = rbd->size - 1;
            } else {
                ptr--;
            }
            ns++;
        }

        if (rbd->lock) {
        	RINGS_mutexUnLock(rbd->mutex);
        	#ifdef RINGS_FREERTOS
        	if (xTaskGetTickCount() != taw)
        		taskYIELD();
    		#endif
        }
        return *(rbd->buffer + ptr);

    } else { //Если смещение равно нулю
        if (rbd->lock) {
        	RINGS_mutexUnLock(rbd->mutex);
        	#ifdef RINGS_FREERTOS
        	if (xTaskGetTickCount() != taw)
        		taskYIELD();
    		#endif
        }
        return *(rbd->buffer + rbd->writer);
    }
}

/**
 * Возвращает байт по смещения от ридера. Не изменяет значение
 * ридера.
 *
 *
 * @param shift положительное или отрицательное смещение
 * @param rbd определение кольца
 */
uint8_t RINGS_getByShiftFromReader(int32_t shift, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

    if (shift > 0) { //Если искомы байт находится впереди от ридера
        uint16_t ns = shift;
        uint16_t ptr = rbd->reader;
        while (ns > 0) {
            if (ptr == rbd->size -1) {
                ptr = 0;
            } else {
                ptr++;
            }
            ns--;
        }

        if (rbd->lock) {
        	RINGS_mutexUnLock(rbd->mutex);
        	#ifdef RINGS_FREERTOS
        	if (xTaskGetTickCount() != taw)
        		taskYIELD();
    		#endif
        }
        return *(rbd->buffer + ptr);

    } else if (shift < 0) { //Если искомый байт находится сзади от райтера
        int32_t ns = shift;
        uint16_t ptr = rbd->reader;
        while (ns < 0) {
            if (ptr == 0) {
                ptr = rbd->size - 1;
            } else {
                ptr--;
            }
            ns++;
        }

        if (rbd->lock) {
        	RINGS_mutexUnLock(rbd->mutex);
        	#ifdef RINGS_FREERTOS
        	if (xTaskGetTickCount() != taw)
        		taskYIELD();
    		#endif
        }
        return *(rbd->buffer + ptr);

    } else { //Если смещение равно нулю
        if (rbd->lock) {
        	RINGS_mutexUnLock(rbd->mutex);
        	#ifdef RINGS_FREERTOS
        	if (xTaskGetTickCount() != taw)
        		taskYIELD();
    		#endif
        }
        return *(rbd->buffer + rbd->reader);
    }
}

/**
 * Сдвигает ридер кольца на заданное количество байт
 *
 * @param shift положительное или отрицательное число сдвига
 * @param rbd определение кольца
 */
void RINGS_shiftReader(int32_t shift, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

    if (shift > 0) { //Если искомы байт находится впереди от райтера
        uint16_t ns = shift;
        uint16_t ptr = rbd->reader;
        while (ns > 0) {
            if (ptr == rbd->size -1) {
                ptr = 0;
            } else {
                ptr++;
            }
            ns--;
        }
        rbd->reader = ptr;
    } else if (shift < 0) { //Если искомый байт находится сзади от райтера
        int32_t ns = shift;
        uint16_t ptr = rbd->reader;
        while (ns < 0) {
            if (ptr == 0) {
                ptr = rbd->size - 1;
            } else {
                ptr--;
            }
            ns++;
        }
        rbd->reader = ptr;
    }

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }
}

/**
 * Сдвигает райтер кольца на заданное количество байт
 *
 * @param shift положительное или отрицательное число сдвига
 * @param rbd определение кольца
 */
void RINGS_shiftWriter(int32_t shift, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

    if (shift > 0) { //Если искомы байт находится впереди от райтера
        uint16_t ns = shift;
        uint16_t ptr = rbd->writer;
        while (ns > 0) {
            if (ptr == rbd->size -1) {
                ptr = 0;
            } else {
                ptr++;
            }
            ns--;
        }
        rbd->writer = ptr;
    } else if (shift < 0) { //Если искомый байт находится сзади от райтера
        int32_t ns = shift;
        uint16_t ptr = rbd->writer;
        while (ns < 0) {
            if (ptr == 0) {
                ptr = rbd->size - 1;
            } else {
                ptr--;
            }
            ns++;
        }
        rbd->writer = ptr;
    }

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }
}

/**
 * Извлекает из буфера некоторый объекм данных в байтовый массив
 *
 * @param position позиция начала выборки
 * @param size количество байт для выпборки
 * @prarm buffer указатель на буфер в который будет размещены данные
 * @param rbd определение кольца
 *
 * @return кол-во фактически записнных в буфер байт
 */
uint16_t RINGS_extractData(int32_t position, uint16_t size, uint8_t *buffer, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	uint16_t wptr = 0;
	uint16_t writed = 0;
	uint16_t realPosition = 0;
	if (position >= 0)
		realPosition = position;
	else
		realPosition = rbd->size - (0 - position);

	if (position > rbd->size - 1)
		realPosition = position - rbd->size;

	while (writed < size) {
		buffer[writed] = rbd->buffer[realPosition + wptr];
		writed++;
		if (realPosition + wptr > rbd->size - 2) {
			realPosition = 0;
			wptr = 0;
		} else {
			wptr++;
		}
	}

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }
	return wptr;
}

uint8_t RINGS_cmpData(uint16_t readerOffset, uint8_t *cmpData, uint16_t dataSize, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	uint16_t count = 0;
	uint16_t ptr = rbd->reader + readerOffset;
	if (ptr > rbd->size)
		ptr = ptr - rbd->size;

	while(count < dataSize) {
		volatile char ch = *(rbd->buffer + ptr);
		volatile char cmpch = *(cmpData + count);
		if (ch != cmpch) {
			ptr = ch;
		    if (rbd->lock) {
		    	RINGS_mutexUnLock(rbd->mutex);
		    	#ifdef RINGS_FREERTOS
		    	if (xTaskGetTickCount() != taw)
		    		taskYIELD();
				#endif
		    }

			return -1;
		}

		if (ptr == rbd->size -1) {
			ptr = 0;
		} else {
		    ptr++;
		}

		count++;
	}

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }

	return 0;
}

uint8_t RINGS_cmpDataReverse(uint16_t writerOffset, uint8_t *cmpData, uint16_t dataSize, RingBufferDef* rbd) {
	if (rbd->lock)
		RINGS_mutexLock(rbd->mutex);
	#ifdef RINGS_FREERTOS
	TickType_t taw = 0;
	if (rbd->lock)
		taw = xTaskGetTickCount();
	#endif

	uint16_t count = dataSize;
	uint16_t ptr = rbd->writer + writerOffset - 1;

	while(count > 0) {
		char ch = *(rbd->buffer + ptr);
		if (*(rbd->buffer + ptr) != *(cmpData + count - 1)) {
			ptr = ch;
		    if (rbd->lock) {
		    	RINGS_mutexUnLock(rbd->mutex);
		    	#ifdef RINGS_FREERTOS
		    	if (xTaskGetTickCount() != taw)
		    		taskYIELD();
				#endif
		    }

			return -1;
		}

		if (ptr == 0) {
		    ptr = rbd->size - 1;
		} else {
		    ptr--;
		}

		count--;
	}

    if (rbd->lock) {
    	RINGS_mutexUnLock(rbd->mutex);
    	#ifdef RINGS_FREERTOS
    	if (xTaskGetTickCount() != taw)
    		taskYIELD();
		#endif
    }

	return 0;
}
