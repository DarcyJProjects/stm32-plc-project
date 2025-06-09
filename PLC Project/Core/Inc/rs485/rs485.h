#ifndef RS485_H
#define RS485_H

#include "stm32g4xx_hal.h"
#include <stdbool.h>
#include "modbus/modbus.h" // for relaying received frames to modbus

void RS485_Setup(GPIO_TypeDef* dir_port, uint16_t dir_pin);

void RS485_SetTransmitMode(void);

void RS485_SetReceiveMode(void);

void RS485_Transmit(uint8_t* data, uint16_t len);

void RS485_TCCallback(void);

void RS485_ProcessPendingFrames(void);

void RS485_TransmitPendingFrames(void);

#endif
