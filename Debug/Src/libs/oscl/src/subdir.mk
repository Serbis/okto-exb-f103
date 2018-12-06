################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/libs/oscl/src/data.c \
../Src/libs/oscl/src/hardware.c \
../Src/libs/oscl/src/malloc.c \
../Src/libs/oscl/src/threads.c \
../Src/libs/oscl/src/time.c \
../Src/libs/oscl/src/utils.c 

OBJS += \
./Src/libs/oscl/src/data.o \
./Src/libs/oscl/src/hardware.o \
./Src/libs/oscl/src/malloc.o \
./Src/libs/oscl/src/threads.o \
./Src/libs/oscl/src/time.o \
./Src/libs/oscl/src/utils.o 

C_DEPS += \
./Src/libs/oscl/src/data.d \
./Src/libs/oscl/src/hardware.d \
./Src/libs/oscl/src/malloc.d \
./Src/libs/oscl/src/threads.d \
./Src/libs/oscl/src/time.d \
./Src/libs/oscl/src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
Src/libs/oscl/src/%.o: ../Src/libs/oscl/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32F103xB -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Src/libs" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Inc" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/STM32F1xx_HAL_Driver/Inc" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Drivers/CMSIS/Include" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Middlewares/Third_Party/FreeRTOS/Source/include" -I"/home/serbis/code/main/MRCS/hw/frs/base_10348_0/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


