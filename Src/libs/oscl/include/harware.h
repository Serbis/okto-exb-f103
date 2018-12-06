//
// Created by serbis on 13.09.18.
//

#ifndef FMB_HARWARE_H
#define FMB_HARWARE_H

#include <stdint.h>

void Harware_initUart();
int32_t Hardware_readAdc(uint8_t number);
bool Hardware_writeToUart(char *pData);

#endif //FMB_HARWARE_H
