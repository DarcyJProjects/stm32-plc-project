#ifndef IO_DIGITAL_H
#define IO_DIGITAL_H

#include "stm32g4xx_hal.h"
#include <stdbool.h> // Lets us use boolean logic

// Creates an enum named IO_Direction with two possible values
// 0 : IO_DIGITAL_INPUT
// 1 : IO_DIGITAL_OUTPUT
typedef enum {
	IO_DIGITAL_INPUT,
	IO_DIGITAL_OUTPUT
} IO_Digital_Direction;


// Define a struct to hold everything required to represent a single IO channel
// port : which GPIO port e.g., GPIOA, GPIOB, GPIOC
// pin : which pin number e.g., GPIO_PIN_13
// direction : whether its an input or output (uses IO_Digital_Direction enum)
typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
	IO_Digital_Direction direction;
} IO_Digital_Channel;



#define MAX_IO_DIGITAL 16 // Max 16 digital IOs

// Adds a new channel (either input or output) to the list
void io_digital_add_channel(GPIO_TypeDef* port, uint16_t pin, IO_Digital_Direction dir);

// Reads an IO channel (if it is an input)
GPIO_PinState io_digital_read(int index);

// Writes to an IO channel (if it is an output).
void io_digital_write(int index, GPIO_PinState state);

#endif
