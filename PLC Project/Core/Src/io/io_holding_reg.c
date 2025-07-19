#include <io/io_holding_reg.h>
#include "i2c/eeprom.h"
#include <string.h> // for saving to eeprom, used by memcpy

// Allocates storage for all the channels.
IO_Holding_Reg_Channel io_holding_reg_channels[MAX_IO_HOLDING_REG]; // Array of type channel (struct created in header)
uint16_t io_holding_reg_channel_count = 0;
uint16_t io_holding_adc_reg_channel_count = 0;

extern DAC_HandleTypeDef hdac1; // Declare external handle for DAC1


bool io_holding_reg_add_channel(void (*write_func)(void*, uint16_t, IO_Holding_Reg_Mode), void* context, IO_Holding_Reg_Mode mode) {
	if (io_holding_reg_channel_count == MAX_IO_HOLDING_REG) {
		return false;
	}

	// Check is a physical ADC output channel is being added
	if (write_func == (void*)dac_write_func) {
		// Enforce limit only for physical ADC outputs
		if (io_holding_adc_reg_channel_count >= MAX_IO_HOLDING_REG_PHYSICAL) {
			return false;
		} else {
			io_holding_adc_reg_channel_count++; // increase physical ADC channel count
		}
	}

	io_holding_reg_channels[io_holding_reg_channel_count].write_func = write_func;
	io_holding_reg_channels[io_holding_reg_channel_count].context = context;
	io_holding_reg_channels[io_holding_reg_channel_count].storedValue = 0;
	io_holding_reg_channels[io_holding_reg_channel_count].mode = mode;

	io_holding_reg_channel_count++; // increase overall channel count
	return true;
}



uint16_t io_holding_reg_read(uint16_t index) {
	if (index < io_holding_reg_channel_count) {
		return io_holding_reg_channels[index].storedValue;
	}
	return 0;
}



void io_holding_reg_write(uint16_t index, uint16_t value) {
	if (index < io_holding_reg_channel_count) {
		if (io_holding_reg_channels[index].write_func) {
			io_holding_reg_channels[index].write_func(io_holding_reg_channels[index].context, value, io_holding_reg_channels[index].mode);
		}
		io_holding_reg_channels[index].storedValue = value;
	}
}


void dac_write_func(uint32_t channel, uint16_t value, IO_Holding_Reg_Mode mode) {
#ifdef HAL_DAC_MODULE_ENABLED
		DAC_HandleTypeDef* hdac = &hdac1;

		uint32_t scaledValue;
		switch (mode) {
			case IO_HOLDING_REG_VOLTAGE:
				// Scale modbus 16 bit value to 12 bit DAC range
				scaledValue = (value * 4095U) / 65535U;
				break;
			case IO_HOLDING_REG_CURRENT:
				// 0.634 V for 4mA [0]
				// 3.168 V for 20mA [65535 - 16 bit max]

				// DAC value = V_DAC / 3.3 * 4095 (12 bit DAC)
				// DAC value for 4mA = 0.634/3.3 * 4095 = 787
				// DAC value for 20mA = 3.168/3.3 * 4095 = 3931
				uint16_t dac_min_current_mode = 785;
				uint16_t dac_max_current_mode = 3932;

				uint16_t dac_range_current_mode = dac_max_current_mode - dac_min_current_mode;
				scaledValue = dac_min_current_mode + ((uint32_t)value * dac_range_current_mode) / 65535U;
		}

		// Start DAC conversion (DAC peripheral, DAC channel, data alignment, value)
		HAL_DAC_SetValue(hdac, channel, DAC_ALIGN_12B_R, scaledValue);

		// Enable the DAC channel and apply the value to the pin
		HAL_DAC_Start(hdac, channel);
#endif
}


bool io_holding_reg_set_mode(uint16_t index, IO_Holding_Reg_Mode mode) {
	if (index < io_holding_reg_channel_count) {
		io_holding_reg_channels[index].mode = mode;
		return true;
	}
	return false;
}

bool io_holding_reg_get_mode(uint16_t index, IO_Holding_Reg_Mode* mode) {
	if (index < io_holding_reg_channel_count) {
		*mode = io_holding_reg_channels[index].mode;
		return true;
	}
	return false;
}

bool io_holding_reg_type_save(uint16_t baseAddress) {
	// Count physical DAC holding regs
	uint16_t count = 0;
	for (uint16_t i = 0; i < io_holding_reg_channel_count; i++) {
		if (io_holding_reg_channels[i].write_func == (void*)dac_write_func) {
			count++;
		}
		// ignore other types as they cannot be changed on the fly
	}

	if (count == 0) return true; // nothing to save


	// Buffer to hold all data for CRC16
	uint8_t buffer[
				   sizeof(count) +
				   sizeof(IO_Holding_Reg_Type_Record) * count
    ];

	uint16_t offset = 0;

	// Build buffer

	// count
	memcpy(&buffer[offset], &count, sizeof(count));
	offset += sizeof(count);

	// holding register records
	for (uint16_t i = 0; i < io_holding_reg_channel_count; i++) {
		if (io_holding_reg_channels[i].write_func == (void*)dac_write_func) {
			IO_Holding_Reg_Type_Record newRecord;
			newRecord.index = i;
			newRecord.mode = io_holding_reg_channels[i].mode;

			memcpy(&buffer[offset], &newRecord, sizeof(newRecord));
			offset += sizeof(newRecord);
		}
	}

	// Write buffer to EEPROM
	if (!EEPROM_WriteBlock(baseAddress, buffer, offset)) return false;
	baseAddress += offset;

	// Compute and write CRC16
	uint16_t crc = modbus_crc16(buffer, offset);
	if (!EEPROM_WriteBlock(baseAddress, &crc, sizeof(crc))) return false;

	return true;
}

bool io_holding_reg_type_load(uint16_t baseAddress) {
	// Buffer for reading max possible amount
	uint16_t dataLen = sizeof(MAX_IO_HOLDING_REG_PHYSICAL) + (sizeof(IO_Holding_Reg_Type_Record) * MAX_IO_HOLDING_REG_PHYSICAL);
	uint8_t buffer[dataLen];

	uint16_t offset = 0;

	// read max data (up to just before CRC)
	if (!EEPROM_LoadBlock(baseAddress, buffer, dataLen)) return false;

	// Read count first then data
	uint16_t temp_reg_count;
	memcpy(&temp_reg_count, &buffer[offset], sizeof(temp_reg_count));
	if (temp_reg_count > MAX_IO_HOLDING_REG_PHYSICAL) return false;
	offset += sizeof(temp_reg_count);

	IO_Holding_Reg_Type_Record temp_records[MAX_IO_HOLDING_REG_PHYSICAL];
	memcpy(temp_records, &buffer[offset], temp_reg_count * sizeof(IO_Holding_Reg_Type_Record));
	offset += temp_reg_count * sizeof(IO_Holding_Reg_Type_Record);

	// Read CRC16
	uint16_t stored_crc;
	if (!EEPROM_LoadBlock(baseAddress + offset, &stored_crc, sizeof(stored_crc))) return false;

	// Check CRC16
	uint16_t computed_crc = modbus_crc16(buffer, offset);
	if (computed_crc != stored_crc) return false;

	// All checks passed - commit to channels
	for (uint16_t i = 0; i < temp_reg_count; i++) {
		uint16_t index = temp_records[i].index;
		if (io_holding_reg_channels[index].write_func == (void*)dac_write_func) {
			io_holding_reg_channels[index].mode = temp_records[i].mode;
		}
	}

	return true;
}

bool io_holding_reg_type_clear(bool factoryResetMode) {
	for (uint16_t i = 0; i < io_holding_reg_channel_count; i++) {
		if (io_holding_reg_channels[i].write_func == (void*)dac_write_func) {
			io_holding_reg_channels[i].mode = IO_HOLDING_REG_VOLTAGE; // default is voltage
		}
	}

	if (!factoryResetMode) {
		return automation_save_rules();
		// only saves when not in factory reset mode (saving is already triggered in factory reset mode externally)
	}

	return true;
}
