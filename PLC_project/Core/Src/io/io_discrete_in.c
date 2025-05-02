#include <io/io_discrete_in.h>

// Allocates storage for all the IO channels.
IO_Discrete_In_Channel io_discrete_in_channels[MAX_IO_DISCRETE_IN]; // Array of type IO_Channel (struct created in header)
uint16_t io_discrete_in_channel_count = 0;

/**
 * @Brief Adds a discrete input to the system
 *
 * This function adds a discrete input to the system creating a channel which can
 * be used to read from. The channel number is incremented with each channel added.
 *
 * @param port: Pointer to the port where the coil pin is connected (e.g., GPIOA, GPIOB, GPIOC).
 * @param pin: Pin number of the GPIO port where the coil pin is connected.
 */
void io_discrete_in_add_channel(GPIO_TypeDef* port, uint16_t pin) {
	if (io_discrete_in_channel_count >= MAX_IO_DISCRETE_IN) return; // Cannot add another channel if all channels taken

	io_discrete_in_channels[io_discrete_in_channel_count].port = port;
	io_discrete_in_channels[io_discrete_in_channel_count].pin = pin;
	io_discrete_in_channel_count++;
}

/**
 * @Brief Reads the current value from a registered discrete input channel of a provided index.
 *
 * @param index: The index of the channel to read (assigned in order of registration with io_coil_add_channel)
 *
 * @retval The GPIO_PinState of the coil input, or 0 if the channel index is invalid.
 */
GPIO_PinState io_discrete_in_read(uint16_t index) {
	if (index >= 0 && index < io_discrete_in_channel_count) {
		return HAL_GPIO_ReadPin(io_discrete_in_channels[index].port, io_discrete_in_channels[index].pin);
	}
	return GPIO_PIN_RESET;
}
