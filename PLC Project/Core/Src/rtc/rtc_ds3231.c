#include "rtc/rtc_ds3231.h"

static uint8_t BCDToDecimal(uint8_t bcd) {
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static uint8_t DecimalToBCD(uint8_t dec) {
	return ((dec / 10) << 4) | (dec % 10);
}

HAL_StatusTypeDef DS3231_ReadTime(RTC_Time* time) {
	uint8_t rawData[7]; // 7 bytes
	uint8_t startRegister = 0x00;

	// Tell the DS3231 to start reading from register 0x00
	if (HAL_I2C_Master_Transmit(&hi2c1, DS3231_ADDRESS, &startRegister, 1, HAL_MAX_DELAY) != HAL_OK) {
		return HAL_ERROR;
	}

	// Read the 7 bytes of time data
	if (HAL_I2C_Master_Receive(&hi2c1, DS3231_ADDRESS, rawData, 7, HAL_MAX_DELAY) != HAL_OK) {
		return HAL_ERROR;
	}

	// Convert BCD to decimal
	time->seconds = BCDToDecimal(rawData[0] & 0x7F);
	time->minutes = BCDToDecimal(rawData[1]);
	time->hours = BCDToDecimal(rawData[2] & 0x3F); // 24-hour format
	time->day_of_week = BCDToDecimal(rawData[3]);
	time->day = BCDToDecimal(rawData[4]);
	time->month = BCDToDecimal(rawData[5] & 0x1F);
	time->year = BCDToDecimal(rawData[6]);

	return HAL_OK;
}

HAL_StatusTypeDef DS3231_SetTime(RTC_Time* time) {
	uint8_t rawData[7]; // 7 bytes
	uint8_t startRegister = 0x00;

	rawData[0] = DecimalToBCD(time->seconds);
	rawData[1] = DecimalToBCD(time->minutes);
	rawData[2] = DecimalToBCD(time->hours);
	rawData[3] = DecimalToBCD(time->day_of_week);
	rawData[4] = DecimalToBCD(time->day);
	rawData[5] = DecimalToBCD(time->month);
	rawData[6] = DecimalToBCD(time->year);

	if (HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, startRegister, 1, rawData, 7, HAL_MAX_DELAY) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

uint16_t DS3231_ReadTemp(void* context)
{
	if (context == NULL) return 0xFFFF; // error fallback
	I2C_HandleTypeDef* hi2c = (I2C_HandleTypeDef*)context;

	uint8_t temp_reg[2];
	if (HAL_I2C_Mem_Read(hi2c, 0x68 << 1, 0x11, I2C_MEMADD_SIZE_8BIT, temp_reg, 2, HAL_MAX_DELAY) != HAL_OK) {
	        return 0xFFFF; // error fallback
	};

	int8_t temperature_signed = (int8_t)temp_reg[0]; // MSB is signed
	if (temperature_signed < 0) {
		return 0; // Clamp to 0 if negative
	}

	return (uint16_t)temperature_signed;
}
