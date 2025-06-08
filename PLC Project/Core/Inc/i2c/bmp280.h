#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>

// BMP280 I2C address (default 0x76 or 0x77 depending on SDO pin)
#define BMP280_I2C_ADDR  0x76

// BMP280 register addresses
#define BMP280_REG_TEMP_PRESS_DATA  0xF7
#define BMP280_REG_CALIB_DATA       0x88

// API
void BMP280_Init(void);
void BMP280_ReadRaw(int32_t* raw_temp, int32_t* raw_pres);
void BMP280_ReadCompensated(int32_t* temperature, uint32_t* pressure);

#endif
