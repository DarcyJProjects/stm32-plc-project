#include <io/io_holding_reg.h>

// Allocates storage for all the channels.
IO_Holding_Reg_Channel io_holding_reg_channels[MAX_IO_HOLDING_REG]; // Array of type channel (struct created in header)
uint16_t io_holding_reg_channel_count = 0;

/**
 * @Brief Adds a holding register input channel to the system
 *
 * This function adds a holding register to the system creating a channel which can
 * be used to read from or write to. The channel number is incremented with each channel added.
 *
 * @param handle: Pointer to generic DAC handle.
 * @param channel: DAC channel number (e.g., DAC_CHANNEL_1)
 */
void io_holding_reg_add_channel(void* handle, uint32_t channel) {
	if (io_holding_reg_channel_count >= MAX_IO_HOLDING_REG) return; // Cannot add another channel if all channels taken

	io_holding_reg_channels[io_holding_reg_channel_count].handle = handle;
	io_holding_reg_channels[io_holding_reg_channel_count].channel = channel;
	io_holding_reg_channels[io_holding_reg_channel_count].storedValue = 0;
	io_holding_reg_channel_count++;
}

/**
 * @Brief Reads the last set value from a registered holding register channel of a provided index.
 *
 * This function returns the last set value of a holding register.
 *
 * @param index: The index of the channel to read (assigned in order of registration with io_holding_reg_add_channel)
 *
 * @retval The last written value for the holding register, or 0 if the channel is invalid.
 */
uint16_t io_holding_reg_read(uint16_t index) {
	if (index >= 0 && index < io_holding_reg_channel_count) {
		return io_holding_reg_channels[index].storedValue;
	}
	return 0;
}

/**
 * @Brief Writes a value to a registered holding register channel of a provided index.
 *
 * This function writes the provided value to a holding register. It sets the DAC output voltage based on the provided value (e.g., 0-4095).
 *
 * @param index: The index of the channel to write to (assigned in order of registration with io_holding_reg_add_channel).
 * @param value: The value to write to the holding register.
 */
void io_holding_reg_write(uint16_t index, uint16_t value) {
	if (index >= 0 && index < io_holding_reg_channel_count) {
#ifdef HAL_DAC_MODULE_ENABLED
		DAC_HandleTypeDef* hdac = (DAC_HandleTypeDef*)io_holding_reg_channels[index].handle; // cast generic handle to DAC_HandleTypeDef

		// Scale modbus 16 bit value to 12 bit DAC range
		uint32_t scaledValue = (value * 4095U) / 65535U;

		// Start DAC conversion (DAC peripheral, DAC channel, data alignment, value)
		HAL_DAC_SetValue(hdac, io_holding_reg_channels[index].channel, DAC_ALIGN_12B_R, scaledValue);

		// Enable the DAC channel and apply the value to the pin
		// TODO: Move to init so its only ran once. doesnt need to be run each time
		HAL_DAC_Start(hdac, io_holding_reg_channels[index].channel);

		// Store the set value
		io_holding_reg_channels[index].storedValue = value;
#endif
	}
}
