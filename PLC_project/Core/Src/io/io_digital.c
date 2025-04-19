#include <io/io_digital.h>

// Allocates storage for all the IO channels.
IO_Digital_Channel io_digital_channels[MAX_IO_DIGITAL]; // Array of type IO_Channel (struct created in header)
int io_digital_channel_count = 0;


void io_digital_add_channel(GPIO_TypeDef* port, uint16_t pin, IO_Direction dir) {
	if (io_digital_channel_count >= MAX_IO_DIGITAL) return; // Cannot add another channel if all channels taken

	io_digital_channels[io_digital_channel_count].port = port;
	io_digital_channels[io_digital_channel_count].pin = pin;
	io_digital_channels[io_digital_channel_count].direction = dir;
	io_digital_channel_count++;
}

GPIO_PinState io_digital_read(int index) {
	if (index >= 0 && index < io_digital_channel_count && io_digital_channels[index].direction == IO_INPUT) {
		return HAL_GPIO_ReadPin(io_digital_channels[index].port, io_digital_channels[index].pin);
	}
	return GPIO_PIN_RESET;
}

void io_digital_write(int index, GPIO_PinState value) {
	if (index >= 0 && index < io_digital_channel_count && io_digital_channels[index].direction == IO_OUTPUT) {
		HAL_GPIO_WritePin(io_digital_channels[index].port, io_digital_channels[index].pin, value);
	}
}
