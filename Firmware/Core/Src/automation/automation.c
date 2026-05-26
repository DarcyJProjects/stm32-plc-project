#include "automation/automation.h"
#include "modbus/modbus_util.h"

#define MAX_RULES 32 // max rules due to EEPROM size TODO: Find actual max size
#define MAX_COPY_RULES 8

static LogicRule rules[MAX_RULES];
static uint16_t rule_count = 0;

static CopyRule copyRules[MAX_COPY_RULES];
static uint16_t copyRule_count = 0;

static uint16_t read_register_value(RegisterType type, uint16_t addr) {
	switch (type) {
		case REG_COIL:		return io_coil_read(addr);
		case REG_DISCRETE:	return io_discrete_in_read(addr);
		case REG_HOLDING:	return io_holding_reg_read(addr);
		case REG_INPUT:		return io_input_reg_read(addr);
		case REG_VIR_COIL: {
			uint16_t value = 0;
			io_virtual_read(VIR_COIL, addr, &value);
			return value;
		}
		case REG_VIR_HOLDING: {
			uint16_t value = 0;
			io_virtual_read(VIR_HOLDING, addr, &value);
			return value;
		}
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
		case REG_VIR_COIL:
			io_virtual_write(VIR_COIL, addr, write_value);
			break;
		case REG_VIR_HOLDING:
			io_virtual_write(VIR_HOLDING, addr, write_value);
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

static void apply_copyRule(const CopyRule* rule) {
	uint16_t value = read_register_value(rule->input_type, rule->input_reg);
	write_register_value(rule->output_type, rule->output_reg, value);
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

	for (uint16_t i = 0; i < copyRule_count; i++) {
		apply_copyRule(&copyRules[i]);
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

bool automation_add_copyRule(CopyRule newCopyRule) {
	if (copyRule_count < MAX_COPY_RULES) {
		copyRules[copyRule_count] = newCopyRule;
		copyRule_count++;

		// Save to EEPROM
		if (!automation_save_rules()) {
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

uint16_t automation_get_copyRule_count(void) {
	return copyRule_count;
}

bool automation_get_rule(uint16_t index, LogicRule* rule) {
	if (index >= rule_count) {
		return false;
	}

	*rule = rules[index];
	return true;
}

bool automation_get_copyRule(uint16_t index, CopyRule* copyRule) {
	if (index >= copyRule_count) {
		return false;
	}

	*copyRule = copyRules[index];
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

bool automation_delete_copyRule(uint16_t index) {
	if (index >= copyRule_count) {
		return false;
	}

	// Shift elements after index down by 1
	if (index < copyRule_count - 1) {
		memmove(&copyRules[index], &copyRules[index + 1], (copyRule_count - index - 1) * sizeof(CopyRule));
	}

	// Clear the last element
	memset(&copyRules[copyRule_count - 1], 0, sizeof(CopyRule));

	// Decrement the count
	copyRule_count--;

	// Save to EEPROM
	if (!automation_save_rules()) {
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

	// Write the copy rule count
	if (!EEPROM_WriteBlock(addr, &copyRule_count, sizeof(copyRule_count))) {
		return false;
	}
	addr += sizeof(copyRule_count);

	// Write copy rules
	for (uint16_t i = 0; i < copyRule_count; i++) {
		if (!EEPROM_WriteBlock(addr, &copyRules[i], sizeof(CopyRule))) {
			return false;
		}
		addr += sizeof(CopyRule);
	}

	// Compute CRC16 over rule_count + rules
	uint16_t total_size = sizeof(rule_count) + (rule_count * sizeof(LogicRule)) +
						  sizeof(copyRule_count) + (copyRule_count * sizeof(CopyRule));

	uint8_t crc_buffer[sizeof(rule_count) + MAX_RULES * sizeof(LogicRule) +
					   sizeof(copyRule_count) + MAX_COPY_RULES * sizeof(CopyRule)];

	uint16_t offset = 0;
	memcpy(crc_buffer + offset, &rule_count, sizeof(rule_count));
	offset += sizeof(rule_count);

	memcpy(crc_buffer + offset, rules, rule_count * sizeof(LogicRule));
	offset += rule_count * sizeof(LogicRule);

	memcpy(crc_buffer + offset, &copyRule_count, sizeof(copyRule_count));
	offset += sizeof(copyRule_count);

	memcpy(crc_buffer + offset, copyRules, copyRule_count * sizeof(CopyRule));

	uint16_t crc = modbus_crc16(crc_buffer, total_size);

	if (!EEPROM_WriteBlock(addr, (uint8_t*)&crc, sizeof(crc))) {
		return false;
	}

	addr += sizeof(crc);

	// Now save virtual registers
	return io_virtual_save(addr);
}

bool automation_factory_reset(void) {
	// factory reset io_holding_reg_types
	io_holding_reg_type_clear(true); // true for factory reset mode
	io_input_reg_type_clear(true); // true for factory reset mode
	emergencyStop_clear(true); // true for factory reset mode

	memset(rules, 0, sizeof(rules));
	rule_count = 0;

	memset(copyRules, 0, sizeof(copyRules));
	copyRule_count = 0;

	if (!automation_save_rules()) return false;

	uint16_t baseAddress = sizeof(rule_count) + sizeof(copyRule_count) + sizeof(uint16_t);

	// Now save virtual registers
	if (!io_virtual_factory_reset(baseAddress)) return false;

	return true;
}

bool automation_load_rules(void) {
	uint16_t addr = 0x0000;
	uint16_t saved_rule_count = 0;
	uint16_t saved_copyRule_count = 0;

	// Load rule count
	if (!EEPROM_LoadBlock(addr, &saved_rule_count, sizeof(saved_rule_count))) return false;
	if (saved_rule_count > MAX_RULES) return false;
	addr += sizeof(saved_rule_count);

	// Load rules into temp buffer
	LogicRule temp_rules[MAX_RULES];
	if (saved_rule_count > 0) {
		if (!EEPROM_LoadBlock(addr, temp_rules, saved_rule_count * sizeof(LogicRule))) return false;
		addr += saved_rule_count * sizeof(LogicRule);
	}

	// Load copyRule count
	if (!EEPROM_LoadBlock(addr, &saved_copyRule_count, sizeof(saved_copyRule_count))) return false;
	if (saved_copyRule_count > MAX_COPY_RULES) return false;
	addr += sizeof(saved_copyRule_count);

	// Load copyRules into temp buffer
	CopyRule temp_copyRules[MAX_COPY_RULES];
	if (saved_copyRule_count > 0) {
		if (!EEPROM_LoadBlock(addr, temp_copyRules, saved_copyRule_count * sizeof(CopyRule))) return false;
		addr += saved_copyRule_count * sizeof(CopyRule);
	}

	// Read stored CRC16
	uint16_t stored_crc = 0;
	if (!EEPROM_LoadBlock(addr, &stored_crc, sizeof(stored_crc))) return false;

	// Compute CRC16 for validation
	uint16_t total_size = sizeof(saved_rule_count) + (saved_rule_count * sizeof(LogicRule)) +
						  sizeof(saved_copyRule_count) + (saved_copyRule_count * sizeof(CopyRule));

	uint8_t crc_buffer[sizeof(saved_rule_count) + MAX_RULES * sizeof(LogicRule) +
					   sizeof(saved_copyRule_count) + MAX_COPY_RULES * sizeof(CopyRule)];

	uint16_t offset = 0;
	memcpy(crc_buffer + offset, &saved_rule_count, sizeof(saved_rule_count));
	offset += sizeof(saved_rule_count);

	if (saved_rule_count > 0) {
		memcpy(crc_buffer + offset, temp_rules, saved_rule_count * sizeof(LogicRule));
		offset += saved_rule_count * sizeof(LogicRule);
	}

	memcpy(crc_buffer + offset, &saved_copyRule_count, sizeof(saved_copyRule_count));
	offset += sizeof(saved_copyRule_count);

	if (saved_copyRule_count > 0) {
		memcpy(crc_buffer + offset, temp_copyRules, saved_copyRule_count * sizeof(CopyRule));
	}

	uint16_t computed_crc = modbus_crc16(crc_buffer, total_size);

	if (computed_crc != stored_crc) {
		return false;
	}

	addr += sizeof(stored_crc);

	// All valid, so apply to live arrays
	memcpy(rules, temp_rules, saved_rule_count * sizeof(LogicRule));
	rule_count = saved_rule_count;

	memcpy(copyRules, temp_copyRules, saved_copyRule_count * sizeof(CopyRule));
	copyRule_count = saved_copyRule_count;

	// Now load virtual registers
	if (!io_virtual_load(addr)) {
		return false;
	}

	return true;
}
