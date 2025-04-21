// Sources
// https://deepbluembedded.com/how-to-receive-uart-serial-data-with-stm32-dma-interrupt-polling/
// https://deepbluembedded.com/stm32-uart-receive-unknown-length-idle-line-detection-examples/#stm32-uart-idle-line-detection-dma-example

#include "rs485/rs485.h"
#include "usb_serial.h" // debug only

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;

// Variables
static GPIO_TypeDef* RS485_DIR_PORT;
static uint16_t RS485_DIR_PIN;

#define RS485_DMA_BUFFER_SIZE 64

static uint8_t RS485_DMA_BUFFER[RS485_DMA_BUFFER_SIZE] = {0};
static volatile uint16_t RS485_ReceivedLength = 0;


void RS485_SetTransmitMode(void) {
	// Set direction pin to HIGH (transmit)
	HAL_GPIO_WritePin(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_PIN_SET);
}

void RS485_SetReceiveMode(void) {
	// Set direction pin to LOW (receive)
	HAL_GPIO_WritePin(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_PIN_RESET);
}

void RS485_Setup(GPIO_TypeDef* dir_port, uint16_t dir_pin) {
	RS485_DIR_PORT = dir_port;
	RS485_DIR_PIN = dir_pin;

	RS485_SetReceiveMode();

	// Start DMA
	//HAL_UART_Receive_DMA(&huart2, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);

	// Enable IDLE interrupt
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
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

void RS485_Receive(uint8_t* buffer, uint16_t len) {
	// Set to receive mode
	RS485_SetReceiveMode();

	// Receive data
	HAL_UART_Receive(&huart2, buffer, len, HAL_MAX_DELAY);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
	RS485_ReceivedLength = size;

	// DEBUG PRINT TO USB OVER SERIAL
	if (RS485_ReceivedLength < RS485_DMA_BUFFER_SIZE) {
		RS485_DMA_BUFFER[RS485_ReceivedLength] = '\0';
	}
	usb_serial_println((char*)RS485_DMA_BUFFER);

	// Ready for next reception
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
}
