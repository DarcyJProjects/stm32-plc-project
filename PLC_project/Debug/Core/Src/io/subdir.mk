################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/io/io_analogue.c \
../Core/Src/io/io_digital.c 

OBJS += \
./Core/Src/io/io_analogue.o \
./Core/Src/io/io_digital.o 

C_DEPS += \
./Core/Src/io/io_analogue.d \
./Core/Src/io/io_digital.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/io/%.o Core/Src/io/%.su Core/Src/io/%.cyclo: ../Core/Src/io/%.c Core/Src/io/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../USB_Device/App -I../USB_Device/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-io

clean-Core-2f-Src-2f-io:
	-$(RM) ./Core/Src/io/io_analogue.cyclo ./Core/Src/io/io_analogue.d ./Core/Src/io/io_analogue.o ./Core/Src/io/io_analogue.su ./Core/Src/io/io_digital.cyclo ./Core/Src/io/io_digital.d ./Core/Src/io/io_digital.o ./Core/Src/io/io_digital.su

.PHONY: clean-Core-2f-Src-2f-io

