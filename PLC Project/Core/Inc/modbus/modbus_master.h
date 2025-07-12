#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <stdint.h>
#include "rs485/rs485.h"
#include "usb_serial.h"
#include "io/io_modbus_slaves.h"

#define MODBUS_MASTER_REQUEST_TIMEOUT 200 // ms

typedef struct {
	bool active;
	uint8_t slave_address;
	Modbus_Register_Type type;
	uint16_t register_address;
	uint32_t timestamp_ms;
	uint16_t* destination;
} Modbus_Master_Request;


// Handle a full received modbus frame
void modbus_master_handle_frame(uint8_t* frame, uint16_t len);

// Request handler
bool modbus_master_request_read(uint8_t slave_address, Modbus_Register_Type type, uint16_t reg_address, uint16_t* dest);

void modbus_master_poll_timeout(void);

bool modbus_master_is_busy(void);

#endif
