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
	CMP_EQ,
	CMP_NEQ,
	CMP_GT,
	CMP_LT
} ComparisonOp;

typedef struct {
	RegisterType input_type;
	uint16_t input_reg;
	ComparisonOp op;
	uint16_t compare_value;

	RegisterType output_type;
	uint16_t output_reg;
	uint16_t output_value;
} LogicRule;

void automation_Init(void);
void automation_Tick(void);

bool parse_rule(const char* rule_str, LogicRule* rule_out);

#endif
