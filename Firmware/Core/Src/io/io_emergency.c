#include "io/io_emergency.h"
#include "i2c/display.h"
#include "io/io_coils.h"
#include "io/io_holding_reg.h"
#include "modbus/modbus_util.h"
#include "i2c/eeprom.h"
#include <string.h>
#include "automation/automation.h"

static bool defined = false;
static uint16_t channel;
static bool latched = false;
static Emergency_Stop_Input_Mode inputMode;

bool emergencyStop_setInput(uint16_t index, Emergency_Stop_Input_Mode mode) {
	if (!io_discrete_in_check_channel(index)) return false;

	// TODO: ADD MODE FOR NC OR NO

	channel = index;
	inputMode = mode;
	defined = true;

	// Save to EEPROM
	automation_save_rules();

	return true;
}

bool emergencyStop_isDefined(void) {
	return defined;
}

uint16_t emergencyStop_getChannel(void) {
	return channel;
}

Emergency_Stop_Input_Mode emergencyStop_getInputMode(void) {
	return inputMode;
}

bool emergencyStop_check(void) {
	if (!defined) return false;
	if (latched) return true;

	if ((io_discrete_in_read(channel) && inputMode == EMERGENCY_STOP_NO) || (!io_discrete_in_read(channel) && inputMode == EMERGENCY_STOP_NC)) {
		// Emergency!
		latched = true;

		display_EmergencyStop();

		// Set all coils to OFF
		io_coils_emergencystop();

		// Set all holding registers to 0
		io_holding_reg_emergencystop();

		return true;
	}

	return false;
}

void emergencyStop_reset(void) {
	latched = false;
}

bool emergencyStop_save(uint16_t baseAddress) {
	// Buffer to hold all data for CRC16
	uint8_t buffer[
				   sizeof(bool) + // defined
				   sizeof(uint16_t) + // channel
				   sizeof(Emergency_Stop_Input_Mode) // input mode
	];

	uint16_t offset = 0;

	// Build buffer

	// defined
	memcpy(&buffer[offset], &defined, sizeof(defined));
	offset += sizeof(defined);

	// channel
	memcpy(&buffer[offset], &channel, sizeof(channel));
	offset += sizeof(channel);

	// input mode
	memcpy(&buffer[offset], &inputMode, sizeof(inputMode));
	offset += sizeof(inputMode);

	// Write buffer to EEPROM
	if (!EEPROM_WriteBlock(baseAddress, buffer, offset)) return false;
	baseAddress += offset;

	// Compute and write CRC16
	uint16_t crc = modbus_crc16(buffer, offset);
	if (!EEPROM_WriteBlock(baseAddress, &crc, sizeof(crc))) return false;

	// Calculate base address in case of requiring pass over
	baseAddress += sizeof(crc);

	return true; // no pass over required
}

bool emergencyStop_clear(bool factoryResetMode) {
	defined = false;

	if (!factoryResetMode) {
		return automation_save_rules();
		// only saves when not in factory reset mode (saving is already triggered in factory reset mode externally)
	}

	return true;
}

bool emergencyStop_load(uint16_t baseAddress) {
	// Buffer for reading max possible amount
	uint16_t dataLen = sizeof(bool) + // defined
			   	   	   sizeof(uint16_t) + // channel
					   sizeof(Emergency_Stop_Input_Mode); // input mode

	uint8_t buffer[dataLen];

	uint16_t offset = 0;

	// read max data (up to just before CRC)
	if (!EEPROM_LoadBlock(baseAddress, buffer, dataLen)) {
		return false;
	}

	// Read defined
	bool temp_defined;
	memcpy(&temp_defined, &buffer[offset], sizeof(temp_defined));
	offset += sizeof(temp_defined);

	// Read channel
	uint16_t temp_channel;
	memcpy(&temp_channel, &buffer[offset], sizeof(temp_channel));
	offset += sizeof(temp_channel);

	// Read input mode
	Emergency_Stop_Input_Mode temp_inputMode;
	memcpy(&temp_inputMode, &buffer[offset], sizeof(temp_inputMode));
	offset += sizeof(temp_inputMode);

	// Read CRC16
	uint16_t stored_crc;
	if (!EEPROM_LoadBlock(baseAddress + offset, &stored_crc, sizeof(stored_crc))) {
		return false;
	}

	// Check CRC16
	uint16_t computed_crc = modbus_crc16(buffer, offset);
	if (computed_crc != stored_crc) {
		defined = false;
		return false;
	}

	// All checks passed - commit to channels
	defined = temp_defined;
	if (defined) {
		channel = temp_channel;
		inputMode = temp_inputMode;
	}

	return true;
}
