#include "rs485/rs485.h"

// Variables
static GPIO_TypeDef* RS485_DIR_PORT;
static uint16_t RS485_DIR_PIN;

extern UART_HandleTypeDef huart2;


void RS485_Setup(GPIO_TypeDef* dir_port, uint16_t dir_pin) {
	RS485_DIR_PORT = dir_port;
	RS485_DIR_PIN = dir_pin;
}

void RS485_SetTransmitMode(void) {
	// Set direction pin to HIGH (transmit)
	HAL_GPIO_WritePin(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_PIN_SET);
}

void RS485_SetReceiveMode(void) {
	// Set direction pin to LOW (receive)
	HAL_GPIO_WritePin(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_PIN_RESET);
}

void RS485_Transmit(uint8_t *data, uint16_t len) {
	// Set to transmit mode
	RS485_SetTransmitMode();

	// Transmit data
	HAL_Delay(1); // short delay
	HAL_UART_Transmit(&huart2, data, len, HAL_MAX_DELAY);
	HAL_Delay(1); // short delay

	// Revert mode ready to receive
	RS485_SetReceiveMode();
}
