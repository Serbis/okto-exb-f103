################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/FreeRTOS-openocd.c \
../Src/NRF24.c \
../Src/adc.c \
../Src/config.c \
../Src/eeprom.c \
../Src/exb_gate.c \
../Src/exb_packet.c \
../Src/executor.c \
../Src/freertos.c \
../Src/indicator.c \
../Src/main.c \
../Src/proc.c \
../Src/rf_receiver.c \
../Src/splitter.c \
../Src/stm32f1xx_hal_msp.c \
../Src/stm32f1xx_it.c \
../Src/system_stm32f1xx.c \
../Src/transmitter.c \
../Src/uart_receiver.c 

OBJS += \
./Src/FreeRTOS-openocd.o \
./Src/NRF24.o \
./Src/adc.o \
./Src/config.o \
./Src/eeprom.o \
./Src/exb_gate.o \
./Src/exb_packet.o \
./Src/executor.o \
./Src/freertos.o \
./Src/indicator.o \
./Src/main.o \
./Src/proc.o \
./Src/rf_receiver.o \
./Src/splitter.o \
./Src/stm32f1xx_hal_msp.o \
./Src/stm32f1xx_it.o \
./Src/system_stm32f1xx.o \
./Src/transmitter.o \
./Src/uart_receiver.o 

C_DEPS += \
./Src/FreeRTOS-openocd.d \
./Src/NRF24.d \
./Src/adc.d \
./Src/config.d \
./Src/eeprom.d \
./Src/exb_gate.d \
./Src/exb_packet.d \
./Src/executor.d \
./Src/freertos.d \
./Src/indicator.d \
./Src/main.d \
./Src/proc.d \
./Src/rf_receiver.d \
./Src/splitter.d \
./Src/stm32f1xx_hal_msp.d \
./Src/stm32f1xx_it.d \
./Src/system_stm32f1xx.d \
./Src/transmitter.d \
./Src/uart_receiver.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32F103xB -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Src/libs" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Inc" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/STM32F1xx_HAL_Driver/Inc" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/CMSIS/Include" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Middlewares/Third_Party/FreeRTOS/Source/include" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


