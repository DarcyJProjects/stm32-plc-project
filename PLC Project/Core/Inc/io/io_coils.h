// TODO: I2C support
// COILS ARE DIGITAL OUTPUTS
// R/W as output can be set (W), but last state can be read (R)

#ifndef IO_COILS_H
#define IO_COILS_H

#include "stm32g4xx_hal.h"
#include <stdbool.h> // Lets us use boolean logic
#include "io/io_discrete_in.h" // for the gpio_config struct

extern uint16_t io_coil_channel_count; // extern so modbus.c can check this
extern uint16_t io_hardware_coil_channel_count;


// Define a struct to hold everything required to represent a single discrete input
typedef struct {
	void (*write_func)(void* context, uint16_t value);
	void* context;
	GPIO_PinState storedState;
} IO_Coil_Channel;


#define MAX_IO_COILS 32 // max total
#define MAX_IO_COILS_PHYSICAL 4

// Adds a new channel to the list
bool io_coil_add_channel(void (*write_func)(void*, uint16_t), void* context);

// Reads the coil
GPIO_PinState io_coil_read(uint16_t index);

// Read function for physical discrete input channels, i2c is device dependent
GPIO_PinState io_coil_read_func(void* context);

// Writes to the coil
void io_coil_write(uint16_t index, GPIO_PinState state);

void hardware_coil_write_func(void* context, GPIO_PinState state);

#endif
