// Sources
// https://deepbluembedded.com/how-to-receive-uart-serial-data-with-stm32-dma-interrupt-polling/
// https://deepbluembedded.com/stm32-uart-receive-unknown-length-idle-line-detection-examples/#stm32-uart-idle-line-detection-dma-example

#include "rs485/rs485.h"
#include "usb_serial.h" // debug only

#define LED_STATUS_ENABLE

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;


// Queue
#define RS485_FRAME_QUEUE_LEN 8
#define RS485_FRAME_MAX_SIZE 256

typedef struct {
	uint8_t data[RS485_FRAME_MAX_SIZE];
	uint16_t len;
} RS485_Frame;

// Variables
static GPIO_TypeDef* RS485_DIR_PORT;
static uint16_t RS485_DIR_PIN;

#define RS485_DMA_BUFFER_SIZE 256 // MODBUS max packet size
static uint8_t RS485_DMA_BUFFER[RS485_DMA_BUFFER_SIZE] = {0};
static volatile uint16_t RS485_ReceivedLength = 0;

static volatile RS485_Frame rs485_rx_frame_queue[RS485_FRAME_QUEUE_LEN]; // circular buffer storing full Modbus frames received
static volatile RS485_Frame rs485_tx_frame_queue[RS485_FRAME_QUEUE_LEN]; // same but for frames to be transmitted
static volatile uint8_t rs485_rx_frame_head = 0; // points to where the next incoming frame will go
static volatile uint8_t rs485_rx_frame_tail = 0; // points to where the oldest unprocessed frame is
static volatile uint8_t rs485_tx_frame_head = 0;
static volatile uint8_t rs485_tx_frame_tail = 0;
int txBusy = 0;

void RS485_SetTransmitMode(void) {
	// Set direction pin to HIGH (transmit)
	HAL_GPIO_WritePin(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_PIN_SET);
#ifdef LED_STATUS_ENABLE
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
#endif
}

void RS485_SetReceiveMode(void) {
	// Set direction pin to LOW (receive)
	HAL_GPIO_WritePin(RS485_DIR_PORT, RS485_DIR_PIN, GPIO_PIN_RESET);
#ifdef LED_STATUS_ENABLE
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
#endif
}

void RS485_Setup(GPIO_TypeDef* dir_port, uint16_t dir_pin) {
	memset((void*)rs485_rx_frame_queue, 0, sizeof(rs485_rx_frame_queue));
	memset((void*)rs485_tx_frame_queue, 0, sizeof(rs485_tx_frame_queue));
	rs485_tx_frame_head = 0;
	rs485_tx_frame_tail = 0;

	RS485_DIR_PORT = dir_port;
	RS485_DIR_PIN = dir_pin;

	RS485_SetReceiveMode();

	// Start DMA
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
}

void RS485_Transmit(uint8_t *data, uint16_t len) {
	// Adds frames to be transmitted to a circular queue.
	// Call RS485_TransmitPendingFrames() from main loop.

	if (len == 0 || len > RS485_FRAME_MAX_SIZE) return;

    uint8_t next = (rs485_tx_frame_head + 1) % RS485_FRAME_QUEUE_LEN;

    if (next != rs485_tx_frame_tail) { // if not full
    	// Copy the data to transmit into the queue
		memcpy((void*)rs485_tx_frame_queue[rs485_tx_frame_head].data, data, len);
		rs485_tx_frame_queue[rs485_tx_frame_head].len = len;
		rs485_tx_frame_head = next;
    } else {
    	usb_serial_println("TX queue overflow!");
    }
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
	// Adds received frame from DMA buffer to a circular queue to be processed.
	// Call RS485_ProcessPendingFrames() from main loop.

	if (huart->Instance == USART1) {
		uint8_t next = (rs485_rx_frame_head + 1) % RS485_FRAME_QUEUE_LEN;
		if (next != rs485_rx_frame_tail) { // if not full
			// Copy the received data into the queue
			if (size <= RS485_FRAME_MAX_SIZE) {
				memcpy((void*)rs485_rx_frame_queue[rs485_rx_frame_head].data, RS485_DMA_BUFFER, size);
				rs485_rx_frame_queue[rs485_rx_frame_head].len = size;
				rs485_rx_frame_head = next;
			}
		} else {
			usb_serial_println("RX queue overflow!");
		}

		// Ready for next reception
		if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE) != HAL_OK) {
		    usb_serial_println("Failed to re-arm UART RX DMA!");
		}
	}
}

void RS485_TCCallback(void)
{
	// Finished sending this frame
	txBusy = 0;

	// Transmission fully complete, switch to receive mode
	RS485_SetReceiveMode();
	__HAL_UART_DISABLE_IT(&huart1, UART_IT_TC);

	// Restart DMA receive
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);

	// !! See bottom of this file for required extra code !!
}

void RS485_ProcessPendingFrames(void) {
	while (rs485_rx_frame_tail != rs485_rx_frame_head) {
		uint8_t* frame_data = (uint8_t*)rs485_rx_frame_queue[rs485_rx_frame_tail].data;
		uint16_t frame_len = rs485_rx_frame_queue[rs485_rx_frame_tail].len;

		modbus_handle_frame(frame_data, frame_len); // Pass the frames over to modbus to handle

		rs485_rx_frame_tail = (rs485_rx_frame_tail + 1) % RS485_FRAME_QUEUE_LEN;
	}
}

void RS485_TransmitPendingFrames(void) {
	// Only transmit if not already sending
	if (huart1.gState == HAL_UART_STATE_READY && txBusy != 1) {
		if (rs485_tx_frame_tail != rs485_tx_frame_head) {
			txBusy = 1;
			uint8_t* frame_data = (uint8_t*)rs485_tx_frame_queue[rs485_tx_frame_tail].data;
			uint16_t frame_len = rs485_tx_frame_queue[rs485_tx_frame_tail].len;

			/*
			// TODO: REMOVE DEBUG ONLY
			char hex_string[2 * frame_len + 1];

			for (size_t i = 0; i < frame_len; i++) {
				snprintf(&hex_string[2 * i], 3, "%02X", frame_data[i]);
			}

			usb_serial_println(hex_string);
*/

			RS485_SetTransmitMode();

			// Disable TC interrupt
			__HAL_UART_DISABLE_IT(&huart1, UART_IT_TC);

			// Transmit and store the status of it
			HAL_StatusTypeDef transmitStatus = HAL_UART_Transmit_DMA(&huart1, frame_data, frame_len);

			// Enable TC interrupt
			__HAL_UART_ENABLE_IT(&huart1, UART_IT_TC);

			if (transmitStatus != HAL_OK) {
				// UART TX DMA Error - switch back to receiving
				RS485_SetReceiveMode();
				__HAL_UART_DISABLE_IT(&huart1, UART_IT_TC);
				HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
			} else {
				rs485_tx_frame_tail = (rs485_tx_frame_tail + 1) % RS485_FRAME_QUEUE_LEN;
			}
		}
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        usb_serial_println("UART Error! Reinitializing RX...");
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RS485_DMA_BUFFER, RS485_DMA_BUFFER_SIZE);
    }
}








/* NEED TO ADD THE FOLLOWING CODE IN stm32g4xx_it.c IN USART1_IRQHandler TO CALL THIS FUNCTION:
	 *
	 * !!! This code needs to be put AFTER HAL_UART_IQRHandler(&huart1); !!!
	 *
    // Check if TC interrupt is triggered
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC)) {
        // Clear the TC interrupt flag
        __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);

        // Disable TC interrupt (optional, if no longer needed)
        __HAL_UART_DISABLE_IT(&huart1, UART_IT_TC);

        // Call the post-transmission function from RS485.c
        RS485_TCCallback();
    }

     * // REMEMBER TO INCLUDE RS485.h: #include "rs485/rs485.h"
     */

