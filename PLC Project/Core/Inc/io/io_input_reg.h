// INPUT REGISTERS ARE ANALOGUE INPUTS
// R as input can ONLY be read (R)

#ifndef IO_INPUT_REG_H
#define IO_INPUT_REG_H

#include "stm32g4xx_hal.h"

extern uint16_t io_input_reg_channel_count; // count for all channels or any type, extern so modbus.c can check this
extern uint16_t io_input_adc_reg_channel_count; // count for only physical adc channels for max channel enforcement

// Only include if enabled in the project config
#ifdef HAL_ADC_MODULE_ENABLED
#include "stm32g4xx_hal_adc.h"
#endif

#include "stm32g4xx_hal_gpio.h"
#include <stdbool.h> // Lets us use boolean logic

// Enumeration to define voltage or current mode (only for physical channels, so default to voltage for all).
typedef enum {
	IO_INPUT_REG_VOLTAGE,
	IO_INPUT_REG_CURRENT
} IO_Input_Reg_Mode;


// Define a struct to represent an input register channel
typedef struct {
	uint16_t (*read_func)(void* context, IO_Input_Reg_Mode mode); // Function to read the particular channel device, that returns a value
	void* context; // Pointer passed to that function (e.g., ADC handle or I2C struct)
	IO_Input_Reg_Mode mode; // Voltage or current mode (default to voltage)
} IO_Input_Reg_Channel;

// Struct required for writing physical channel types to EEPROM
typedef struct {
	uint16_t index;
	IO_Input_Reg_Mode mode;
} IO_Input_Reg_Type_Record;

#define MAX_IO_INPUT_REG 32
#define MAX_IO_INPUT_REG_PHYSICAL 4 // Limit only applies to physically exposed MCU ADC inputs

// Adds a new channel to the list
void io_input_reg_add_channel(uint16_t (*read_func)(void*, IO_Input_Reg_Mode), void* context, IO_Input_Reg_Mode mode);

// Read a channel with its read function and context
uint16_t io_input_reg_read(uint16_t index);

// Read function for physical adc channels, i2c is device dependent
uint16_t adc_read_func(uint32_t channel, IO_Input_Reg_Mode mode);

bool io_input_reg_set_mode(uint16_t index, IO_Input_Reg_Mode mode);

bool io_input_reg_get_mode(uint16_t, IO_Input_Reg_Mode* mode);

bool io_input_reg_type_save(uint16_t baseAddress);

bool io_input_reg_type_clear(bool factoryResetMode);

bool io_input_reg_type_load(uint16_t baseAddress);

#endif
