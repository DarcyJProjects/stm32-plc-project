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

    if (huart2.gState != HAL_UART_STATE_READY || huart2.ErrorCode != HAL_UART_ERROR_NONE) {
    	// Reset UART
		HAL_UART_Abort(&huart2);
		HAL_UART_Init(&huart2);
	}

    RS485_TxInProgress = true;
	RS485_SetTransmitMode();

	// Enable TC interrupt
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);

	// Transmit and store the status of it
	HAL_StatusTypeDef transmitStatus = HAL_UART_Transmit_DMA(&huart2, data, len);
	if (transmitStatus != HAL_OK) {
		// UART TX DMA Error - switch back to receiving
		RS485_SetReceiveMode();
		RS485_TxInProgress = false;
		__HAL_UART_DISABLE_IT(&huart2, UART_IT_TC);
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
	}
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
    	// Check if this is a TC interrupt (not DMA completion)
    	if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC)) {
    		// Clear TC interrupt
    		__HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_TC);
    		__HAL_UART_DISABLE_IT(&huart2, UART_IT_TC);

    		// Transmission fully complete, switch to receive mode
    		RS485_SetReceiveMode();
    		RS485_TxInProgress = false;

    		// Restart DMA receive
    		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
    	}
    	// DMA Completion (data transferred to UART TDR) does nothing here
    }
}
