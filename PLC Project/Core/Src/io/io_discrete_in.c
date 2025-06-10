#include <io/io_discrete_in.h>

// Allocates storage for all the IO channels.
IO_Discrete_In_Channel io_discrete_in_channels[MAX_IO_DISCRETE_IN]; // Array of type IO_Channel (struct created in header)
uint16_t io_discrete_in_channel_count = 0;
uint16_t io_hardware_discrete_in_channel_count = 0;


/**
 * @Brief Adds a discrete input to the system
 *
 * This function adds a discrete input to the system creating a channel which can
 * be used to read from. The channel number is incremented with each channel added.
 *
 * @param port: Pointer to the port where the coil pin is connected (e.g., GPIOA, GPIOB, GPIOC).
 * @param pin: Pin number of the GPIO port where the coil pin is connected.
 */
void io_discrete_in_add_channel(GPIO_PinState (*read_func)(void*), void* context) {
	// Check is a physical discrete input channel is being added
	if (read_func == hardware_discrete_in_read_func) {
		// Enforce limit only for physical discrete inputs
		if (io_hardware_discrete_in_channel_count >= MAX_IO_DISCRETE_IN) {
			return;
		} else {
			io_hardware_discrete_in_channel_count++; // increase physical discrete in channel count
		}
	}

	io_discrete_in_channels[io_discrete_in_channel_count].read_func = read_func;
	io_discrete_in_channels[io_discrete_in_channel_count].context = context;
	io_discrete_in_channel_count++; // increase overall channel count
}

/**
 * @Brief Reads the current value from a registered discrete input channel of a provided index.
 *
 * @param index: The index of the channel to read (assigned in order of registration with io_coil_add_channel)
 *
 * @retval The GPIO_PinState of the coil input, or GPIO_PIN_RESET if the channel index is invalid.
 */
GPIO_PinState io_discrete_in_read(uint16_t index) {
	if (index < io_discrete_in_channel_count) {
		return io_discrete_in_channels[index].read_func(io_discrete_in_channels[index].context);
	}
	return GPIO_PIN_RESET;
}


// Read function for physical discrete input channels, i2c is device dependent
GPIO_PinState hardware_discrete_in_read_func(void* context) {
	gpio_config* gpio = (gpio_config*)context;
	return HAL_GPIO_ReadPin(gpio->port, gpio->pin);
}











