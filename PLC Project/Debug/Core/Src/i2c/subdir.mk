################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/i2c/bmp280.c \
../Core/Src/i2c/display.c \
../Core/Src/i2c/i2c.c 

OBJS += \
./Core/Src/i2c/bmp280.o \
./Core/Src/i2c/display.o \
./Core/Src/i2c/i2c.o 

C_DEPS += \
./Core/Src/i2c/bmp280.d \
./Core/Src/i2c/display.d \
./Core/Src/i2c/i2c.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/i2c/%.o Core/Src/i2c/%.su Core/Src/i2c/%.cyclo: ../Core/Src/i2c/%.c Core/Src/i2c/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../USB_Device/App -I../USB_Device/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-i2c

clean-Core-2f-Src-2f-i2c:
	-$(RM) ./Core/Src/i2c/bmp280.cyclo ./Core/Src/i2c/bmp280.d ./Core/Src/i2c/bmp280.o ./Core/Src/i2c/bmp280.su ./Core/Src/i2c/display.cyclo ./Core/Src/i2c/display.d ./Core/Src/i2c/display.o ./Core/Src/i2c/display.su ./Core/Src/i2c/i2c.cyclo ./Core/Src/i2c/i2c.d ./Core/Src/i2c/i2c.o ./Core/Src/i2c/i2c.su

.PHONY: clean-Core-2f-Src-2f-i2c

