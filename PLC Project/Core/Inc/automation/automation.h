#ifndef AUTOMATION_H
#define AUTOMAT_H

#include <stdint.h>
#include <stdbool.h>

#include "io/io_coils.h"
#include "io/io_discrete_in.h"
#include "io/io_holding_reg.h"
#include "io/io_input_reg.h"

typedef enum {
	REG_COIL,
	REG_DISCRETE,
	REG_HOLDING,
	REG_INPUT
} RegisterType;

typedef enum {
	CMP_EQ, 	// ==
	CMP_NEQ,	// !=
	CMP_GT,		// >
	CMP_LT,		// <
	CMP_GTET,	// >=
	CMP_LTET	// <=
} ComparisonOp;

typedef enum {
	LOGIC_NONE,
	LOGIC_AND,
	LOGIC_OR
} LogicJoin;

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

bool parse_rule(const char* rule_str, LogicRule* rule_out);

#endif
