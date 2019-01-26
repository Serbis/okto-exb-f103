
/** This header is used for redefine os specific stdlib functions such as malloc, free and etc */

#ifndef STDLIBO_H_
#define STDLIBO_H_

#include "cmsis_os.h"

#define malloc(size) pvPortMalloc(size)
#define free(ptr) vPortFree(ptr)

#endif
