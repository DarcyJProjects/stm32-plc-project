// TODO: I2C support

// HOLDING REGISTERS ARE ANALOGUE OUTPUTS
// R/W as output can be set (W), but last state can be read (R)

#ifndef IO_HOLDING_REG_H
#define IO_HOLDING_REG_H

#include "stm32g4xx_hal.h"

extern uint16_t io_holding_reg_channel_count; // count for all channels or any type, extern so modbus.c can check this
extern uint16_t io_holding_adc_reg_channel_count; // count for only physical dac channels for max channel enforcement


// Only include if enabled in the project config
#ifdef HAL_DAC_MODULE_ENABLED
#include "stm32g4xx_hal_dac.h"
#endif

#include "stm32g4xx_hal_gpio.h"
#include <stdbool.h> // Lets us use boolean logic


// Define a struct to represent a holding register channel
typedef struct {
	void (*write_func)(void* context, uint16_t value); // Function to write to the particular channel device.
	void* context; // Pointer passed to that function (e.g., DAC channel address, or I2C related handle)
	uint16_t storedValue; // Last output value
} IO_Holding_Reg_Channel;


#define MAX_IO_HOLDING_REG 2

// Adds a new channel to the list
void io_holding_reg_add_channel(void (*write_func)(void*, uint16_t), void* context);


uint16_t io_holding_reg_read(uint16_t index);

void io_holding_reg_write(uint16_t index, uint16_t value);

// Write function for physical adc channels, i2c is device dependent
void dac_write_func(uint32_t channel, uint16_t value);

#endif
