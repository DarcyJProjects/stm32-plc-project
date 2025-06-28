#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>

// BMP280 I2C address (0x76 if SDO pulled LOW, 0x77 if SDO pulled HIGH)
#define BMP280_REG_CTRL_MEAS       0xF4
#define BMP280_OSRS_T              (1 << 5) // oversampling x1 temperature
#define BMP280_OSRS_P              (1 << 2) // oversampling x1 pressure
#define BMP280_MODE_NORMAL         0x03
#define BMP280_I2C_ADDR            0x77      // adjust if needed
#define BMP280_REG_CALIB_DATA      0x88
#define BMP280_REG_TEMP_PRESS_DATA 0xF7

// API
void BMP280_Init(void);
void BMP280_ReadRaw(int32_t* raw_temp, int32_t* raw_pres);
void BMP280_ReadCompensated(int32_t* temperature, uint32_t* pressure);

uint16_t BMP280_Read_Temp_Func(void* context);

#endif
