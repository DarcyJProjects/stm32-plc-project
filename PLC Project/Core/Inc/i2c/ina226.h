#ifndef INA226_H
#define INA226_H

#include "stm32g4xx_hal.h"

#define INA226_I2C_ADDR 		0x40

// Register Addresses
#define INA226_REG_CONFIG		0x00
#define INA226_REG_SHUNT_VOLT	0x01
#define INA226_REG_BUS_VOLT		0x02
#define INA226_REG_POWER		0x03
#define INA226_REG_CURRENT		0x04
#define INA226_REG_CALIBRATION	0x05
#define INA226_REG_MASK_ENABLE	0x06
#define INA226_REG_ALERT_LIMIT	0x07
#define INA226_REG_MANUFACTURER	0xFE
#define INA226_REG_DIE_ID		0xFF

// Configuration
#define INA226_CONFIG_DEFAULT	0x4127 // Reset, 16-sample averaging, 1.1ms conversion time, continuous shunt and bus voltage
#define INA226_CALIBRATION_VALUE 0x0155 // Based on shunt resistor and max current [CAL = 0.00512/(Current_LSB * R_shunt)	where Current_LSB = I_MAX/2^15]

// Functions
void INA226_Init(I2C_HandleTypeDef *hi2c);

// Bus Voltage in Volts
uint16_t INA226_ReadBusVoltageRaw(void* context);
float INA226_ReadBusVoltage(void);

// Current in Amps
uint16_t INA226_ReadCurrentRaw(void* context);
float INA226_ReadCurrent(void);

// Power in Watts
uint16_t INA226_ReadPowerRaw(void* context);
float INA226_ReadPower(void);

#endif
