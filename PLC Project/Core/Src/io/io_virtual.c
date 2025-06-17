#include "io/io_virtual.h"

extern uint16_t io_coil_channel_count;
extern uint16_t io_holding_reg_channel_count;

bool io_virtual_add(RegisterType type, uint16_t* index) {
	switch (type) {
		case REG_COIL: {
			bool status = io_coil_add_channel(NULL, NULL);
			if (status == true) {
				*index = io_coil_channel_count - 1;
				return true;
			}
			return false;
		}
		case REG_HOLDING: {
			bool status = io_holding_reg_add_channel(NULL, NULL);
			if (status == true) {
				*index = io_holding_reg_channel_count - 1;
				return true;
			}
			return false;
		}
		default: {
			return false;
		}
	}
}
