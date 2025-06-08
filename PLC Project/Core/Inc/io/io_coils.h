// COILS ARE DIGITAL OUTPUTS
// R/W as output can be set (W), but last state can be read (R)

#ifndef IO_COILS_H
#define IO_COILS_H

#include "stm32g4xx_hal.h"
#include <stdbool.h> // Lets us use boolean logic

extern uint16_t io_coil_channel_count; // extern so modbus.c can check this


// Define a struct to hold everything required to represent a single IO coil
// port : which GPIO port e.g., GPIOA, GPIOB, GPIOC
// pin : which pin number e.g., GPIO_PIN_13
// storedState : what the last set state is e.g., GPIO_PIN_SET
typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
	GPIO_PinState storedState;
} IO_Coil_Channel;


#define MAX_IO_COILS 4

// Adds a new channel to the list
void io_coil_add_channel(GPIO_TypeDef* port, uint16_t pin);

// Reads the coil
GPIO_PinState io_coil_read(uint16_t index);

// Writes to the coil
void io_coil_write(uint16_t index, GPIO_PinState state);

#endif
