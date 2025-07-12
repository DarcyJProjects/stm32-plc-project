#include "io/io_modbus_slaves.h"
#include "modbus/modbus_util.h"
#include "modbus/modbus_master.h"

// Allocates storage for all the slaves.
Modbus_Slave_Channel io_modbus_slave_channels[MAX_MODBUS_SLAVES];
uint16_t io_modbus_slave_channel_count = 0;

static uint8_t current_polling_index = 0; // index of slave being polled
static bool polling_in_progress = false;

// Adds a new channel to the list
void io_modbus_slave_add_channel(uint8_t slave_address, Modbus_Register_Type type, uint16_t register_address) {
	if (io_modbus_slave_channel_count >= MAX_MODBUS_SLAVES) return;

	Modbus_Slave_Channel* ch = &io_modbus_slave_channels[io_modbus_slave_channel_count++];

	ch->slave_address = slave_address;
	ch->type = type;
	ch->register_address = register_address;
	ch->buffer = 0;
	ch->last_updated_ms = get_ms();
}

// Read/write the channels with their read/write functions and context
uint16_t io_modbus_slave_read(uint16_t index) {
	if (index >= io_modbus_slave_channel_count) return 0;
	return io_modbus_slave_channels[index].buffer;
}

// Poll one slave at a time
void io_modbus_slave_poll_all(void) {
	if (modbus_master_is_busy()) return;
	polling_in_progress = true;

	if (io_modbus_slave_channel_count == 0) return;

	uint32_t now_ms = get_ms();

	for (uint8_t i = 0; i < io_modbus_slave_channel_count; i++) {
		uint8_t index = (current_polling_index + i) % io_modbus_slave_channel_count;
		Modbus_Slave_Channel* ch = &io_modbus_slave_channels[index];

		// Check if it's time to poll this one
		if ((now_ms - ch->last_updated_ms) >= MODBUS_SLAVE_POLLING_MS) {
			bool success = modbus_master_request_read(ch->slave_address, ch->type, ch->register_address, &ch->buffer);
			if (success) {
				polling_in_progress = true;
				ch->last_updated_ms = now_ms;
				current_polling_index = (index + 1) % io_modbus_slave_channel_count;
				break;
			} else {
				polling_in_progress = false;
				// Master is busy or failed to queue, just wait and try again in the next cycle
				usb_serial_println("Read failed");
			}
		}
	}
}

void io_modbus_slave_write(uint16_t index, uint16_t writeValue) {
	if (index >= io_modbus_slave_channel_count) return;

	Modbus_Slave_Channel currentSlave = io_modbus_slave_channels[index];

	// TODO: MODBUS MASTER, SEND WRITE COMMAND DEPENDING ON TYPE

	switch (currentSlave.type) {
		case MODBUS_REGISTER_COIL:
			uint8_t val = writeValue ? 1 : 0;
			io_modbus_slave_channels[index].buffer = val;
			// TODO: WRITE
			return;
		case MODBUS_REGISTER_DISCRETE_INPUT:
			io_modbus_slave_channels[index].buffer = writeValue;
			return; // TODO
		case MODBUS_REGISTER_HOLDING:
			return; // R only
		case MODBUS_REGISTER_INPUT:
			return; // R only
	}
}
