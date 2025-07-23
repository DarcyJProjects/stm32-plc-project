#include <io/io_input_reg.h>
#include "i2c/eeprom.h"
#include <string.h>
#include "automation/automation.h"
#include "modbus/modbus_util.h"
#include "usb_serial.h"

// Allocates storage for all the channels.
IO_Input_Reg_Channel io_input_reg_channels[MAX_IO_INPUT_REG]; // Array of type channel (struct created in header)
uint16_t io_input_reg_channel_count = 0;
uint16_t io_input_adc_reg_channel_count = 0;
uint8_t is_calibrated = 0;

extern ADC_HandleTypeDef hadc1; // Declare external handle for ADC1

/**
 * @Brief Adds an input register channel to the system
 *
 * This function adds an input register to the system creating a channel which can
 * be used to read from. The channel number is incremented with each channel added.
 *
 * @param read_func: Function that reads the channels device, returning a value. For physical ADC input, use "adc_read_func". For I2C, it is device dependent.
 * @param context: Pointer passed to that read function. For physical ADC input, use "&hadcX" replacing X as necessary.
 */
void io_input_reg_add_channel(uint16_t (*read_func)(void*, IO_Input_Reg_Mode), void* context, IO_Input_Reg_Mode mode) {
	// Check is a physical ADC input channel is being added
	if (read_func == (void*)adc_read_func) {
		// Enforce limit only for physical ADC inputs
		if (io_input_adc_reg_channel_count >= MAX_IO_INPUT_REG_PHYSICAL) {
			return;
		} else {
			io_input_adc_reg_channel_count++; // increase physical ADC channel count
		}
	}

	io_input_reg_channels[io_input_reg_channel_count].read_func = read_func;
	io_input_reg_channels[io_input_reg_channel_count].context = context;
	io_input_reg_channels[io_input_reg_channel_count].mode = mode;

	io_input_reg_channel_count++; // increase overall channel count
}

/**
 * @Brief Reads the last set value from a registered holding register channel of a provided index.
 *
 * This function returns the last set value of a holding register. It reads with the channels set read_function and context.
 *
 * @param index: The index of the channel to read (assigned in order of registration with io_holding_reg_add_channel)
 *
 * @retval The ADC conversion result, or 0 if the channel is invalid.
 */
uint16_t io_input_reg_read(uint16_t index) {
	if (index < io_input_reg_channel_count) {
		return io_input_reg_channels[index].read_func(io_input_reg_channels[index].context, io_input_reg_channels[index].mode);
	}
	return 0;
}


// Read function for physical adc channels, i2c is device dependent
uint16_t adc_read_func(uint32_t channel, IO_Input_Reg_Mode mode) {
// TODO: NEED TO ADD CURRENT MODE!

#ifdef HAL_ADC_MODULE_ENABLED
		//ADC_HandleTypeDef* hadc = (ADC_HandleTypeDef*)context; // cast generic handle to ADC_HandleTypeDef
		ADC_HandleTypeDef* hadc = &hadc1;

		// Stop ADC before reconfiguration
    	HAL_ADC_Stop(&hadc1);

		// Configure the specified channel
		ADC_ChannelConfTypeDef sConfig = {0};
		sConfig.Channel = channel;
		sConfig.Rank = ADC_REGULAR_RANK_1; /* !!! WILL NOT WORK WITHOUT !! */
		sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5; // Better accuracy
		sConfig.SingleDiff = ADC_SINGLE_ENDED; /* !!! WILL NOT WORK WITHOUT !!! */
		HAL_ADC_ConfigChannel(hadc, &sConfig);

		// Start ADC conversion
		HAL_ADC_Start(hadc);

		// Wait until the ADC conversion is done (or a timeout of 100 ms occurs)
		if (HAL_ADC_PollForConversion(hadc, 100) == HAL_OK) {
			uint16_t adcValue = HAL_ADC_GetValue(hadc); // 12 bit 0-4095
			HAL_ADC_Stop(hadc);

			switch (mode) {
				case IO_INPUT_REG_VOLTAGE: {
					// direct linear mapping to 16 bit
					return (adcValue * 65535U) / 4095U;
				}

				case IO_INPUT_REG_CURRENT: {
					// define the calibrated ADC range
					const uint16_t adc_min_current = 497; // 4mA through 10R = 40mV -> 10x nsi1200 gain -> 0.4V = 497
					const uint16_t adc_max_current = 2481; // 20mA through 10R = 200mV -> 10x nsi1200 gain -> 2V = 2481

					if (adcValue <= adc_min_current) return 0;
					if (adcValue >= adc_max_current) return 65535; // round to max 16 bit val

					uint16_t range = adc_max_current - adc_min_current;
					return ((adcValue - adc_min_current) * 65535U) / range;
				}

				default:
					return 0;
			}

		}
		HAL_ADC_Stop(hadc);
#endif
	return 0;
}

bool io_input_reg_set_mode(uint16_t index, IO_Input_Reg_Mode mode) {
	if (index < io_input_reg_channel_count) {
		io_input_reg_channels[index].mode = mode;

		// trigger EEPROM save
		return automation_save_rules();
	}
	return false;
}

bool io_input_reg_get_mode(uint16_t index, IO_Input_Reg_Mode* mode) {
	if (index < io_input_reg_channel_count) {
		*mode = io_input_reg_channels[index].mode;
		return true;
	}
	return false;
}

bool io_input_reg_type_save(uint16_t baseAddress) {
	// Count physical ADC input regs
	uint16_t count = 0;
	for (uint16_t i = 0; i < io_input_reg_channel_count; i++) {
		if (io_input_reg_channels[i].read_func == (void*)adc_read_func) {
			count++;
		}
		// ignore other types as they cannot be changed on the fly
	}

	if (count == 0) return true; // nothing to save
	// TODO: WILL NOT PASS ONTO NEXT SAVE (not an issue yet but could be)


	// Buffer to hold all data for CRC16
	uint8_t buffer[
				   sizeof(count) +
				   sizeof(IO_Input_Reg_Type_Record) * count
	];

	uint16_t offset = 0;

	// Build buffer

	// count
	memcpy(&buffer[offset], &count, sizeof(count));
	offset += sizeof(count);

	// input register records
	for (uint16_t i = 0; i < io_input_reg_channel_count; i++) {
		if (io_input_reg_channels[i].read_func == (void*)adc_read_func) {
			IO_Input_Reg_Type_Record newRecord;
			newRecord.index = i;
			newRecord.mode = io_input_reg_channels[i].mode;

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

	// Pass onto emergency stop to save
	baseAddress += sizeof(crc);

	return emergencyStop_save(baseAddress);
}

bool io_input_reg_type_clear(bool factoryResetMode) {
	for (uint16_t i = 0; i < io_input_reg_channel_count; i++) {
		if (io_input_reg_channels[i].read_func == (void*)adc_read_func) {
			io_input_reg_channels[i].mode = IO_INPUT_REG_VOLTAGE; // default is voltage
		}
	}

	if (!factoryResetMode) {
		return automation_save_rules();
		// only saves when not in factory reset mode (saving is already triggered in factory reset mode externally)
	}

	return true;
}

bool io_input_reg_type_load(uint16_t baseAddress) {
	// Buffer for reading max possible amount
	uint16_t dataLen = sizeof(uint16_t) + sizeof(IO_Input_Reg_Type_Record) * MAX_IO_INPUT_REG_PHYSICAL + sizeof(uint16_t);
	uint8_t buffer[dataLen];

	uint16_t offset = 0;

	// read max data (up to just before CRC)
	if (!EEPROM_LoadBlock(baseAddress, buffer, dataLen)) {
		return false;
	}

	// Read count first then data
	uint16_t temp_reg_count;
	memcpy(&temp_reg_count, &buffer[offset], sizeof(temp_reg_count));
	if (temp_reg_count > MAX_IO_INPUT_REG_PHYSICAL) {
		return false;
	}
	offset += sizeof(temp_reg_count);

	IO_Input_Reg_Type_Record temp_records[MAX_IO_INPUT_REG_PHYSICAL];
	memcpy(temp_records, &buffer[offset], temp_reg_count * sizeof(IO_Input_Reg_Type_Record));
	offset += temp_reg_count * sizeof(IO_Input_Reg_Type_Record);

	// Read CRC16
	uint16_t stored_crc;
	if (!EEPROM_LoadBlock(baseAddress + offset, &stored_crc, sizeof(stored_crc))) {
		return false;
	}

	// Check CRC16
	uint16_t computed_crc = modbus_crc16(buffer, offset);
	if (computed_crc != stored_crc) {
		return false;
	}

	// All checks passed - commit to channels
	for (uint16_t i = 0; i < temp_reg_count; i++) {
		uint16_t index = temp_records[i].index;
		if (io_input_reg_channels[index].read_func == (void*)adc_read_func) {
			io_input_reg_channels[index].mode = temp_records[i].mode;
		}
	}

	// Pass onto emergencystop to load
	baseAddress += offset;
	baseAddress += sizeof(stored_crc);

	return emergencyStop_load(baseAddress);
}
