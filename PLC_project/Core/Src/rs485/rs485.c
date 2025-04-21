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

static uint8_t* RS485_TXBuffer = NULL;
static uint16_t RS485_TXBufferLen = 0;
static volatile bool RS485_TXBusy = false;

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
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
}

void RS485_Transmit(uint8_t *data, uint16_t len) {
	if (RS485_TXBusy) return; // return to prevent overlapping TX
	// Note may need to implement a queue of transmits?

	RS485_TXBusy = true;
	RS485_TXBuffer = data;
	RS485_TXBufferLen = len;

	// Set to transmit mode
	RS485_SetTransmitMode();

	// Start interrupt based transmit
	HAL_UART_Transmit_IT(&huart2, RS485_TXBuffer, RS485_TXBufferLen);
}

void RS485_Receive(uint8_t* buffer, uint16_t len) {
	// Set to receive mode
	RS485_SetReceiveMode();

	// Receive data
	HAL_UART_Receive(&huart2, buffer, len, HAL_MAX_DELAY);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
	if (huart->Instance == USART2) {
		RS485_ReceivedLength = size;

		// DEBUG PRINT TO USB OVER SERIAL
		if (RS485_ReceivedLength < RS485_DMA_BUFFER_SIZE) {
			RS485_DMA_BUFFER[RS485_ReceivedLength] = '\0';
		}
		usb_serial_println((char*)RS485_DMA_BUFFER);

		// Ready for next reception
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		// Transmission complete, revert to receive mode
		RS485_SetReceiveMode();
		RS485_TXBusy = false;
	}
}
