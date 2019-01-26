//
// Created by serbis on 13.07.18.
//
#include "../include/utils.h"

void Utils_printArrayAsHex(uint8_t *array, uint32_t size) {
    for (int i = 0; i < size; i++) {
        _printf("%02X ", array[i]);
    }
    _printf("\n");
}
