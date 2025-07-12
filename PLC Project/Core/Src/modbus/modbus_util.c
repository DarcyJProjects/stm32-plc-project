#include "modbus/modbus_util.h"

// Calculate CRC16
// Source: https://stackoverflow.com/questions/19347685/calculating-modbus-rtu-crc-16
uint16_t modbus_crc16(uint8_t* frame, uint16_t len) {
	uint16_t crc = 0xFFFF;

	for (uint16_t pos = 0; pos < len; pos++) {
		crc ^= frame[pos]; // XOR byte into LSB of CRC

		for (uint8_t b = 0; b < 8; b++) { // Iterate over each bit
			if ((crc & 0x0001) != 0) { // If LSB is set
				crc >>= 1; // Shift right
				crc ^= 0xA001; // XOR 0xA001
			} else { // Else (LSB is not set)
				crc >>= 1; // Shift right
			}
		}
	}

	return crc;
}

// Send the response over RS485
void modbus_send_response(uint8_t* frame, uint16_t len) {
	// Add CRC
	uint16_t crc = modbus_crc16(frame, len);
	frame[len++] = crc & 0xFF;         // LSB first
	frame[len++] = (crc >> 8) & 0xFF;  // MSB second

	//debug
	//static char debug_msg_response[256];
	//snprintf(debug_msg_response, sizeof(debug_msg_response), "DEBUG: Transmit len = %u, first four = 0x%02X 0x%02X 0x%02X 0x%02X\r\n", len, frame[0], frame[1], frame[2], frame[3]);
	//CDC_Transmit_FS((uint8_t*)debug_msg_response, strlen(debug_msg_response));

	// Transmit over RS485
	RS485_Transmit(frame, len);
}

// Send the exception over RS485
void modbus_send_exception(uint8_t address, uint8_t function, uint8_t exception) {
	// Craft exceptionFrame
	uint8_t exceptionFrame[5];
	exceptionFrame[0] = address;
	exceptionFrame[1] = function | 0x80; // Must OR function with 0x80 to indicate an exception
	exceptionFrame[2] = exception;

	// Add CRC
	uint16_t crc = modbus_crc16(exceptionFrame, 3);
	exceptionFrame[3] = crc & 0xFF;         // LSB first
	exceptionFrame[4] = (crc >> 8) & 0xFF;  // MSB second

	// Transmit over RS485
	RS485_Transmit(exceptionFrame, 5);
}



uint32_t get_ms(void) {
	return HAL_GetTick();
}
