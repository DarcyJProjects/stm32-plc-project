#include <io/io_digital.h>

// Allocates storage for all the IO channels.
IO_Digital_Channel io_digital_channels[MAX_IO_DIGITAL]; // Array of type IO_Channel (struct created in header)
int io_digital_channel_count = 0;

/**
 * @Brief Adds a digital input or output channel to the system
 *
 * This function adds a digital input or output to the system creating a channel which can
 * be used to read from or write to. The channel number is incremented with each channel added.
 *
 * @param port: Pointer to the port where the digital pin is connected (e.g., GPIOA, GPIOB, GPIOC).
 * @param pin: Pin number of the GPIO port where the digital pin is connected.
 * @param dir: Direction of the channel (IO_DIGITAL_INPUT or IO_DIGITAL_OUTPUT)
 */
void io_digital_add_channel(GPIO_TypeDef* port, uint16_t pin, IO_Direction dir) {
	if (io_digital_channel_count >= MAX_IO_DIGITAL) return; // Cannot add another channel if all channels taken

	io_digital_channels[io_digital_channel_count].port = port;
	io_digital_channels[io_digital_channel_count].pin = pin;
	io_digital_channels[io_digital_channel_count].direction = dir;
	io_digital_channel_count++;
}

/**
 * @Brief Reads the current value from a registered digital input channel of a provided index.
 *
 * @param index: The index of the channel to read (assigned in order of registration with io_digital_add_channel)
 *
 * @retval The GPIO_PinState of the digital input, or 0 if the channel is invalid or not an input channel.
 */
GPIO_PinState io_digital_read(int index) {
	if (index >= 0 && index < io_digital_channel_count && io_digital_channels[index].direction == IO_DIGITAL_INPUT) {
		return HAL_GPIO_ReadPin(io_digital_channels[index].port, io_digital_channels[index].pin);
	}
	return GPIO_PIN_RESET;
}

/**
 * @Brief Writes a value to a registered digital output channel of a provided index.
 *
 * This function writes the provided value to a digital output.
 *
 * @param index: The index of the channel to write to (assigned in order of registration with io_digital_add_channel).
 * @param value: The GPIO_PinState to write to the output channel (GPIO_PIN_RESET, or GPIO_PIN_SET).
 */
void io_digital_write(int index, GPIO_PinState value) {
	if (index >= 0 && index < io_digital_channel_count && io_digital_channels[index].direction == IO_DIGITAL_OUTPUT) {
		HAL_GPIO_WritePin(io_digital_channels[index].port, io_digital_channels[index].pin, value);
	}
}
