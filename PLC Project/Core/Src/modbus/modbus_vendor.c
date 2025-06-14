#include "modbus/modbus_vendor.h"
#include "modbus/modbus.h"
#include "automation/automation.h"

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
			if (op1Raw == 0 || op1Raw >= AUTOMATION_OPERATION_COUNT) {
				modbus_send_exception(slave_address, function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
				return;
			}
			if ((joinRaw != 1) && (op2Raw == 0 || op2Raw >= AUTOMATION_OPERATION_COUNT)) {
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

			automation_add_rule(newRule);

			// Create the response frame
			uint8_t responseData[MODBUS_MAX_FRAME_SIZE];

			responseData[0] = slave_address; // the address of us
			responseData[1] = MODBUS_VENDOR_FUNC_ADD_RULE;
			responseData[2] = 0x01; // 1 byte to indicate success

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
