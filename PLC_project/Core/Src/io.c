#include "io.h"

// Allocates storage for all the IO channels.
IO_Channel io_channel[MAX_IO_CHANNELS]; // Array of type IO_Channel (struct created in header)
int io_channel_count = 0;


void io_add_channel(GPIO_TypeDef* port, uint16_t pin, IO_Direction dir) {
	if (io_channel_count >= MAX_IO_CHANNELS) return; // Cannot add another channel if all channels taken

	io_channel[io_channel_count].port = port;
	io_channel[io_channel_count].pin = pin;
	io_channel[io_channel_count].direction = dir;
	io_channel_count++;
}

GPIO_PinState io_read(int index) {
	if (index >= 0 && index < io_channel_count && io_channel[index].direction == IO_INPUT) {
		return HAL_GPIO_ReadPin(io_channel[index].port, io_channel[index].pin);
	}
	return GPIO_PIN_RESET;
}

void io_write(int index, GPIO_PinState value) {
	if (index >= 0 && index < io_channel_count && io_channel[index].direction == IO_OUTPUT) {
		HAL_GPIO_WritePin(io_channel[index].port, io_channel[index].pin, value);
	}
}
