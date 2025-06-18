#include "automation/automation.h"

#define MAX_RULES 32 // max rules due to EEPROM size TODO: Find actual max size

static LogicRule rules[MAX_RULES];
static uint16_t rule_count = 0;

static uint16_t read_register_value(RegisterType type, uint16_t addr) {
	switch (type) {
		case REG_COIL:		return io_coil_read(addr);
		case REG_DISCRETE:	return io_discrete_in_read(addr);
		case REG_HOLDING:	return io_holding_reg_read(addr);
		case REG_INPUT:		return io_input_reg_read(addr);
		default:			return 0;
	}
}

static void write_register_value(RegisterType type, uint16_t addr, uint16_t write_value) {
	switch (type) {
		case REG_COIL:
			io_coil_write(addr, write_value);
			break;
		case REG_HOLDING:
			io_holding_reg_write(addr, write_value);
			break;
		default:
			break;
	}
}

static bool compare(uint16_t val1, ComparisonOp op, uint16_t val2) {
	switch (op) {
		case CMP_EQ:	return val1 == val2;
		case CMP_NEQ:	return val1 != val2;
		case CMP_GT:	return val1 > val2;
		case CMP_LT:	return val1 < val2;
		case CMP_GTET:	return val1 >= val2;
		case CMP_LTET:	return val1 <= val2;
		default:		return false;
	}
}

static bool evaluate_rule(const LogicRule* rule) {
	bool condition1 = compare(
		read_register_value(rule->input_type1, rule->input_reg1),
		rule->op1,
		rule->compare_value1
	);

	if (rule->join == LOGIC_NONE) {
		return condition1;
	}

	bool condition2 = compare(
		read_register_value(rule->input_type2, rule->input_reg2),
		rule->op2,
		rule->compare_value2
	);

	if (rule->join == LOGIC_AND) return condition1 && condition2;
	if (rule->join == LOGIC_OR) return condition1 || condition2;
	return false;
}

static void apply_rule(const LogicRule* rule) {
	if (evaluate_rule(rule)) {
		write_register_value(rule->output_type, rule->output_reg, rule->output_value);
	}
}

void automation_Init(void) {
	automation_load_rules();
}

void automation_Tick(void) {
	for (uint16_t i = 0; i < rule_count; i++) {
		apply_rule(&rules[i]);
	}
}

bool automation_add_rule(LogicRule newRule) {
	if (rule_count < MAX_RULES) {
		rules[rule_count] = newRule;
		rule_count++;

		// Save to EEPROM
		bool status = automation_save_rules();
		if (status == false) {
			return false;
		}

		return true;
	} else {
		return false;
	}
}

uint16_t automation_get_rule_count(void) {
	return rule_count;
}

bool automation_get_rule(uint16_t index, LogicRule* rule) {
	if (index >= rule_count) {
		return false;
	}

	*rule = rules[index];
	return true;
}

bool automation_delete_rule(uint16_t index) {
	if (index >= rule_count) {
		return false;
	}

	// Shift elements after index down by 1
	if (index < rule_count - 1) {
		memmove(&rules[index], &rules[index + 1], (rule_count - index - 1) * sizeof(LogicRule));
	}

	// Clear the last element
	memset(&rules[rule_count -1], 0, sizeof(LogicRule));

	// Decrement the count
	rule_count--;

	// Save to EEPROM
	bool status = automation_save_rules();
	if (status == false) {
		return false;
	}

	return true;
}

bool automation_save_rules(void) {
	if (rule_count > MAX_RULES) return false;

	uint16_t addr = 0x0000;

	// Write the rule count first
	if (!EEPROM_WriteBlock(addr, &rule_count, sizeof(rule_count))) {
		return false;
	}
	addr += sizeof(rule_count);

	// Write rules
	for (uint16_t i = 0; i < rule_count; i++) {
		if (!EEPROM_WriteBlock(addr, &rules[i], sizeof(LogicRule))) {
			return false;
		}
		addr += sizeof(LogicRule);
	}

	// Compute CRC16 over rule_count + rules
	uint16_t crc_input_len = sizeof(rule_count) + rule_count * sizeof(LogicRule);
	uint8_t crc_buffer[sizeof(rule_count) + MAX_RULES * sizeof(LogicRule)];
	memcpy(crc_buffer, &rule_count, sizeof(rule_count));
	memcpy(crc_buffer + sizeof(rule_count), rules, rule_count * sizeof(LogicRule));

	uint16_t crc = modbus_crc16(crc_buffer, crc_input_len);

	if (!EEPROM_WriteBlock(addr, (uint8_t*)&crc, sizeof(crc))) {
		return false;
	}

	return true;
}

bool automation_load_rules(void) {
	uint16_t addr = 0x0000;
	uint16_t saved_count = 0;

	if (!EEPROM_LoadBlock(addr, &saved_count, sizeof(saved_count))) {
		return false;
	}
	if (saved_count > MAX_RULES) {
		return false;
	}

	addr += sizeof(saved_count);

	if (saved_count == 0) {
		// Still need to validate CRC16 of just the rule count (2 bytes)
		uint16_t stored_crc = 0;
		if (!EEPROM_LoadBlock(addr, &stored_crc, sizeof(stored_crc))) {
			return false;
		}

		uint16_t computed_crc = modbus_crc16((uint8_t*)&saved_count, sizeof(saved_count));
		if (computed_crc != stored_crc) {
			return false;
		}

		rule_count = 0;
		return true;
	}
	// Otherwise load as usual:

	// Temporarily load the data into a buffer
	LogicRule temp_rules[MAX_RULES];
	if (!EEPROM_LoadBlock(addr, temp_rules, saved_count * sizeof(LogicRule))) {
		return false;
	}
	addr += saved_count * sizeof(LogicRule);

	// Read stored CRC16
	uint16_t stored_crc = 0;
	if (!EEPROM_LoadBlock(addr, &stored_crc, sizeof(stored_crc))) {
		return false;
	}

	// Compute CRC16
	uint8_t crc_buffer[sizeof(saved_count) + MAX_RULES * sizeof(LogicRule)];
	memcpy(crc_buffer, &saved_count, sizeof(saved_count));
	memcpy(crc_buffer + sizeof(saved_count), temp_rules, saved_count * sizeof(LogicRule));

	uint16_t computed_crc = modbus_crc16(crc_buffer, sizeof(saved_count) + saved_count * sizeof(LogicRule));

	if (computed_crc != stored_crc) {
		return false;
	}


	// All valid, so use these rules
	memcpy(rules, temp_rules, saved_count * sizeof(LogicRule));
	rule_count = saved_count;
	return true;
}
