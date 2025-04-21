#ifndef RS485_H
#define RS485_H

#include "stm32g4xx_hal.h"

void RS485_Setup(GPIO_TypeDef* dir_port, uint16_t dir_pin);

void RS485_SetTransmitMode(void);

void RS485_SetReceiveMode(void);

void RS485_Transmit(uint8_t* data, uint16_t len);

void RS485_Receive(uint8_t* buffer, uint16_t len);

#endif
