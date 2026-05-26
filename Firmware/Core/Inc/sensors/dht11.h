#ifndef DHT11_H
#define DHT11_H

#include "stm32g4xx_hal.h"

// Emum to describe read status
typedef enum {
	DHT11_OK = 0,
	DHT11_ERROR_TIMEOUT,
	DHT11_ERROR_CHECKSUM,
	DHT11_ERROR_NO_RESPONSE
} DHT11_Response;

// --- Functions ---
// Defines the GPIO pin and port for the DHT11
void DHT11_Setup(GPIO_TypeDef* port, uint16_t pin);

// Initiates and reads a response from the DHT11
DHT11_Response DHT11_Read(uint8_t* temperature_int, uint8_t* temperature_dec, uint8_t* humidity_int, uint8_t* humidity_dec);

uint16_t DHT11_read_func(uint16_t temp); // for io

#endif
