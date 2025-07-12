#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <modbus/modbus_slave.h> // for modbus_crc()
#include <stdint.h>
#include <stdbool.h>

#include "io/io_coils.h"
#include "io/io_discrete_in.h"
#include "io/io_holding_reg.h"
#include "io/io_input_reg.h"
#include "io/io_virtual.h"

#include "i2c/eeprom.h"

typedef enum {
	REG_COIL,
	REG_DISCRETE,
	REG_HOLDING,
	REG_INPUT,
	REG_VIR_COIL,
	REG_VIR_HOLDING
} RegisterType;

#define AUTOMATION_TYPE_COUNT 6 // required for modbus frame validation

typedef enum {
	CMP_EQ, 	// ==
	CMP_NEQ,	// !=
	CMP_GT,		// >
	CMP_LT,		// <
	CMP_GTET,	// >=
	CMP_LTET	// <=
} ComparisonOp;

#define AUTOMATION_OPERATION_COUNT 6 // required for modbus frame validation

typedef enum {
	LOGIC_NONE,
	LOGIC_AND,
	LOGIC_OR
} LogicJoin;

#define AUTOMATION_JOIN_COUNT 3 // required for modbus frame validation

typedef struct {
	RegisterType input_type1;
	uint16_t input_reg1;
	ComparisonOp op1;
	uint16_t compare_value1;

	RegisterType input_type2;
	uint16_t input_reg2;
	ComparisonOp op2;
	uint16_t compare_value2;

	LogicJoin join;

	RegisterType output_type;
	uint16_t output_reg;
	uint16_t output_value;
} LogicRule;

void automation_Init(void);
void automation_Tick(void);

bool automation_add_rule(LogicRule newRule);

uint16_t automation_get_rule_count(void);

bool automation_get_rule(uint16_t index, LogicRule* rule);

bool automation_delete_rule(uint16_t index);

bool automation_save_rules(void);

bool automation_factory_reset(void);

bool automation_load_rules(void);

#endif
