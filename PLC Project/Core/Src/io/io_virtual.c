#include "io/io_virtual.h"

Virtual_Coil_Channel virtual_coil_channels[MAX_VIRTUAL_COILS];
uint16_t virtual_coil_channel_count = 0;

Virtual_Holding_Reg_Channel virtual_holding_reg_channels[MAX_VIRTUAL_HOLDING_REG];
uint16_t virtual_holding_reg_channel_count = 0;


bool io_virtual_add(VirtualRegisterType type) {
	switch (type) {
		case VIR_COIL: {
			if (virtual_coil_channel_count == MAX_VIRTUAL_COILS) {
				return false;
			}
			virtual_coil_channels[virtual_coil_channel_count].storedValue = 0;
			virtual_coil_channel_count++;
			return true;
		}
		case VIR_HOLDING: {
			if (virtual_holding_reg_channel_count == MAX_VIRTUAL_HOLDING_REG) {
				return false;
			}
			virtual_holding_reg_channels[virtual_holding_reg_channel_count].storedValue = 0;
			virtual_holding_reg_channel_count++;
			return true;
		}
		default: {
			return false;
		}
	}
}


bool io_virtual_get_count(VirtualRegisterType type, uint16_t* count) {
	if (!count) return false;

	switch (type) {
		case VIR_COIL: {
			*count = virtual_coil_channel_count;
			return true;
		}
		case VIR_HOLDING: {
			*count = virtual_holding_reg_channel_count;
			return true;
		}
		default: {
			return false;
		}
	}
}

bool io_virtual_read(VirtualRegisterType type, uint16_t index, uint16_t* value) {
	if (!value) return false;

	switch (type) {
		case VIR_COIL: {
			if (index >= virtual_coil_channel_count) {
				return false;
			}

			*value = (uint16_t)(virtual_coil_channels[index].storedValue);
			return true;
		}
		case VIR_HOLDING: {
			if (index >= virtual_holding_reg_channel_count) {
				return false;
			}
			*value = virtual_holding_reg_channels[index].storedValue;
			return true;
		}
		default: {
			return false;
		}
	}
}

bool io_virtual_write(VirtualRegisterType type, uint16_t index, uint16_t value) {
	switch (type) {
		case VIR_COIL: {
			if (index >= virtual_coil_channel_count) {
				return false;
			}

			virtual_coil_channels[index].storedValue = (value != 0) ? 1 : 0;
			return true;
		}
		case VIR_HOLDING: {
			if (index >= virtual_holding_reg_channel_count) {
				return false;
			}

			virtual_holding_reg_channels[index].storedValue = value;
			return true;
		}
		default: {
			return false;
		}
	}
}
