#include <io/io_input_reg.h>

// Allocates storage for all the channels.
IO_Input_Reg_Channel io_input_reg_channels[MAX_IO_INPUT_REG]; // Array of type channel (struct created in header)
uint16_t io_input_reg_channel_count = 0;
uint16_t io_input_adc_reg_channel_count = 0;
uint8_t is_calibrated = 0;

extern ADC_HandleTypeDef hadc1; // Declare external handle for ADC1

/**
 * @Brief Adds an input register channel to the system
 *
 * This function adds an input register to the system creating a channel which can
 * be used to read from. The channel number is incremented with each channel added.
 *
 * @param read_func: Function that reads the channels device, returning a value. For physical ADC input, use "adc_read_func". For I2C, it is device dependent.
 * @param context: Pointer passed to that read function. For physical ADC input, use "&hadcX" replacing X as necessary.
 */
void io_input_reg_add_channel(uint16_t (*read_func)(void*), void* context) {
	// Check is a physical ADC input channel is being added
	if (read_func == (void*)adc_read_func) {
		// Enforce limit only for physical ADC inputs
		if (io_input_adc_reg_channel_count >= MAX_IO_INPUT_REG_PHYSICAL) {
			return;
		} else {
			io_input_adc_reg_channel_count++; // increase physical ADC channel count
		}
	}

	io_input_reg_channels[io_input_reg_channel_count].read_func = read_func;
	io_input_reg_channels[io_input_reg_channel_count].context = context;
	io_input_reg_channel_count++; // increase overall channel count
}

/**
 * @Brief Reads the last set value from a registered holding register channel of a provided index.
 *
 * This function returns the last set value of a holding register. It reads with the channels set read_function and context.
 *
 * @param index: The index of the channel to read (assigned in order of registration with io_holding_reg_add_channel)
 *
 * @retval The ADC conversion result, or 0 if the channel is invalid.
 */
uint16_t io_input_reg_read(uint16_t index) {
	if (index < io_input_reg_channel_count) {
		return io_input_reg_channels[index].read_func(io_input_reg_channels[index].context);
	}
	return 0;
}


// Read function for physical adc channels, i2c is device dependent
uint16_t adc_read_func(uint32_t channel) {
#ifdef HAL_ADC_MODULE_ENABLED
		//ADC_HandleTypeDef* hadc = (ADC_HandleTypeDef*)context; // cast generic handle to ADC_HandleTypeDef
		ADC_HandleTypeDef* hadc = &hadc1;

		// Stop ADC before reconfiguration
    	HAL_ADC_Stop(&hadc1);

		// Configure the specified channel
		ADC_ChannelConfTypeDef sConfig = {0};
		sConfig.Channel = channel;
		sConfig.Rank = ADC_REGULAR_RANK_1; /* !!! WILL NOT WORK WITHOUT !! */
		sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5; // Better accuracy
		sConfig.SingleDiff = ADC_SINGLE_ENDED; /* !!! WILL NOT WORK WITHOUT !!! */
		HAL_ADC_ConfigChannel(hadc, &sConfig);

		// Start ADC conversion
		HAL_ADC_Start(hadc);

		// Wait until the ADC conversion is done (or a timeout of 100 ms occurs)
		if (HAL_ADC_PollForConversion(hadc, 100) == HAL_OK) {
			//HAL_ADC_Stop(hadc);
			// Return the ADC value
			return (HAL_ADC_GetValue(hadc) * 65535) / 4095; // scale 12 bit value to 16 bit value as expected by modbus specifications
		}
		HAL_ADC_Stop(hadc);
#endif
	return 0;
}
