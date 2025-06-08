#include <io/io_coils.h>

// Allocates storage for all the IO channels.
IO_Coil_Channel io_coil_channels[MAX_IO_COILS]; // Array of type IO_Channel (struct created in header)
uint16_t io_coil_channel_count = 0;

/**
 * @Brief Adds a coil output channel to the system
 *
 * This function adds a coil output to the system creating a channel which can
 * be used to read from or write to. The channel number is incremented with each channel added.
 *
 * @param port: Pointer to the port where the coil pin is connected (e.g., GPIOA, GPIOB, GPIOC).
 * @param pin: Pin number of the GPIO port where the coil pin is connected.
 * @param dir: Direction of the channel (IO_COIL_INPUT or IO_COIL_OUTPUT)
 */
void io_coil_add_channel(GPIO_TypeDef* port, uint16_t pin) {
	if (io_coil_channel_count >= MAX_IO_COILS) return; // Cannot add another channel if all channels taken

	io_coil_channels[io_coil_channel_count].port = port;
	io_coil_channels[io_coil_channel_count].pin = pin;
	io_coil_channels[io_coil_channel_count].storedState = GPIO_PIN_RESET; // default to OFF = RESET
	io_coil_channel_count++;
}

/**
 * @Brief Reads the current value from a registered coil input channel of a provided index.
 *
 * @param index: The index of the channel to read (assigned in order of registration with io_coil_add_channel)
 *
 * @retval The GPIO_PinState of the coil input, or 0 if the channel index is invalid.
 */
GPIO_PinState io_coil_read(uint16_t index) {
	if (index >= 0 && index < io_coil_channel_count) {
		return io_coil_channels[index].storedState;
	}
	return GPIO_PIN_RESET;
}

/**
 * @Brief Writes a value to a registered coil output channel of a provided index.
 *
 * This function writes the provided value to a coil output.
 *
 * @param index: The index of the channel to write to (assigned in order of registration with io_coil_add_channel).
 * @param value: The GPIO_PinState to write to the output channel (GPIO_PIN_RESET, or GPIO_PIN_SET).
 */
void io_coil_write(uint16_t index, GPIO_PinState value) {
	if (index >= 0 && index < io_coil_channel_count) {
		HAL_GPIO_WritePin(io_coil_channels[index].port, io_coil_channels[index].pin, value);
		io_coil_channels[index].storedState = value;
	}
}
