// HOLDING REGISTERS ARE ANALOGUE INPUTS
// R as input can ONLY be read (R)

#ifndef IO_INPUT_REG_H
#define IO_INPUT_REG_H

#include "stm32g4xx_hal.h"

extern uint16_t io_input_reg_channel_count; // extern so modbus.c can check this

// Only include if enabled in the project config
#ifdef HAL_ADC_MODULE_ENABLED
#include "stm32g4xx_hal_adc.h"
#endif

#include "stm32g4xx_hal_gpio.h"
#include <stdbool.h> // Lets us use boolean logic

// Define a struct to represent an input register channel
typedef struct {
	void* handle; // ADC handle
	uint32_t channel; // ADC channel number
} IO_Input_Reg_Channel;



#define MAX_IO_INPUT_REG 4

// Adds a new channel to the list
void io_input_reg_add_channel(void* handle, uint32_t channel);

// Reads from an ADC
uint16_t io_input_reg_read(uint16_t index);

#endif
