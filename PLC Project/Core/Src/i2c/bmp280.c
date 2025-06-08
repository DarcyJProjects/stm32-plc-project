#include "i2c/bmp280.h"
#include "i2c/i2c.h" //

// Calibration coefficients
static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;

// Internal fine temperature for pressure compensation
static int32_t t_fine;

void BMP280_Init(void) {
    uint8_t calib[24];
    I2C_Read(calib, BMP280_I2C_ADDR, BMP280_REG_CALIB_DATA, 24);

    dig_T1 = (uint16_t)(calib[1] << 8 | calib[0]);
    dig_T2 = (int16_t)(calib[3] << 8 | calib[2]);
    dig_T3 = (int16_t)(calib[5] << 8 | calib[4]);

    dig_P1 = (uint16_t)(calib[7] << 8 | calib[6]);
    dig_P2 = (int16_t)(calib[9] << 8 | calib[8]);
    dig_P3 = (int16_t)(calib[11] << 8 | calib[10]);
    dig_P4 = (int16_t)(calib[13] << 8 | calib[12]);
    dig_P5 = (int16_t)(calib[15] << 8 | calib[14]);
    dig_P6 = (int16_t)(calib[17] << 8 | calib[16]);
    dig_P7 = (int16_t)(calib[19] << 8 | calib[18]);
    dig_P8 = (int16_t)(calib[21] << 8 | calib[20]);
    dig_P9 = (int16_t)(calib[23] << 8 | calib[22]);
}

void BMP280_ReadRaw(int32_t* raw_temp, int32_t* raw_pres) {
    uint8_t data[6];
    I2C_Read(data, BMP280_I2C_ADDR, BMP280_REG_TEMP_PRESS_DATA, 6);

    *raw_pres = (int32_t)(data[0] << 12 | data[1] << 4 | data[2] >> 4);
    *raw_temp = (int32_t)(data[3] << 12 | data[4] << 4 | data[5] >> 4);
}

void BMP280_ReadCompensated(int32_t* temperature, uint32_t* pressure) {
    int32_t raw_temp, raw_pres;
    BMP280_ReadRaw(&raw_temp, &raw_pres);

    // Temperature compensation
    int32_t var1 = ((((raw_temp >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((raw_temp >> 4) - ((int32_t)dig_T1)) *
                      ((raw_temp >> 4) - ((int32_t)dig_T1))) >> 12) *
                    ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    *temperature = (t_fine * 5 + 128) >> 8;

    // Pressure compensation
    int64_t var3 = ((int64_t)t_fine) - 128000;
    int64_t var4 = var3 * var3 * (int64_t)dig_P6;
    var4 += ((var3 * (int64_t)dig_P5) << 17);
    var4 += (((int64_t)dig_P4) << 35);
    var3 = ((var3 * var3 * (int64_t)dig_P3) >> 8) + ((var3 * (int64_t)dig_P2) << 12);
    var3 = (((((int64_t)1) << 47) + var3)) * ((int64_t)dig_P1) >> 33;

    if (var3 == 0) {
        *pressure = 0;
        return;
    }

    int64_t p = 1048576 - raw_pres;
    p = (((p << 31) - var4) * 3125) / var3;
    var3 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    int64_t var5 = (((int64_t)dig_P8) * p) >> 19;

    p = ((p + var3 + var5) >> 8) + (((int64_t)dig_P7) << 4);
    *pressure = (uint32_t)p;
}
