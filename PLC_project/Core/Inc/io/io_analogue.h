#ifndef IO_ANALOGUE_H
#define IO_ANALOGUE_H

#include "stm32g4xx_hal.h"

// Only include if enabled in the project config
#ifdef HAL_ADC_MODULE_ENABLED
#include "stm32g4xx_hal_adc.h"
#endif

#ifdef HAL_DAC_MODULE_ENABLED
#include "stm32g4xx_hal_dac.h"
#endif

#include "stm32g4xx_hal_gpio.h"
#include <stdbool.h> // Lets us use boolean logic

// Creates an enum named IO_Direction with two possible values
// 0 : IO_INPUT
// 1 : IO_OUTPUT
typedef enum {
	IO_ANALOGUE_INPUT,
	IO_ANALOGUE_OUTPUT
} IO_Analogue_Direction;


// Define a struct to represent an analogue I/O channel
typedef struct {
	void* handle; // ADC/DAC handle
	uint32_t channel; // ADC/DAC channel number
	IO_Analogue_Direction direction; // Input/Output direction
} IO_Analogue_Channel;



#define MAX_IO_ANALOGUE 8 // Max 8 analogue IOs

// Adds a new channel (either input or output) to the list
void io_analogue_add_channel(void* handle, uint32_t channel, IO_Analogue_Direction dir);

// Reads from an ADC
uint32_t io_analogue_read(int index);

// Writes to a DAC
void io_analogue_write(int index, uint32_t value);

#endif
