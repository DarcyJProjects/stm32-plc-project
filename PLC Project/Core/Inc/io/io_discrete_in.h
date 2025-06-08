// DISCRETE INPUTS ARE DIGITAL INPUTS
// R as input can ONLY be read (R)

#ifndef IO_DISCRETE_IN_H
#define IO_DISCRETE_IN_H

#include "stm32g4xx_hal.h"
#include <stdbool.h> // Lets us use boolean logic

extern uint16_t io_discrete_in_channel_count; // extern so modbus.c can check this


// Define a struct to hold everything required to represent a single discrete input
// port : which GPIO port e.g., GPIOA, GPIOB, GPIOC
// pin : which pin number e.g., GPIO_PIN_13
typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
} IO_Discrete_In_Channel;


#define MAX_IO_DISCRETE_IN 4

// Adds a new channel to the list
void io_discrete_in_add_channel(GPIO_TypeDef* port, uint16_t pin);

// Reads the discrete input
GPIO_PinState io_discrete_in_read(uint16_t index);

#endif
