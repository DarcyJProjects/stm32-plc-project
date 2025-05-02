#include <io/io_analogue.h>

// Allocates storage for all the IO channels.
IO_Analogue_Channel io_analogue_channels[MAX_IO_ANALOGUE]; // Array of type IO_Channel (struct created in header)
int io_analogue_channel_count = 0;

/**
 * @Brief Adds an analogue input or output channel to the system
 *
 * This function adds an analogue input or output to the system creating a channel which can
 * be used to read from or write to. The channel number is incremented with each channel added.
 *
 * @param handle: Pointer to generic ADC or DAC handle.
 * @param channel: ADC or DAC channel number (e.g., ADC_CHANNEL_4)
 * @param dir: Direction of the channel (IO_ANALOGUE_INPUT or IO_ANALOGUE_OUTPUT)
 */
void io_analogue_add_channel(void* handle, uint32_t channel, IO_Analogue_Direction dir) {
	if (io_analogue_channel_count >= MAX_IO_ANALOGUE) return; // Cannot add another channel if all channels taken

	io_analogue_channels[io_analogue_channel_count].handle = handle;
	io_analogue_channels[io_analogue_channel_count].channel = channel;
	io_analogue_channels[io_analogue_channel_count].direction = dir;
	io_analogue_channel_count++;
}

/**
 * @Brief Reads the current value from a registered analogue input channel of a provided index.
 *
 * This function reads the value of an analogue input. It starts an ADC conversion and returns the digital value (0-4095)
 *
 * @param index: The index of the channel to read (assigned in order of registration with io_analogue_add_channel)
 *
 * @retval The ADC conversion result, or 0 if the channel is invalid or not an input channel.
 */
uint32_t io_analogue_read(int index) {
	if (index >= 0 && index < io_analogue_channel_count && io_analogue_channels[index].direction == IO_ANALOGUE_INPUT) {
#ifdef HAL_ADC_MODULE_ENABLED
		ADC_HandleTypeDef* hadc = (ADC_HandleTypeDef*)io_analogue_channels[index].handle; // cast generic handle to ADC_HandleTypeDef
		// Start ADC conversion
		HAL_ADC_Start(io_analogue_channels[index].hadc);
		// Wait until the AD conversion is done (or a timeout of 100 ms occurs)
		if (HAL_ADC_PollForConversion(io_analogue_channels[index].hadc, 100) == HAL_OK) {
			// Return the ADC value
			return HAL_ADC_GetValue(io_analogue_channels[index].hadc);
		}
#endif
	}
	return 0;
}

/**
 * @Brief Writes a value to a registered analogue output channel of a provided index.
 *
 * This function writes the provided value to an analogue output. It sets the DAC output voltage based on the provided value (e.g., 0-4095).
 *
 * @param index: The index of the channel to write to (assigned in order of registration with io_analogue_add_channel).
 * @param value: The value to write to the output channel.
 */
void io_analogue_write(int index, uint32_t value) {
	if (index >= 0 && index < io_analogue_channel_count && io_analogue_channels[index].direction == IO_ANALOGUE_OUTPUT) {
#ifdef HAL_DAC_MODULE_ENABLED
		DAC_HandleTypeDef* hdac = (DAC_HandleTypeDef*)io_analogue_channels[index].handle; // cast generic handle to DAC_HandleTypeDef

		// Start DAC conversion (DAC peripheral, DAC channel, data alignment, value)
		HAL_DAC_SetValue(hdac, io_analogue_channels[index].channel, DAC_ALIGN_12B_R, value);
		// Enable the DAC channel and apply the value to the pin
		HAL_DAC_Start(hdac, io_analogue_channels[index].channel);
#endif
	}
}
