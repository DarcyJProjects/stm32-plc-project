#include "i2c/bmp280.h"
#include "i2c/i2c.h"
#include <stdio.h>
#include "usb_serial.h"

// Calibration coefficients
static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;

// Internal fine temperature for pressure compensation
static int32_t t_fine;

//#define PRINT_DEBUG

// Little-endian unpack helpers
#define U16_LE(buf, i) ((uint16_t)((buf)[(i)] | ((buf)[(i) + 1] << 8)))
#define S16_LE(buf, i) ((int16_t)((buf)[(i)] | ((buf)[(i) + 1] << 8)))

void BMP280_Init(void) {
    uint8_t calib[24];

    I2C_Read(calib, BMP280_I2C_ADDR, BMP280_REG_CALIB_DATA, 24);

#ifdef PRINT_DEBUG
    usb_serial_println("BMP280 reading calibration data...");
#endif

    dig_T1 = U16_LE(calib, 0);
    dig_T2 = S16_LE(calib, 2);
    dig_T3 = S16_LE(calib, 4);

    dig_P1 = U16_LE(calib, 6);
    dig_P2 = S16_LE(calib, 8);
    dig_P3 = S16_LE(calib, 10);
    dig_P4 = S16_LE(calib, 12);
    dig_P5 = S16_LE(calib, 14);
    dig_P6 = S16_LE(calib, 16);
    dig_P7 = S16_LE(calib, 18);
    dig_P8 = S16_LE(calib, 20);
    dig_P9 = S16_LE(calib, 22);

#ifdef PRINT_DEBUG
    char msg[128];
    snprintf(msg, sizeof(msg),
             "Calib: T1=%u T2=%d T3=%d P1=%u P2=%d P3=%d P4=%d P5=%d P6=%d P7=%d P8=%d P9=%d",
             dig_T1, dig_T2, dig_T3, dig_P1, dig_P2, dig_P3, dig_P4,
             dig_P5, dig_P6, dig_P7, dig_P8, dig_P9);
    usb_serial_println(msg);
#endif

    // Start measurement mode (normal, oversampling x1 for T and P)
    uint8_t ctrl_meas = BMP280_OSRS_T | BMP280_OSRS_P | BMP280_MODE_NORMAL;
    uint8_t buf[2] = {BMP280_REG_CTRL_MEAS, ctrl_meas};
    HAL_StatusTypeDef status = I2C_Transmit(BMP280_I2C_ADDR << 1, buf, 2);
#ifdef PRINT_DEBUG
    if (status != HAL_OK) {
        char msg[32];
        snprintf(msg, sizeof(msg), "BMP280 ctrl_meas write failed: %d", status);
        usb_serial_println(msg);
    } else {
        usb_serial_println("BMP280 measurement mode set");
    }
#endif

    // Delay for sensor to start measurements (implement your own delay)
    HAL_Delay(10);
}

void BMP280_ReadRaw(int32_t* raw_temp, int32_t* raw_pres) {
    uint8_t data[6];

    I2C_Read(data, BMP280_I2C_ADDR, BMP280_REG_TEMP_PRESS_DATA, 6);

#ifdef PRINT_DEBUG
    char msg[64];
    snprintf(msg, sizeof(msg), "Raw data: %02X %02X %02X %02X %02X %02X",
             data[0], data[1], data[2], data[3], data[4], data[5]);
    usb_serial_println(msg);
#endif

    *raw_pres = (int32_t)((data[0] << 12) | (data[1] << 4) | (data[2] >> 4));
    *raw_temp = (int32_t)((data[3] << 12) | (data[4] << 4) | (data[5] >> 4));
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

uint16_t BMP280_Read_Temp_Func(void* context) {
    (void)context;

    int32_t temperature;
    uint32_t pressure;
    BMP280_ReadCompensated(&temperature, &pressure);

#ifdef PRINT_DEBUG
    char msg[32];
    snprintf(msg, sizeof(msg), "Temp: %ld.%02ld °C", (long)(temperature / 100), (long)(temperature % 100));
    usb_serial_println(msg);
#endif

    return (temperature < 0) ? 0 : (uint16_t)(temperature / 100);
}
