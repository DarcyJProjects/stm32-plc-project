#ifndef MODBUS_VENDOR_H
#define MODBUS_VENDOR_H

#include <stdbool.h>
#include <stdint.h>

// This file is for extended custom functions outside the Modbus specifications.
// For board specific functions, like add rule 0x65.

// FUNCTION CODES
#define MODBUS_VENDOR_FUNC_ADD_RULE 		0x65
#define MODBUS_VENDOR_FUNC_GET_RULE_COUNT	0x66
#define MODBUS_VENDOR_FUNC_GET_RULE			0x67
#define MODBUS_VENDOR_FUNC_DEL_RULE			0x68
// ---------------------------------------- gap to 0x75 for extra functionality in future
#define MODBUS_VENDOR_FUNC_ADD_VIRTUAL_REG	0x75
#define MODBUS_VENDOR_FUNC_DEL_VIRTUAL_REG	0x76

// Handle a full received modbus frame
void modbus_vendor_handle_frame(uint8_t* frame, uint16_t len);


#endif
