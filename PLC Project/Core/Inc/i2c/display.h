#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32g4xx_hal.h"
#include <stdio.h>

void display_Setup();

void display_Boot(void);

void display_StatusPage(void);

void display_FactoryResetPage(uint8_t page);

void display_BtnPress(void);

void display_setPage(uint16_t page);

#endif
