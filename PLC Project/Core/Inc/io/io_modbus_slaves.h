#ifndef IO_MODBUS_SLAVES_H
#define IO_MODBUS_SLAVES_H

#include "stm32g4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_MODBUS_SLAVES 4
#define MODBUS_SLAVE_POLLING_MS 500

extern uint16_t io_modbus_slave_channel_count;

typedef enum {
	MODBUS_REGISTER_COIL,
	MODBUS_REGISTER_DISCRETE_INPUT,
	MODBUS_REGISTER_HOLDING,
	MODBUS_REGISTER_INPUT
} Modbus_Register_Type;

// Define a struct to represent a modbus channel
typedef struct {
	uint8_t slave_address;
	Modbus_Register_Type type;
	uint16_t register_address;
	uint16_t buffer;
	uint32_t last_updated_ms;
} Modbus_Slave_Channel;


// Adds a new channel to the list
void io_modbus_slave_add_channel(uint8_t slave_address, Modbus_Register_Type type, uint16_t register_address);

// Read/write the channels with their read/write functions and context
uint16_t io_modbus_slave_read(uint16_t index);

void io_modbus_slave_poll_all(void);

void io_modbus_slave_write(uint16_t index, uint16_t writeValue);


#endif
