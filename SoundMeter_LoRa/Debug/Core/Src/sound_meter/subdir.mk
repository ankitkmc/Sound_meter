################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/sound_meter/svan958a.c 

OBJS += \
./Core/Src/sound_meter/svan958a.o 

C_DEPS += \
./Core/Src/sound_meter/svan958a.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/sound_meter/%.o Core/Src/sound_meter/%.su Core/Src/sound_meter/%.cyclo: ../Core/Src/sound_meter/%.c Core/Src/sound_meter/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-sound_meter

clean-Core-2f-Src-2f-sound_meter:
	-$(RM) ./Core/Src/sound_meter/svan958a.cyclo ./Core/Src/sound_meter/svan958a.d ./Core/Src/sound_meter/svan958a.o ./Core/Src/sound_meter/svan958a.su

.PHONY: clean-Core-2f-Src-2f-sound_meter

