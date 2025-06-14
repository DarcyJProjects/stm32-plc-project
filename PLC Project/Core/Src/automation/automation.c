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
	// Hard coded test rules TODO: THIS IS ONLY FOR TESTING

	/*rules[0] = (LogicRule){
		REG_INPUT, 0, CMP_GT, 30000, // If Input Register 0 > 30000
		REG_HOLDING, 0, CMP_EQ, 4096, // If Holding Register 0 == 4096
		LOGIC_AND, // Condition 1 AND Condition 2
		REG_COIL, 0, 1 // Then, set Coil 0 to 1.
	};

	// Inverse of rules[0] to reset the coil state:
	rules[1] = (LogicRule){
		REG_INPUT, 0, CMP_LT, 30001, // If Input Register 0 !> 30000
		REG_HOLDING, 0, CMP_NEQ, 4096, // If Holding Register 0 != 4096
		LOGIC_OR, // Condition 1 OR Condition 2
		REG_COIL, 0, 0 // Then, set Coil 0 to 0.
	};

	rule_count = 2;*/
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
		return true;
	} else {
		return false;
	}
}
