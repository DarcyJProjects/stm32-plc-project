#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <stdint.h>
#include "rs485/rs485.h"
#include "usb_serial.h"
#include "io/io_coils.h" // required for controlling coils
#include "io/io_discrete_in.h" // required for reading discrete inputs
#include "io/io_holding_reg.h" // required for controlling holding registers
#include "io/io_input_reg.h" // required for reading input registers

// FUNCTIONS

// Setup slave address
void modbus_Setup(uint8_t slaveAddress);

// Handle a full received modbus frame
void modbus_slave_handle_frame(uint8_t* frame, uint16_t len);

uint8_t modbusGetSlaveAddress(void);


#endif
