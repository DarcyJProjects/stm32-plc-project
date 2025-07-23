#ifndef IO_EMERGENCY_H
#define IO_EMERGENCY_H

#include "io_discrete_in.h"
#include <stdbool.h>

typedef enum {
	EMERGENCY_STOP_NO,
	EMERGENCY_STOP_NC
} Emergency_Stop_Input_Mode;

bool emergencyStop_setInput(uint16_t index, Emergency_Stop_Input_Mode mode);

bool emergencyStop_check(void);

void emergencyStop_reset(void);

#endif
