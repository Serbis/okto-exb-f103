################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/libs/collections/src/lbq.c \
../Src/libs/collections/src/lbq8.c \
../Src/libs/collections/src/list.c \
../Src/libs/collections/src/map.c \
../Src/libs/collections/src/map2.c \
../Src/libs/collections/src/rings.c 

OBJS += \
./Src/libs/collections/src/lbq.o \
./Src/libs/collections/src/lbq8.o \
./Src/libs/collections/src/list.o \
./Src/libs/collections/src/map.o \
./Src/libs/collections/src/map2.o \
./Src/libs/collections/src/rings.o 

C_DEPS += \
./Src/libs/collections/src/lbq.d \
./Src/libs/collections/src/lbq8.d \
./Src/libs/collections/src/list.d \
./Src/libs/collections/src/map.d \
./Src/libs/collections/src/map2.d \
./Src/libs/collections/src/rings.d 


# Each subdirectory must supply rules for building sources it contributes
Src/libs/collections/src/%.o: ../Src/libs/collections/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32F103xB -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Src/libs" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Inc" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/STM32F1xx_HAL_Driver/Inc" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/CMSIS/Include" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Middlewares/Third_Party/FreeRTOS/Source/include" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS"  -Og -fmerge-all-constants -g3 -Wall -fmessage-length=0 -fmerge-all-constants -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


