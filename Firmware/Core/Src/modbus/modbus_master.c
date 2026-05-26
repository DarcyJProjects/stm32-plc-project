#include "modbus/modbus_master.h"
#include "modbus/modbus_util.h"


static Modbus_Master_Request current_request = {0};

static uint8_t get_function_code(Modbus_Register_Type type) {
	switch (type) {
		case MODBUS_REGISTER_COIL: return MODBUS_FUNC_READ_COILS;
		case MODBUS_REGISTER_DISCRETE_INPUT: return MODBUS_FUNC_READ_DISCRETE_INPUTS;
		case MODBUS_REGISTER_HOLDING: return MODBUS_FUNC_READ_HOLDING_REGISTERS;
		case MODBUS_REGISTER_INPUT: return MODBUS_FUNC_READ_INPUT_REGISTERS;
		default: return 0x00;
	}
}

// Handle a full received modbus frame
void modbus_master_handle_frame(uint8_t* frame, uint16_t len) {
	if (!current_request.active) return;

	if (len < 5) return;

	uint8_t address = frame[0];
	uint8_t func = frame[1];

	if (address != current_request.slave_address) return; // throw it out!

	uint16_t received_crc = (frame[len - 1] << 8) | frame[len - 2];
	if (modbus_crc16(frame, len - 2) != received_crc) return;

	uint8_t expected_func = get_function_code(current_request.type);
	if (func != expected_func) return; // throw it out!

	if (current_request.destination == NULL) {
		current_request.active = false;
		return;
	}

	// Only supporting standard 16 bit (2 byte) responses for now
	if (len >= 5 && frame[2] >= 2) {
		uint16_t value = (frame[3] << 8) | frame[4];
		*(current_request.destination) = value;
	}

	current_request.active = false;
}


// Request handler
bool modbus_master_request_read(uint8_t slave_address, Modbus_Register_Type type, uint16_t reg_address, uint16_t* dest)
{
	if (current_request.active) return false;

	uint8_t func = get_function_code(type);

	uint8_t frame[8];
	frame[0] = slave_address;
	frame[1] = func;
	frame[2] = (reg_address >> 8) & 0xFF;
	frame[3] = reg_address & 0xFF;
	frame[4] = 0x00; // High byte of quantity
	frame[5] = 0x01; // request 1 register only (low byte)

	modbus_send_response(frame, 8);

	current_request.active = true;
	current_request.slave_address = slave_address;
	current_request.type = type;
	current_request.register_address = reg_address;
	current_request.destination = dest;
	current_request.timestamp_ms = get_ms();

	return true;
}

void modbus_master_poll_timeout(void) {
	uint32_t now_ms = get_ms();
	if (current_request.active && (now_ms - current_request.timestamp_ms > MODBUS_MASTER_REQUEST_TIMEOUT)) {
		current_request.active = false; // timeout
	}
}


bool modbus_master_is_busy(void) {
	return current_request.active;
}




