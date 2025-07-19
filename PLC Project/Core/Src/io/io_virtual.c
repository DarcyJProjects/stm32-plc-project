#include "io/io_virtual.h"

Virtual_Coil_Channel virtual_coil_channels[MAX_VIRTUAL_COILS];
uint16_t virtual_coil_channel_count = 0;

Virtual_Holding_Reg_Channel virtual_holding_reg_channels[MAX_VIRTUAL_HOLDING_REG];
uint16_t virtual_holding_reg_channel_count = 0;


bool io_virtual_add(VirtualRegisterType type) {
	switch (type) {
		case VIR_COIL: {
			if (virtual_coil_channel_count == MAX_VIRTUAL_COILS) {
				return false;
			}
			virtual_coil_channels[virtual_coil_channel_count].storedValue = 0;
			virtual_coil_channel_count++;
			break;
		}
		case VIR_HOLDING: {
			if (virtual_holding_reg_channel_count == MAX_VIRTUAL_HOLDING_REG) {
				return false;
			}
			virtual_holding_reg_channels[virtual_holding_reg_channel_count].storedValue = 0;
			virtual_holding_reg_channel_count++;
			break;
		}
		default: {
			return false;
		}
	}

	// Save virtual registers to EEPROM (managed by automation as virtual registers are written after rules at dynamic address)
	if (!automation_save_rules()) {
		return false;
	}

	return true;
}


bool io_virtual_get_count(VirtualRegisterType type, uint16_t* count) {
	if (!count) return false;

	switch (type) {
		case VIR_COIL: {
			*count = virtual_coil_channel_count;
			return true;
		}
		case VIR_HOLDING: {
			*count = virtual_holding_reg_channel_count;
			return true;
		}
		default: {
			return false;
		}
	}
}

bool io_virtual_read(VirtualRegisterType type, uint16_t index, uint16_t* value) {
	if (!value) return false;

	switch (type) {
		case VIR_COIL: {
			if (index >= virtual_coil_channel_count) {
				return false;
			}

			*value = (uint16_t)(virtual_coil_channels[index].storedValue);
			return true;
		}
		case VIR_HOLDING: {
			if (index >= virtual_holding_reg_channel_count) {
				return false;
			}
			*value = virtual_holding_reg_channels[index].storedValue;
			return true;
		}
		default: {
			return false;
		}
	}
}

bool io_virtual_write(VirtualRegisterType type, uint16_t index, uint16_t value) {
	switch (type) {
		case VIR_COIL: {
			if (index >= virtual_coil_channel_count) {
				return false;
			}

			virtual_coil_channels[index].storedValue = (value != 0) ? 1 : 0;
			return true;
		}
		case VIR_HOLDING: {
			if (index >= virtual_holding_reg_channel_count) {
				return false;
			}

			virtual_holding_reg_channels[index].storedValue = value;
			return true;
		}
		default: {
			return false;
		}
	}
}

bool io_virtual_save(uint16_t baseAddress) {
	// Buffer to hold all data for CRC16
	uint8_t buffer[
				   sizeof(virtual_coil_channel_count) +
				   sizeof(virtual_coil_channels) +
				   sizeof(virtual_holding_reg_channel_count) +
				   sizeof(virtual_holding_reg_channels)
    ];

	uint16_t offset = 0;

	// Build buffer

	// Virtual coil count
	memcpy(&buffer[offset], &virtual_coil_channel_count, sizeof(virtual_coil_channel_count));
	offset += sizeof(virtual_coil_channel_count);

	// Virtual coil data
	memcpy(&buffer[offset], virtual_coil_channels, virtual_coil_channel_count * sizeof(Virtual_Coil_Channel));
	offset += virtual_coil_channel_count * sizeof(Virtual_Coil_Channel);

	// Virtual holding register count
	memcpy(&buffer[offset], &virtual_holding_reg_channel_count, sizeof(virtual_holding_reg_channel_count));
	offset += sizeof(virtual_holding_reg_channel_count);

	// Virtual holding register data
	memcpy(&buffer[offset], virtual_holding_reg_channels, virtual_holding_reg_channel_count * sizeof(Virtual_Holding_Reg_Channel));
	offset += virtual_holding_reg_channel_count * sizeof(Virtual_Holding_Reg_Channel);


	// Write buffer to EEPROM
	if (!EEPROM_WriteBlock(baseAddress, buffer, offset)) return false;
	baseAddress += offset;

	// Compute and write CRC16
	uint16_t crc = modbus_crc16(buffer, offset);
	if (!EEPROM_WriteBlock(baseAddress, &crc, sizeof(crc))) return false;

	baseAddress += sizeof(crc);

	// Now save physical holding register modes
	return io_holding_reg_type_save(baseAddress);
}

bool io_virtual_load(uint16_t baseAddress) {
	// Buffer for reading max possible sizes
	uint8_t buffer[
				   sizeof(virtual_coil_channel_count) +
				   sizeof(virtual_coil_channels) +
				   sizeof(virtual_holding_reg_channel_count) +
				   sizeof(virtual_holding_reg_channels)
	];

	uint16_t offset = 0;

	uint16_t dataLen = sizeof(virtual_coil_channel_count) +
						MAX_VIRTUAL_COILS * sizeof(Virtual_Coil_Channel) +
						sizeof(virtual_holding_reg_channel_count) +
						MAX_VIRTUAL_HOLDING_REG * sizeof(Virtual_Holding_Reg_Channel);

	// Read max data (up to just before CRC)
	if (!EEPROM_LoadBlock(baseAddress, buffer, dataLen)) return false;

	// Read counts first then data
	uint16_t temp_coil_count;
	memcpy(&temp_coil_count, &buffer[offset], sizeof(temp_coil_count));
	if (temp_coil_count > MAX_VIRTUAL_COILS) return false;
	offset += sizeof(temp_coil_count);

	Virtual_Coil_Channel temp_coils[MAX_VIRTUAL_COILS];
	memcpy(temp_coils, &buffer[offset], temp_coil_count * sizeof(Virtual_Coil_Channel));
	offset += temp_coil_count * sizeof(Virtual_Coil_Channel);

	uint16_t temp_holding_count;
	memcpy(&temp_holding_count, &buffer[offset], sizeof(temp_holding_count));
	if (temp_holding_count > MAX_VIRTUAL_HOLDING_REG) return false;
	offset += sizeof(temp_holding_count);

	Virtual_Holding_Reg_Channel temp_holding[MAX_VIRTUAL_HOLDING_REG];
	memcpy(temp_holding, &buffer[offset], temp_holding_count * sizeof(Virtual_Holding_Reg_Channel));
	offset += temp_holding_count * sizeof(Virtual_Holding_Reg_Channel);

	// Read CRC16
	uint16_t stored_crc;
	if (!EEPROM_LoadBlock(baseAddress + offset, &stored_crc, sizeof(stored_crc))) return false;

	// Check CRC16
	uint16_t computed_crc = modbus_crc16(buffer, offset);
	if (computed_crc != stored_crc) return false;

	// All checks passed — commit to globals
	virtual_coil_channel_count = temp_coil_count;
	memcpy(virtual_coil_channels, temp_coils, temp_coil_count * sizeof(Virtual_Coil_Channel));

	virtual_holding_reg_channel_count = temp_holding_count;
	memcpy(virtual_holding_reg_channels, temp_holding, temp_holding_count * sizeof(Virtual_Holding_Reg_Channel));

	return true;
}

// WARNING. Deletes all virtual registers in memory and on EEPROM
bool io_virtual_clear(void) {
	virtual_coil_channel_count = 0;
	virtual_holding_reg_channel_count = 0;

	memset(virtual_coil_channels, 0, sizeof(virtual_coil_channels));
	memset(virtual_holding_reg_channels, 0, sizeof(virtual_holding_reg_channels));

	// Save virtual registers to EEPROM (managed by automation as virtual registers are written after rules at dynamic address)
	return automation_save_rules();
}

bool io_virtual_factory_reset(uint16_t baseAddress) {
	virtual_coil_channel_count = 0;
	virtual_holding_reg_channel_count = 0;

	memset(virtual_coil_channels, 0, sizeof(virtual_coil_channels));
	memset(virtual_holding_reg_channels, 0, sizeof(virtual_holding_reg_channels));

	// Save virtual registers to EEPROM
	io_virtual_save(baseAddress);
}
