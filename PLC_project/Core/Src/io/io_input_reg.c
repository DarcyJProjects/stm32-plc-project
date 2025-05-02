#include <io/io_input_reg.h>

// Allocates storage for all the channels.
IO_Input_Reg_Channel io_input_reg_channels[MAX_IO_INPUT_REG]; // Array of type channel (struct created in header)
uint16_t io_input_reg_channel_count = 0;

/**
 * @Brief Adds an input register channel to the system
 *
 * This function adds an input register to the system creating a channel which can
 * be used to read from. The channel number is incremented with each channel added.
 *
 * @param handle: Pointer to generic ADC handle.
 * @param channel: ADC channel number (e.g., ADC_CHANNEL_4)
 */
void io_input_reg_add_channel(void* handle, uint32_t channel) {
	if (io_input_reg_channel_count >= MAX_IO_INPUT_REG) return; // Cannot add another channel if all channels taken

	io_input_reg_channels[io_input_reg_channel_count].handle = handle;
	io_input_reg_channels[io_input_reg_channel_count].channel = channel;
	io_input_reg_channel_count++;
}

/**
 * @Brief Reads the last set value from a registered holding register channel of a provided index.
 *
 * This function returns the last set value of a holding register. It starts an ADC conversion.
 *
 * @param index: The index of the channel to read (assigned in order of registration with io_holding_reg_add_channel)
 *
 * @retval The ADC conversion result, or 0 if the channel is invalid.
 */
uint16_t io_input_reg_read(uint16_t index) {
	if (index >= 0 && index < io_input_reg_channel_count) {
#ifdef HAL_ADC_MODULE_ENABLED
		ADC_HandleTypeDef* hadc = (ADC_HandleTypeDef*)io_input_reg_channels[index].handle; // cast generic handle to ADC_HandleTypeDef
		// Start ADC conversion
		HAL_ADC_Start(hadc);
		// Wait until the ADC conversion is done (or a timeout of 100 ms occurs)
		if (HAL_ADC_PollForConversion(hadc, 100) == HAL_OK) {
			// Return the ADC value
			return (HAL_ADC_GetValue(hadc) * 65535) / 4095; // scale 12 bit value to 16 bit value as expected by modbus specifications
		}
#endif
	}
	return 0;
}
