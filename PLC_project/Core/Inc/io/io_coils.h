#ifndef IO_COILS_H
#define IO_COILS_H

#include "stm32g4xx_hal.h"
#include <stdbool.h> // Lets us use boolean logic

// Creates an enum named IO_Coil_Direction with two possible values
// 0 : IO_COIL_INPUT
// 1 : IO_COIL_OUTPUT
typedef enum {
	IO_COIL_INPUT,
	IO_COIL_OUTPUT,
	IO_COIL_INVALID // invalid return for io_coil_get_direction
} IO_Coil_Direction;


// Define a struct to hold everything required to represent a single IO coil
// port : which GPIO port e.g., GPIOA, GPIOB, GPIOC
// pin : which pin number e.g., GPIO_PIN_13
// direction : whether its an input or output (uses IO_Coil_Direction enum)
typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
	IO_Coil_Direction direction;
} IO_Coil_Channel;


#define MAX_IO_COILS 8

// Adds a new channel (either input or output) to the list
void io_coil_add_channel(GPIO_TypeDef* port, uint16_t pin, IO_Coil_Direction dir);

// Reads an IO channel (if it is an input)
GPIO_PinState io_coil_read(uint16_t index);

// Writes to an IO channel (if it is an output).
void io_coil_write(uint16_t index, GPIO_PinState state);

// Check if a coil is an input or output
IO_Coil_Direction io_coil_get_direction(uint16_t index);

#endif
