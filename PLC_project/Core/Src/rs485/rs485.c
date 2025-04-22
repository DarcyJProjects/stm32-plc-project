// Sources
// https://deepbluembedded.com/how-to-receive-uart-serial-data-with-stm32-dma-interrupt-polling/
// https://deepbluembedded.com/stm32-uart-receive-unknown-length-idle-line-detection-examples/#stm32-uart-idle-line-detection-dma-example

#include "rs485/rs485.h"
#include "usb_serial.h" // debug only

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

// Variables
static GPIO_TypeDef* RS485_DIR_PORT;
static uint16_t RS485_DIR_PIN;

#define RS485_DMA_BUFFER_SIZE 256 // MODBUS max packet size
static uint8_t RS485_DMA_BUFFER[RS485_DMA_BUFFER_SIZE] = {0};
static volatile uint16_t RS485_ReceivedLength = 0;

static volatile bool RS485_TxInProgress = false;


void RS485_SetTransmitMode(void) {
	// Set direction pin to HIGH (transmit)
	HAL_GPIO_WritePin(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_PIN_SET);
}

void RS485_SetReceiveMode(void) {
	// Set direction pin to LOW (receive)
	HAL_GPIO_WritePin(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_PIN_RESET);
}

void RS485_Setup(GPIO_TypeDef* dir_port, uint16_t dir_pin) {
	RS485_TxInProgress = false;

	RS485_DIR_PORT = dir_port;
	RS485_DIR_PIN = dir_pin;

	RS485_SetReceiveMode();

	// Start DMA
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
}

void RS485_Transmit(uint8_t *data, uint16_t len) {
    if (RS485_TxInProgress) {
    	// TODO: Implement queue of transmits

    	return;
    }


	RS485_SetTransmitMode();

    RS485_TxInProgress = true;
    HAL_UART_Transmit_DMA(&huart2, data, len);
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
	if (huart->Instance == USART2) {
		RS485_ReceivedLength = size;

		// TODO: pass data to MODBUS stack for processing

		// DEBUG PRINT TO USB OVER SERIAL (remove in production)
		if (RS485_ReceivedLength < RS485_DMA_BUFFER_SIZE) {
			RS485_DMA_BUFFER[RS485_ReceivedLength] = '\0';
			usb_serial_println((char*)RS485_DMA_BUFFER);
		}

		// Ready for next reception
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
	}
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
    	RS485_TxInProgress = false;

        // NOTE: DO NOT PUT HAL_DELAYS IN THIS OR IT WILL LEAD TO ALL SORTS OF ISSUES!

        RS485_SetReceiveMode();

        // Restart DMA receive if needed
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
    }
}
