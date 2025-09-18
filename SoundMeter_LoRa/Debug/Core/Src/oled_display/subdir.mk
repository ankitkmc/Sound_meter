################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/oled_display/SH1106.c \
../Core/Src/oled_display/fonts.c 

OBJS += \
./Core/Src/oled_display/SH1106.o \
./Core/Src/oled_display/fonts.o 

C_DEPS += \
./Core/Src/oled_display/SH1106.d \
./Core/Src/oled_display/fonts.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/oled_display/%.o Core/Src/oled_display/%.su Core/Src/oled_display/%.cyclo: ../Core/Src/oled_display/%.c Core/Src/oled_display/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/EKL_PROJECTS/SOUND_METER/Firmware/SoundMeter_LoRa/Core/Src/oled_display" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-oled_display

clean-Core-2f-Src-2f-oled_display:
	-$(RM) ./Core/Src/oled_display/SH1106.cyclo ./Core/Src/oled_display/SH1106.d ./Core/Src/oled_display/SH1106.o ./Core/Src/oled_display/SH1106.su ./Core/Src/oled_display/fonts.cyclo ./Core/Src/oled_display/fonts.d ./Core/Src/oled_display/fonts.o ./Core/Src/oled_display/fonts.su

.PHONY: clean-Core-2f-Src-2f-oled_display

