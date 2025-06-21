#include "modbus/modbus_vendor.h"
#include "modbus/modbus.h"
#include "automation/automation.h"
#include "io/io_virtual.h"
#include "rtc/rtc_ds3231.h"

#include "io/io_coils.h"
#include "io/io_holding_reg.h"

// Handle a full received modbus frame
void modbus_vendor_handle_frame(uint8_t* frame, uint16_t len) {
	uint8_t function = frame[1];
	uint8_t slave_address = modbusGetSlaveAddress();

	switch (function) {
		case MODBUS_VENDOR_FUNC_ADD_RULE: {

			// Check if the frame length is correct: 18 + 4 (slave id (1), function (1), crc (2))
			if (len != 22) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			// Extract fields from the 18-byte payload
			uint8_t input_type1Raw = frame[2];
			uint16_t input_reg1 = (frame[3] << 8) | frame[4];
			uint8_t op1Raw = frame[5];
			uint16_t compare_value1 = (frame[6] << 8) | frame[7];
			uint8_t input_type2Raw = frame[8];
			uint16_t input_reg2 = (frame[9] << 8) | frame[10];
			uint8_t op2Raw = frame[11];
			uint16_t compare_value2 = (frame[12] << 8) | frame[13];
			uint8_t joinRaw = frame[14];
			uint8_t output_typeRaw = frame[15];
			uint16_t output_reg = (frame[16] << 8) | frame[17];
			uint16_t output_value = (frame[18] << 8) | frame[19];

			// Validate fields
			// Ensure types are valid
			if (input_type1Raw == 0 || input_type1Raw > AUTOMATION_TYPE_COUNT || output_typeRaw == 0 || output_typeRaw > AUTOMATION_TYPE_COUNT) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}
			if ((joinRaw != 1) && (input_type2Raw == 0 || input_type2Raw > AUTOMATION_TYPE_COUNT)) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			// Ensure operations are valid
			if (op1Raw == 0 || op1Raw > AUTOMATION_OPERATION_COUNT) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}
			if ((joinRaw != 1) && (op2Raw == 0 || op2Raw > AUTOMATION_OPERATION_COUNT)) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			// Ensure join is valid
			if (joinRaw < 1 || joinRaw > AUTOMATION_JOIN_COUNT) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}


			// Construct the rule (note enums start at 0, but over modbus i start a 1 so that null errors are easier to catch (i.e., they are 0 only if an error has occured))
			RegisterType input_type1 = input_type1Raw - 1;
			RegisterType output_type = output_typeRaw - 1;
			ComparisonOp op1 = op1Raw - 1;
			LogicJoin join = joinRaw - 1;

			RegisterType input_type2 = 0;
			ComparisonOp op2 = 0;
			if (joinRaw != 1) {
				input_type2 = input_type2Raw - 1;
				op2 = op2Raw - 1;
			}


			LogicRule newRule = {
				.input_type1 = input_type1,
				.input_reg1 = input_reg1,
				.op1 = op1,
				.compare_value1 = compare_value1,

				.input_type2 = input_type2,
				.input_reg2 = input_reg2,
				.op2 = op2,
				.compare_value2 = compare_value2,

				.join = join,

				.output_type = output_type,
				.output_reg = output_reg,
				.output_value = output_value
			};


			uint8_t statusByte = 0x01; // Successful

			bool status = automation_add_rule(newRule);
			if (status == false) {
				statusByte = 0x00; // Unsuccessful -> no more rules allowable.
			}

			// Create the response frame
			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			responseData[0] = slave_address; // the address of us
			responseData[1] = MODBUS_VENDOR_FUNC_ADD_RULE;
			responseData[2] = statusByte; // 1 byte to indicate success/failure

			uint16_t responseLen = 3;

			modbus_send_response(responseData, responseLen);
			break;
		}
		case MODBUS_VENDOR_FUNC_GET_RULE_COUNT: {
			// Create the response frame
			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			uint16_t ruleCount = automation_get_rule_count();

			responseData[0] = slave_address; // the address of us
			responseData[1] = MODBUS_VENDOR_FUNC_ADD_RULE;
			responseData[2] = 0x02; // byte count, 2 bytes follow (16 bits -> uint16_t)
			responseData[3] = ruleCount >> 8; // high byte
			responseData[4] = ruleCount & 0x00FF; // low byte

			uint16_t responseLen = 5;
			modbus_send_response(responseData, responseLen);
			break;
		}
		case MODBUS_VENDOR_FUNC_GET_RULE: {
			// Check request length (Slave Address, Function Code, Index High, Index Low, CRC Low, CRC High)
			if (len != 6) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			uint16_t ruleIndex = (frame[2] << 8) | frame[3];

			LogicRule rule;

			bool status = automation_get_rule(ruleIndex, &rule);
			if (status == false) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE);
				return;
			}

			// Extract fields from the rule struct
			// Enumerations have + 1 as to make first index 1 (easier to detect 0 errors).
			uint8_t input_type1Raw = (uint8_t)rule.input_type1 + 1;
			uint16_t input_reg1 = (uint16_t)rule.input_reg1;
			uint8_t op1Raw = (uint8_t)rule.op1 + 1;
			uint16_t compare_value1 = (uint16_t)rule.compare_value1;
			uint8_t input_type2Raw = (uint8_t)rule.input_type2 + 1;
			uint16_t input_reg2 = (uint16_t)rule.input_reg2;
			uint8_t op2Raw = (uint8_t)rule.op2 + 1;
			uint16_t compare_value2 = (uint16_t)rule.compare_value2;
			uint8_t joinRaw = (uint8_t)rule.join + 1;
			uint8_t output_typeRaw = (uint8_t)rule.output_type + 1;
			uint16_t output_reg = (uint16_t)rule.output_reg;
			uint16_t output_value = (uint16_t)rule.output_value;

			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			// Create the response frame
			responseData[0] = slave_address; // the address of us
			responseData[1] = MODBUS_VENDOR_FUNC_GET_RULE;

			responseData[2] = input_type1Raw;
			responseData[3] = input_reg1 >> 8; // high bit
			responseData[4] = input_reg1 & 0xFF; // low bit
			responseData[5] = op1Raw;
			responseData[6] = compare_value1 >> 8;
			responseData[7] = compare_value1 & 0xFF;
			responseData[8] = input_type2Raw;
			responseData[9] = input_reg2 >> 8;
			responseData[10] = input_reg2 & 0xFF;
			responseData[11] = op2Raw;
			responseData[12] = compare_value2 >> 8;
			responseData[13] = compare_value2 & 0xFF;
			responseData[14] = joinRaw;
			responseData[15] = output_typeRaw;
			responseData[16] = output_reg >> 8;
			responseData[17] = output_reg & 0xFF;
			responseData[18] = output_value >> 8;
			responseData[19] = output_value & 0xFF;

			uint16_t responseLen = 20;
			modbus_send_response(responseData, responseLen);
			break;
		}
		case MODBUS_VENDOR_FUNC_DEL_RULE: {
			// Check request length (Slave Address, Function Code, Index High, Index Low, CRC Low, CRC High)
			if (len != 6) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			uint16_t ruleIndex = (frame[2] << 8) | frame[3];

			bool status = automation_delete_rule(ruleIndex);
			if (status == false) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE);
				return;
			}

			// Create the response frame
			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			responseData[0] = slave_address; // the address of us
			responseData[1] = MODBUS_VENDOR_FUNC_DEL_RULE;
			responseData[2] = 0x01; // 1 byte to indicate success --> no failure byte at this point

			uint16_t responseLen = 3;

			modbus_send_response(responseData, responseLen);
			break;
		}
		case MODBUS_VENDOR_FUNC_ADD_VIRTUAL_REG: {
			// Check request length (Slave Address, Function Code, Register Type, 0x00, CRC Low, CRC High)
			if (len != 6) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			if (frame[2] == 0 || frame[2] > 2) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			VirtualRegisterType registerType = frame[2] - 1; // register type should start at 1 when sent over modbus

			bool status = io_virtual_add(registerType);
			if (status == false) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE);
				return;
			}

			// Create the response frame
			uint8_t responseData[3];

			responseData[0] = slave_address; // the address of us
			responseData[1] = MODBUS_VENDOR_FUNC_ADD_VIRTUAL_REG;
			responseData[2] = 0x01; // status

			uint16_t responseLen = 3;

			modbus_send_response(responseData, responseLen);
			break;
		}
		case MODBUS_VENDOR_FUNC_READ_VIRTUAL_REG: {
			// Check request length (Slave Address, Function Code, Register Type, Address High, Address Low, CRC Low, CRC High)
			if (len != 7) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			// Verify type
			if (frame[2] == 0 || frame[2] > 2) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			VirtualRegisterType registerType = frame[2] - 1; // register type should start at 1 when sent over modbus

			// Get Address
			uint16_t address = frame[3] << 8 | frame[4];

			uint16_t value;
			bool status = io_virtual_read(registerType, address, &value);
			if (status == false) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE);
				return;
			}


			// Create the response frame
			uint8_t byteCount;
			switch (registerType) {
				case VIR_COIL:
					byteCount = 1;
					break;
				case VIR_HOLDING:
					byteCount = 2;
					break;
			}

			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			responseData[0] = slave_address; // the address of us
			responseData[1] = MODBUS_VENDOR_FUNC_READ_VIRTUAL_REG;
			responseData[2] = byteCount;

			switch (registerType) {
				case VIR_COIL:
					responseData[3] = (value != 0) ? 1 : 0;
					break;
				case VIR_HOLDING:
					responseData[3] = value >> 8; // high byte
					responseData[4] = value & 0xFF; // low byte
					break;
			}

			uint16_t responseLen = 3 + byteCount; // slave address, function, byte count, (byte count bytes)

			modbus_send_response(responseData, responseLen);
			break;
		}
		case MODBUS_VENDOR_FUNC_WRITE_VIRTUAL_REG: {
			// Check request length (Slave Address, Function Code, Register Type, Address High, Address Low, Value High, Value Low, CRC Low, CRC High)
			if (len != 9) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			// Verify type
			if (frame[2] == 0 || frame[2] > 2) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			VirtualRegisterType registerType = frame[2] - 1; // register type should start at 1 when sent over modbus

			// Get Address and value
			uint16_t address = (frame[3] << 8) | frame[4];
			uint16_t value = (frame[5] << 8) | frame[6];


			bool status = io_virtual_write(registerType, address, value);
			if (status == false) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE);
				return;
			}

			// Create the response frame
			uint16_t responseLen = 3; // slave address, function, status byte
			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			responseData[0] = slave_address; // the address of us
			responseData[1] = MODBUS_VENDOR_FUNC_WRITE_VIRTUAL_REG;
			responseData[2] = 0x01; // 0x01 = success

			modbus_send_response(responseData, responseLen);
			break;
		}
		case MODBUS_VENDOR_FUNC_GET_VIRTUAL_REG_COUNT: {
			// Check request length (Slave Address, Function Code, Register Type, 0x00, CRC Low, CRC High)
			if (len != 6) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			// Verify type
			if (frame[2] == 0 || frame[2] > 2) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			VirtualRegisterType registerType = frame[2] - 1; // register type should start at 1 when sent over modbus

			uint16_t count;
			bool status = io_virtual_get_count(registerType, &count);
			if (status == false) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE);
				return;
			}

			// Create the response frame
			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			responseData[0] = slave_address; // the address of us
			responseData[1] = MODBUS_VENDOR_FUNC_GET_VIRTUAL_REG_COUNT;
			responseData[2] = 0x02; // byte count
			responseData[3] = count >> 8; // high byte
			responseData[4] = count & 0xFF; // low byte

			uint16_t responseLen = 5;

			modbus_send_response(responseData, responseLen);
			break;
		}
		case MODBUS_VENDOR_FUNC_SET_RTC: {
			// Check request length (Slave Address, Function Code, 7x data, CRC Low, CRC High)
			if (len != 11) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}

			RTC_Time setTime;
			setTime.seconds 	= frame[2];
			setTime.minutes		= frame[3];
			setTime.hours		= frame[4];
			setTime.day_of_week	= frame[5];
			setTime.day			= frame[6];
			setTime.month		= frame[7];
			setTime.year		= frame[8];

			// Set time on DS3231
			HAL_StatusTypeDef status = DS3231_SetTime(&setTime);
			if (status != HAL_OK) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE);
				return;
			}

			// Create the response frame
			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			responseData[0] = slave_address; // the address of us
			responseData[1] = MODBUS_VENDOR_FUNC_SET_RTC;
			responseData[2] = 0x01; // status byte (0x01 = success)

			uint16_t responseLen = 3;

			modbus_send_response(responseData, responseLen);
			break;
		}
		default: {
			modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
			break;
		}
	}
}
