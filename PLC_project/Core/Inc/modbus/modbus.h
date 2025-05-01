#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>
#include "rs485/rs485.h"
#include "usb_serial.h"

// MODBUS ADDRESS
#define MODBUS_SLAVE_ADDRESS 0x01 // Our slave address

// MODBUS REGISTERS
#define MODBUS_REGISTER_COUNT 32 // Number of registers

// FUNCTION CODES
#define MODBUS_FUNC_READ_HOLDING_REGISTERS 0x03
#define MODBUS_FUNC_WRITE_SINGLE_REGISTER 0x06
#define MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS 0x10

// ERROR CODES
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION 0x01
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR 0x02
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE 0x03


// FUNCTIONS

// Handle a full received modbus frame
void modbus_handle_frame(uint8_t* frame, uint16_t len);

// Returns a pointer to the holding register array
uint16_t* modbus_get_holding_registers(void);

#endif
