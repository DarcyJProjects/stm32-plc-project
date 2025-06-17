#include <io/io_holding_reg.h>

// Allocates storage for all the channels.
IO_Holding_Reg_Channel io_holding_reg_channels[MAX_IO_HOLDING_REG]; // Array of type channel (struct created in header)
uint16_t io_holding_reg_channel_count = 0;
uint16_t io_holding_adc_reg_channel_count = 0;

extern DAC_HandleTypeDef hdac1; // Declare external handle for DAC1


void io_holding_reg_add_channel(void (*write_func)(void*, uint16_t), void* context) {
	// Check is a physical ADC output channel is being added
	if (write_func == (void*)dac_write_func) {
		// Enforce limit only for physical ADC outputs
		if (io_holding_adc_reg_channel_count >= MAX_IO_HOLDING_REG) {
			return;
		} else {
			io_holding_adc_reg_channel_count++; // increase physical ADC channel count
		}
	}

	io_holding_reg_channels[io_holding_reg_channel_count].write_func = write_func;
	io_holding_reg_channels[io_holding_reg_channel_count].context = context;
	io_holding_reg_channels[io_holding_reg_channel_count].storedValue = 0;
	io_holding_reg_channel_count++; // increase overall channel count
}



uint16_t io_holding_reg_read(uint16_t index) {
	if (index < io_holding_reg_channel_count) {
		return io_holding_reg_channels[index].storedValue;
	}
	return 0;
}



void io_holding_reg_write(uint16_t index, uint16_t value) {
	if (index < io_holding_reg_channel_count) {
		io_holding_reg_channels[index].write_func(io_holding_reg_channels[index].context, value);
		io_holding_reg_channels[index].storedValue = value;
	}
}


void dac_write_func(uint32_t channel, uint16_t value) {
#ifdef HAL_DAC_MODULE_ENABLED
		DAC_HandleTypeDef* hdac = &hdac1;

		// Scale modbus 16 bit value to 12 bit DAC range
		uint32_t scaledValue = (value * 4095U) / 65535U;

		// Start DAC conversion (DAC peripheral, DAC channel, data alignment, value)
		HAL_DAC_SetValue(hdac, channel, DAC_ALIGN_12B_R, scaledValue);

		// Enable the DAC channel and apply the value to the pin
		HAL_DAC_Start(hdac, channel);
#endif
}
