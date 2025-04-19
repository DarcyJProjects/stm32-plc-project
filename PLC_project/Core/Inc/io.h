#ifndef IO_H
#define IO_H

#include "stm32g4xx_hal.h"
#include <stdbool.h> // Lets us use boolean logic

// Creates an enum named IO_Direction with two possible values
// 0 : IO_INPUT
// 1 : IO_OUTPUT
typedef enum {
	IO_INPUT,
	IO_OUTPUT
} IO_Direction;


// Define a struct to hold everything required to represent a single IO channel
// port : which GPIO port e.g., GPIOA, GPIOB, GPIOC
// pin : which pin number e.g., GPIO_PIN_13
// direction : whether its an input or output (uses IO_Direction enum)
// state : stores the latest input reading or output value
typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
	IO_Direction direction;
} IO_Channel;



#define MAX_IO_CHANNELS 16 // Max 16 digital IOs
extern IO_Channel io_channel[MAX_IO_CHANNELS]; // defined in source, but declared here to be used

// Adds a new channel (either input or output) to the list
void io_add_channel(GPIO_TypeDef* port, uint16_t pin, IO_Direction dir);

// Reads an IO channel (if it is an input)
GPIO_PinState io_read(int index);

// Writes to an IO channel (if it is an output).
void io_write(int index, GPIO_PinState state);

#endif
