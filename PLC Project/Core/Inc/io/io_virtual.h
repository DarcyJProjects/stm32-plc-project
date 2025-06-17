#ifndef IO_VIRTUAL_H
#define IO_VIRTUAL_H

#include "stm32g4xx_hal.h"
#include <stdbool.h> // Lets us use boolean logic
#include "automation/automation.h" // to use the RegisterType enumeration

#include "io/io_coils.h"
#include "io/io_discrete_in.h"
#include "io/io_holding_reg.h"
#include "io/io_input_reg.h"

bool io_virtual_add(RegisterType type, uint16_t* index);

#endif
