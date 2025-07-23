#ifndef MODBUS_VENDOR_H
#define MODBUS_VENDOR_H

#include <stdbool.h>
#include <stdint.h>

// This file is for extended custom functions outside the Modbus specifications.
// For board specific functions, like add rule 0x65.

/*
Range		Hexadecimal		Notes
65–72		0x41–0x48		Vendor-specific (A–H)
100–110		0x64–0x6E		Vendor-specific
111–119		0x6F–0x77		Vendor-specific
>=120		0x78–0x7F		Not defined or reserved for future use
*/


// FUNCTION CODES
#define MODBUS_VENDOR_FUNC_ADD_RULE 				0x65
#define MODBUS_VENDOR_FUNC_GET_RULE_COUNT			0x66
#define MODBUS_VENDOR_FUNC_GET_RULE					0x67
#define MODBUS_VENDOR_FUNC_DEL_RULE					0x68
#define MODBUS_VENDOR_FUNC_ADD_VIRTUAL_REG			0x69
#define MODBUS_VENDOR_FUNC_READ_VIRTUAL_REG			0x6A
#define MODBUS_VENDOR_FUNC_WRITE_VIRTUAL_REG		0x6B
#define MODBUS_VENDOR_FUNC_GET_VIRTUAL_REG_COUNT	0x6C
#define MODBUS_VENDOR_FUNC_CLEAR_VIRTUAL_REG		0x6D // not front-end UI accessible due to rule dependencies on vrs

#define MODBUS_VENDOR_FUNC_SET_RTC					0x6E

#define MODBUS_VENDOR_FUNC_SET_HOLDING_REG_MODE		0x6F
#define MODBUS_VENDOR_FUNC_SET_INPUT_REG_MODE		0x70
#define MODBUS_VENDOR_FUNC_GET_REG_MODE				0x71

#define MODBUS_VENDOR_FUNC_SET_EMERGENCY_STOP		0x72


// Handle a full received modbus frame
void modbus_vendor_handle_frame(uint8_t* frame, uint16_t len);


#endif
