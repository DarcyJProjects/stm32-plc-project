// Sources:
// https://controllerstech.com/stm32-reads-holding-and-input-registers/
// https://stackoverflow.com/questions/19347685/calculating-modbus-rtu-crc-16
// https://www.modbustools.com/modbus.html#Function01
// https://dev.to/madhav_baby_giraffe/bit-packing-the-secret-to-optimizing-data-storage-and-transmission-m70

#include "modbus/modbus.h"

#define MODBUS_MAX_FRAME_SIZE 256

static uint16_t modbus_holding_registers[MODBUS_REGISTER_COUNT] = {0}; // will be moved to io_registers once implemented


// FUNCTIONS
static uint16_t modbus_crc16(uint8_t* frame, uint16_t len);
static void send_response(uint8_t* frame, uint16_t len);
static void send_exception(uint8_t address, uint8_t function, uint8_t exception);


// Handle a full received modbus frame
void modbus_handle_frame(uint8_t* frame, uint16_t len) {
	//debug
	//static char debug_msg[256];
	//snprintf(debug_msg, sizeof(debug_msg), "DEBUG: Frame len = %u, First four = 0x%02X 0x%02X 0x%02X 0x%02X\r\n", len, frame[0], frame[1], frame[2], frame[3]);
	//CDC_Transmit_FS((uint8_t*)debug_msg, strlen(debug_msg));

	if (len < 6) return;

	uint8_t address = frame[0];

	uint8_t function = frame[1];

	// Check if the frame is for us
	if (address != MODBUS_SLAVE_ADDRESS) return;

	// Check if the CRC is valid
	uint16_t received_crc = (frame[len - 1] << 8) | frame[len - 2]; // MODBUS sends LSB first (unlike address, function)
	uint16_t calculated_crc = modbus_crc16(frame, len - 2); // Exclude received CRC from CRC calculation
	//static char debug_crc[256];
	//snprintf(debug_crc, sizeof(debug_crc), "DEBUG: Received CRC = 0x%02X, Calculated CRC = 0x%02X\r\n", received_crc, calculated_crc);
	//CDC_Transmit_FS((uint8_t*)debug_crc, strlen(debug_crc));

	if (received_crc != calculated_crc) {
		// Invalid CRC, No exception for a CRC failure
		return;
	}

	switch (function) {
		case MODBUS_FUNC_READ_COILS: {
			uint16_t startAddress = (frame[2] << 8) | frame[3];
			uint16_t coilCount = (frame[4] << 8) | frame[5];

			// Check if coilCount value is legal for modbus specs
			if (coilCount == 0 || coilCount > MAX_IO_COILS) {
				send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			uint16_t endAddress = startAddress + coilCount - 1; // get the ending coil

			// Check if endAddress is outside the stored registers
			if (endAddress >= io_coil_channel_count) { // io_coil_channel count external from io_coils
				send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR);
				return;
			}

			// Create the response frame
			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			responseData[0] = MODBUS_SLAVE_ADDRESS; // the address of us
			responseData[1] = function;
			responseData[2] = (coilCount + 7) / 8; // round up

			uint16_t responseLen = 3; // 3 bytes currently stored

			// Coil states are to be packed into bytes, one bit per coil, starting with LSB first
			// E.g., if coil states are 1, 0, 0, 1, 0 for these 5 example coils,
			//		 we store the states as: 00001001 (1 byte) - not 1 byte per bit as thats not efficient (note coil 0 is LSB, then filled towards MSB)
			uint8_t currentByte = 0;
			uint8_t bitIndex = 0;

			// Iterate over each coil and add the its value
			for (int i = 0; i < coilCount; i++) {
				GPIO_PinState coilValue = io_coil_read(startAddress);

				if (coilValue == GPIO_PIN_SET) {
					// If coil is SET, set the bit at the bitIndex in the currentByte
					currentByte |= (1 << bitIndex);
				}

				bitIndex++; // Move to next bit

				if (bitIndex == 8 || i == coilCount - 1) {
					// Once filled 8 bits (1 byte), or at last coil, store the currentByte and reset variables
					responseData[responseLen++] = currentByte;
					currentByte = 0;
					bitIndex = 0;
				}

				startAddress++;
			}

			send_response(responseData, responseLen);
			break;
		}

		case MODBUS_FUNC_READ_HOLDING_REGISTERS: {
			// Combine two bytes to get the 16 byte address
			// Example: frame[2] = 00010110, frame[3] = 10110100 (arbitrary)
			// frame[2] << 8 = 00010110 << 8 = 0001011000000000 (16 bit now)
			// 000101100000000 | frame[3] = 000101100000000 | 10110100 = 0001011010110100 (combines them)
			uint16_t startAddress = (frame[2] << 8) | frame[3]; // combines frame[2] and frame[3] to get 16 bit address
			uint16_t regCount = (frame[4] << 8) | frame[5]; // combines frame[4] and frame[5] to get 16 bit number of registers

			// Check if regCount value is legal for modbus specs
			if (regCount == 0 || regCount > MODBUS_REGISTER_COUNT) {
				send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			uint16_t endAddress = startAddress + regCount - 1; // get the ending register

			// Check if endAddress is outside the stored registers
			if (endAddress >= MODBUS_REGISTER_COUNT) {
				send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR);
				return;
			}

			// Create the response frame
			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			responseData[0] = MODBUS_SLAVE_ADDRESS; // the address of us
			responseData[1] = function;
			responseData[2] = regCount * 2; // 16 bit register * 2 = number of bytes (16 bit = 2 bytes)

			uint16_t responseLen = 3; // 3 bytes currently stored

			// Iterate over each register and add the register value
			for (int i = 0; i < regCount; i++) {
				responseData[responseLen++] = (modbus_holding_registers[startAddress] >> 8) & 0xFF; // Extract the higher byte
				responseData[responseLen++] = (modbus_holding_registers[startAddress]) & 0xFF; // Extract the lower byte
				startAddress++;
			}

			// DEBUG
			//static char debug_msg[256];
			//snprintf(debug_msg, sizeof(debug_msg), "DEBUG: Frame len = %u, First four = 0x%02X 0x%02X 0x%02X 0x%02X\r\n", len, frame[0], frame[1], frame[2], frame[3]);
			//CDC_Transmit_FS((uint8_t*)debug_msg, strlen(debug_msg));

			send_response(responseData, responseLen);
			break;
		}

		case MODBUS_FUNC_WRITE_SINGLE_COIL: {
			uint16_t coilAddress = (frame[2] << 8) | frame[3];
			uint16_t writeValue = (frame[4] << 8) | frame[5];

			if (coilAddress >= io_coil_channel_count) { // io_coil_channel count external from io_coils
				send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR);
				return;
			}

			GPIO_PinState writeState = (writeValue == 0xFF00) ? GPIO_PIN_SET : GPIO_PIN_RESET; // 0xFF00 = GPIO_PIN_SET, 0x0000 = GPIO_PIN_RESET

			// Write the GPIO state to the corresponding coil channel in io_coils
			io_coil_write(coilAddress, writeState);

			send_response(frame, 6); // Echo back 6 byes (as per spec) of the original request (to say it was successful)
			break;
		}

		case MODBUS_FUNC_WRITE_SINGLE_REGISTER: {
			uint16_t regAddress = (frame[2] << 8) | frame[3];
			uint16_t writeValue = (frame[4] << 8) | frame[5];

			if (regAddress >= MODBUS_REGISTER_COUNT) {
				send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR);
				return;
			}

			modbus_holding_registers[regAddress] = writeValue; // write the value to the register
			send_response(frame, 6); // Echo back 6 bytes (as per spec) of the original request (to say it was successful)
			break;
		}

		case MODBUS_FUNC_WRITE_MULTIPLE_COILS: {
			uint16_t startAddress = (frame[2] << 8) | frame[3];
			uint16_t coilCount = (frame[4] << 8) | frame[5];
			uint8_t byteCount = frame[6];
			uint16_t expectedBytes = (coilCount + 7) / 8;

			if (startAddress + coilCount > io_coil_channel_count) { // io_coil_channel count external from io_coils
				send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR);
				return;
			}

			if (expectedBytes != byteCount) { // not enough values provided to write to all the coils
				send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			// Index where coil values start in the frame
			uint16_t frameIndex = 7;

			// Iterate over each coil and set the coil value
			for (uint16_t i = 0; i < coilCount; i++) {
				uint16_t coilAddress = startAddress + i; // start at the startAddress and continue

				uint16_t byte_index = i / 8;
				uint16_t bit_index = i % 8;

				uint8_t writeValue = (frame[frameIndex + byte_index] >> bit_index) & 0x01;

				GPIO_PinState writeState = (writeValue) ? GPIO_PIN_SET : GPIO_PIN_RESET; // 1 = GPIO_PIN_SET, 0 = GPIO_PIN_RESET

				// Write the GPIO state to the corresponding coil channel in io_coils
				io_coil_write(coilAddress, writeState);
			}

			send_response(frame, 6); // Echo back 6 bytes (as per spec) of the original request (to say it was successful)
			break;
		}

		case MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS: {
			uint16_t startAddress = (frame[2] << 8) | frame[3];
			uint16_t regCount = (frame[4] << 8) | frame[5];
			uint8_t byteCount = frame[6];

			if (startAddress + regCount > MODBUS_REGISTER_COUNT) {
				send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR);
				return;
			}

			if (byteCount != regCount * 2) { // not enough values provided to write to all the registers
				send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			// Index where register values start in the frame
			uint16_t frameIndex = 7;

			// Iterate over each register and add the register value
			for (int i = 0; i < regCount; i++) {
				uint16_t regAddress = startAddress + i; // start at the startAddress and continue
				uint16_t writeValue = (frame[frameIndex] << 8) | frame[frameIndex + 1]; // MSB received first, so put MSB last, LSB first now.

				modbus_holding_registers[regAddress] = writeValue; // write the value to the register

				frameIndex += 2; // Move to the next register value in frame (2 bytes per register so +=2)
			}

			send_response(frame, 6); // Echo back 6 bytes (as per spec) of the original request (to say it was successful)
			break;
		}

		default:
			send_exception(address, function, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
			break;
	}
}

/*void modbus_handle_frame(uint8_t* frame, uint16_t len) {
    static char debug_msg[64];
    snprintf(debug_msg, sizeof(debug_msg), "DEBUG: Frame len = %u, first byte = 0x%02X\r\n", len, frame[0]);
    CDC_Transmit_FS((uint8_t*)debug_msg, strlen(debug_msg));
    //HAL_Delay(10);
}*/

// Returns a pointer to the holding register array
uint16_t* modbus_get_holding_registers(void) {
	return modbus_holding_registers;
}

// Calculate CRC16
// Source: https://stackoverflow.com/questions/19347685/calculating-modbus-rtu-crc-16
static uint16_t modbus_crc16(uint8_t* frame, uint16_t len) {
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
static void send_response(uint8_t* frame, uint16_t len) {
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
static void send_exception(uint8_t address, uint8_t function, uint8_t exception) {
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
