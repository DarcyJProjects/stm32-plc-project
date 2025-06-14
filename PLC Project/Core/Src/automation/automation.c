#include "automation/automation.h"

#define MAX_RULES 32 // max rules due to EEPROM size TODO: Find actual max size

static LogicRule rules[MAX_RULES];
static uint16_t rule_count = 0;

static uint16_t read_register_value(RegisterType type, uint16_t addr) {
	switch (type) {
		case REG_COIL:
			return io_coil_read(addr);
			break;
		case REG_DISCRETE:
			return io_discrete_in_read(addr);
			break;
		case REG_HOLDING:
			return io_holding_reg_read(addr);
			break;
		case REG_INPUT:
			return io_input_reg_read(addr);
			break;
		default:
			return 0;
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
	}
}

static bool evaluate_rule(const LogicRule* rule) {
	uint16_t val = read_register_value(rule->input_type, rule->input_reg);
	uint16_t comp = rule->compare_value;
	switch (rule->op) {
		case CMP_EQ:	return val == comp;
		case CMP_NEQ:	return val != comp;
		case CMP_GT:	return val > comp;
		case CMP_LT:	return val < comp;
		default:		return false;
	}
}

static void apply_rule(const LogicRule* rule) {
	if (evaluate_rule(rule)) {
		write_register_value(rule->output_type, rule->output_reg, rule->output_value);
	}
}

void automation_Init(void) {
	// Hard coded test rules TODO: THIS IS ONLY FOR TESTING
	rules[0] = (LogicRule){ REG_INPUT, 0, CMP_GT, 30000, REG_COIL, 0, 1 };
	rule_count = 1;
}

void automation_Tick(void) {
	for (uint16_t i = 0; i < rule_count; i++) {
		apply_rule(&rules[i]);
	}
}
