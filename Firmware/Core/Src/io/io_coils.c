#include <io/io_coils.h>

// Allocates storage for all the IO channels.
IO_Coil_Channel io_coil_channels[MAX_IO_COILS]; // Array of type IO_Channel (struct created in header)
uint16_t io_coil_channel_count = 0;
uint16_t io_hardware_coil_channel_count = 0;


bool io_coil_add_channel(void (*write_func)(void*, uint16_t), void* context) {
	if (io_coil_channel_count == MAX_IO_COILS) {
		return false;
	}

	if (write_func == hardware_coil_write_func) {
		// Enforce limit only for physical coil outputs
		if (io_hardware_coil_channel_count >= MAX_IO_COILS_PHYSICAL) {
			return false;
		} else {
			io_hardware_coil_channel_count++; // increase physical coil channel count
		}
	}

	io_coil_channels[io_coil_channel_count].write_func = write_func;
	io_coil_channels[io_coil_channel_count].context = context;
	io_coil_channels[io_coil_channel_count].storedState = 0;
	io_coil_channel_count++; // increase overall channel count
	return true;
}


GPIO_PinState io_coil_read(uint16_t index) {
	if (index < io_coil_channel_count) {
		return io_coil_channels[index].storedState;
	}
	return GPIO_PIN_RESET;
}


void io_coil_write(uint16_t index, GPIO_PinState state) {
	if (index < io_coil_channel_count) {
		if (io_coil_channels[index].write_func) {
			io_coil_channels[index].write_func(io_coil_channels[index].context, state);
		}
		io_coil_channels[index].storedState = state;
	}
}

void io_coils_emergencystop(void) {
	// Iterate over all coils, even if outside channel count just to be safe
	for (uint16_t i = 0; i < MAX_IO_COILS; i++) {
		io_coil_write(i, GPIO_PIN_RESET); // low
	}
}

void hardware_coil_write_func(void* context, GPIO_PinState state) {
	gpio_config* gpio = (gpio_config*)context;
	HAL_GPIO_WritePin(gpio->port, gpio->pin, state);
}
