#ifndef IO_VIRTUAL_H
#define IO_VIRTUAL_H

#include "stm32g4xx_hal.h"
#include <stdbool.h> // Lets us use boolean logic
#include "automation/automation.h" // to use the RegisterType enumeration

#define MAX_VIRTUAL_COILS 128
#define MAX_VIRTUAL_HOLDING_REG 128

typedef enum {
	VIR_COIL, // digital virtual registers
	VIR_HOLDING // analogue virtual registers (16 bit)
} VirtualRegisterType;

typedef struct {
	uint8_t storedValue; // 0 or 1
} Virtual_Coil_Channel;

typedef struct {
	uint16_t storedValue; // 16 bit
} Virtual_Holding_Reg_Channel;

bool io_virtual_add(VirtualRegisterType type);

bool io_virtual_get_count(VirtualRegisterType type, uint16_t* count);

bool io_virtual_read(VirtualRegisterType type, uint16_t index, uint16_t* value);

bool io_virtual_write(VirtualRegisterType type, uint16_t index, uint16_t value);

bool io_virtual_save(uint16_t baseAddress);

bool io_virtual_load(uint16_t baseAddress);

#endif
