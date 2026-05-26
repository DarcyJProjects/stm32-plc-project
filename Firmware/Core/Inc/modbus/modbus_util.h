#ifndef MODBUS_UTIL_H
#define MODBUS_UTIL_H

#include <stdint.h>
#include "rs485/rs485.h"
#include "usb_serial.h"
#include "stm32g4xx_hal.h"

#define MODBUS_MAX_FRAME_SIZE 256

// FUNCTION CODES
#define MODBUS_FUNC_READ_COILS 						0x01
#define MODBUS_FUNC_READ_DISCRETE_INPUTS 			0x02
#define MODBUS_FUNC_READ_HOLDING_REGISTERS			0x03
#define MODBUS_FUNC_READ_INPUT_REGISTERS 			0x04
#define MODBUS_FUNC_WRITE_SINGLE_COIL 				0x05
#define MODBUS_FUNC_WRITE_SINGLE_REGISTER 			0x06
#define MODBUS_FUNC_WRITE_MULTIPLE_COILS 			0x0F
#define MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS 		0x10

// ERROR CODES
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION 			0x01
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR 			0x02
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE 		0x03
#define MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE		0x04
#define MODBUS_EXCEPTION_ACKNOWLEDGE				0x05
#define MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY			0x06
#define MODBUS_EXCEPTION_NEG_ACKNOWLEDGE			0x07
#define MODBUS_EXCEPTION_MEMORY_PARITY_ERROR		0x08
#define MODBUS_EXCEPTION_GATEWAY_PATH_UNAVAIL		0x0A
#define MODBUS_EXCEPTION_GATEWAY_TAR_DEV_RESP_FAIL	0x0B

uint16_t modbus_crc16(uint8_t* frame, uint16_t len);

// Communication
void modbus_send_response(uint8_t* frame, uint16_t len);
void modbus_send_exception(uint8_t address, uint8_t function, uint8_t exception);

uint32_t get_ms(void);

#endif
