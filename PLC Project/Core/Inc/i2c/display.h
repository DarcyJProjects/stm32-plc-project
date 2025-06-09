#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32g4xx_hal.h"
#include <stdio.h>

void display_Setup(GPIO_TypeDef* btn_port, uint16_t btn_pin);

void display_Boot(void);

void display_StatusPage(void);

#endif
