// HOLDING REGISTERS ARE ANALOGUE OUTPUTS
// R/W as output can be set (W), but last state can be read (R)

#ifndef IO_HOLDING_REG_H
#define IO_HOLDING_REG_H

#include "stm32g4xx_hal.h"

extern uint16_t io_holding_reg_channel_count; // extern so modbus.c can check this

// Only include if enabled in the project config
#ifdef HAL_DAC_MODULE_ENABLED
#include "stm32g4xx_hal_dac.h"
#endif

#include "stm32g4xx_hal_gpio.h"
#include <stdbool.h> // Lets us use boolean logic

// Define a struct to represent an analogue I/O channel
typedef struct {
	void* handle; // ADC/DAC handle
	uint32_t channel; // ADC/DAC channel number
	uint16_t storedValue; // Last output value
} IO_Holding_Reg_Channel;



#define MAX_IO_HOLDING_REG 2

// Adds a new channel to the list
void io_analogue_add_channel(void* handle, uint32_t channel);

// Reads from an ADC
uint16_t io_holding_reg_read(uint16_t index); // uint16_t as modbus uses 16 bit values

// Writes to a DAC
void io_holding_reg_write(uint16_t index, uint16_t value);

#endif
