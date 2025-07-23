#include "io/io_emergency.h"
#include "i2c/display.h"
#include "io/io_coils.h"
#include "io/io_holding_reg.h"

static bool defined = false;
static uint16_t channel;
static bool latched = false;
static Emergency_Stop_Input_Mode inputMode;

bool emergencyStop_setInput(uint16_t index, Emergency_Stop_Input_Mode mode) {
	if (!io_discrete_in_check_channel(index)) return false;

	// TODO: ADD MODE FOR NC OR NO

	channel = index;
	inputMode = mode;
	defined = true;
	return true;
}

bool emergencyStop_check(void) {
	if (!defined) return false;
	if (latched) return true;

	if ((io_discrete_in_read(channel) && inputMode == EMERGENCY_STOP_NO) || (!io_discrete_in_read(channel) && inputMode == EMERGENCY_STOP_NC)) {
		// Emergency!
		latched = true;

		display_EmergencyStop();

		// Set all coils to OFF
		io_coils_emergencystop();

		// Set all holding registers to 0
		io_holding_reg_emergencystop();

		return true;
	}

	return false;
}

void emergencyStop_reset(void) {
	latched = false;
}
