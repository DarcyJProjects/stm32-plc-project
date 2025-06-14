################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/automation/automation.c 

OBJS += \
./Core/Src/automation/automation.o 

C_DEPS += \
./Core/Src/automation/automation.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/automation/%.o Core/Src/automation/%.su Core/Src/automation/%.cyclo: ../Core/Src/automation/%.c Core/Src/automation/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../USB_Device/App -I../USB_Device/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-automation

clean-Core-2f-Src-2f-automation:
	-$(RM) ./Core/Src/automation/automation.cyclo ./Core/Src/automation/automation.d ./Core/Src/automation/automation.o ./Core/Src/automation/automation.su

.PHONY: clean-Core-2f-Src-2f-automation

