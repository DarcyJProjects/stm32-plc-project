// DISCRETE INPUTS ARE DIGITAL INPUTS
// R as input can ONLY be read (R)

#ifndef IO_DISCRETE_IN_H
#define IO_DISCRETE_IN_H

#include "stm32g4xx_hal.h"
#include <stdbool.h> // Lets us use boolean logic

extern uint16_t io_discrete_in_channel_count; // extern so modbus.c can check this
extern uint16_t io_hardware_discrete_in_channel_count;

// Define a struct to hold everything required to represent a single discrete input
typedef struct {
	GPIO_PinState (*read_func)(void* context); // Pointer to read function, use hardware_discrete_in_read_func(context) for hardware
	void* context;
} IO_Discrete_In_Channel;

// Struct for GPIO config for physical channels
typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
} gpio_config;


#define MAX_IO_DISCRETE_IN 4 // Limit only applies to physical discrete input channels, not I2C

// Adds a new channel to the list
void io_discrete_in_add_channel(GPIO_PinState (*read_func)(void*), void* context);

// Reads the discrete input
GPIO_PinState io_discrete_in_read(uint16_t index);

// Read function for physical discrete input channels, i2c is device dependent
GPIO_PinState hardware_discrete_in_read_func(void* context);

#endif
