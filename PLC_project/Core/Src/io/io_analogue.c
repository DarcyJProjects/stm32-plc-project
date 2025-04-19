#include <io/io_analogue.h>

// Allocates storage for all the IO channels.
IO_Analogue_Channel io_analogue_channels[MAX_IO_ANALOGUE]; // Array of type IO_Channel (struct created in header)
int io_analogue_channel_count = 0;

/**
 * Adds an analogue input or output channel to the system
 *
 * hadc : Pointer to the ADC handle (used for inputs). Pass NULL if unused
 * hdac : Pointer to the DAC handle (used for outputs). Pass NULL if unused
 * channel : ADC or DAC channel number (e.g., ADC_CHANNEL_4)
 * dir : Direction of the channel: IO_ANALOGUE_INPUT or IO_ANALOGUE_OUTPUT
 */
void io_analogue_add_channel(ADC_HandleTypeDef* hadc, DAC_HandleTypeDef* hdac, uint32_t channel, IO_Analogue_Direction dir) {
	if (io_analogue_channel_count >= MAX_IO_ANALOGUE) return; // Cannot add another channel if all channels taken

	io_analogue_channels[io_analogue_channel_count].hadc = hadc;
	io_analogue_channels[io_analogue_channel_count].hdac = hdac;
	io_analogue_channels[io_analogue_channel_count].channel = channel;
	io_analogue_channels[io_analogue_channel_count].direction = dir;
	io_analogue_channel_count++;
}

/**
 * Reads the current value from a registered analogue input channel of a provided index
 *
 * Starts an ADC conversion and returns the digital value (0-4095)
 *
 * index : Index of the channel to read (assigned in order of registration with io_analogue_add_channel)
 * return uint32_t : The ADC conversion result, or 0 is the channel is invalid or not an input channel.
 */
uint32_t io_analogue_read(int index) {
	if (index >= 0 && index < io_analogue_channel_count && io_analogue_channels[index].direction == IO_ANALOGUE_INPUT) {
		// Start ADC conversion
		HAL_ADC_Start(io_analogue_channels[index].hadc);
		// Wait until the AD conversion is done (or a timeout of 100 ms occurs)
		if (HAL_ADC_PollForConversion(io_analogue_channels[index].hadc, 100) == HAL_OK) {
			// Return the ADC value
			return HAL_ADC_GetValue(io_analogue_channels[index].hadc);
		}
	}
	return 0;
}

/**
 * Writes a value to a registered analogue output channel.
 *
 * Sets the DAC output voltage based on the provided digital value (e.g., 0-4095)
 *
 * index : Index of the channel to write to
 * value : The 12-bit digital value to output via the DAC.
 */
void io_analogue_write(int index, uint32_t value) {
	if (index >= 0 && index < io_analogue_channel_count && io_analogue_channels[index].direction == IO_ANALOGUE_OUTPUT) {
		// Start DAC conversion (DAC peripheral, DAC channel, data alignment, value)
		HAL_DAC_SetValue(io_analogue_channels[index].hdac, io_analogue_channels[index].channel, DAC_ALIGN_12B_R, value);
		// Enable the DAC channel and apply the value to the pin
		HAL_DAC_Start(io_analogue_channels[index].hdac, io_analogue_channels[index].channel);
	}
}
