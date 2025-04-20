#include "sensors/dht11.h"
// Sources:
// https://www.engineersgarage.com/articles-arduino-dht11-humidity-temperature-sensor-interfacing/
// https://kbiva.wordpress.com/2013/03/25/microsecond-delay-function-for-stm32/

// Variables
static GPIO_TypeDef* DHT11_PORT;
static uint16_t DHT11_PIN;

// --- Initialise the DWT ---
// Initates the DWT (Data Watchpoint and Trace) unit (if not already initiated)
// Used for microsecond timing
static void DWT_Init(void) {
    if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

// --- Delay for microseconds ---
static void delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT; // get the current cycle count
    uint32_t ticks = us * (SystemCoreClock / 1000000); // calculate how many CPU cycles represent the required us delay
    while ((DWT->CYCCNT - start) < ticks); // do nothing (wait) until the required number of ticks for the us delay have been achieved
}

// --- Pin Mode Switching ---
static void set_input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Works for me without internal or external pull up. External may be required for some models?
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

static void set_output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

// --- Pulse Duration Measurement ---
static uint32_t wait_for_input_state(uint8_t state, uint32_t timeout_us) {
    uint32_t start = DWT->CYCCNT; // get current cycle count
    uint32_t ticks = timeout_us * (SystemCoreClock / 1000000); // get the number of ticks for the timeout

    while ((DWT->CYCCNT - start) < ticks) {
        if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == state) {
            return (DWT->CYCCNT - start) / (SystemCoreClock / 1000000);
        }
    }
    return 0xFFFFFFFF; // timeout
}

// --- Initiate the DHT11 ---
static void DHT11_Init(void) {
    DWT_Init();
    set_output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET); // set pin to HIGH
}

// --- SETUP DHT11 ---
/**
 * @brief Sets up the GPIO configuration for the DHT11 sensor.
 *
 * This function configures the specified GPIO port and pin for the DHT11 sensor,
 * enabling the clock for the GPIO port if needed. After setup, it initialises the
 * DHT11 sensor.
 *
 * @param port: Pointer to the port where the DHT11 data pin is connected (e.g., GPIOA, GPIOB, GPIOC).
 * @param pin: Pin number of the GPIO port where the DHT11 data pin is connected.
 */
void DHT11_Setup(GPIO_TypeDef* port, uint16_t pin) {
	DHT11_PORT = port;
	DHT11_PIN = pin;

	// Enable the ports peripheral clock
	if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
	else if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
	else if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();

	DHT11_Init();
}

// --- MAIN READ FUNCTION ---
/**
 * @brief Reads temperature and humidity (int and dec) from the DHT11 sensor.
 *
 * This function initiates communication with the DHT11 sensor and reads the temperature and humidity values.
 * It requires a successful setup of the DHT11 sensor through DHT11_Setup() before calling this function.
 *
 * @pre DHT11_Setup(port, pin) must be called before using this function.
 *
 * @param temperature_int: Pointer to store the integer part of the temperature (°C).
 * @param temperature_dec: Pointer to store the decimal part of the temperature (some DHT11 models will just be 0).
 * @param humidity_int: Pointer to store the integer part of the humidity (%RH).
 * @param humidity_dec: Pointer to store the decimal part of the humidity (some DHT11 models will just be 0).
 *
 * @retval DHT11_OK: If the data was read successfully.
 * @retval DHT11_ERROR_NO_RESPONSE: If the sensor did not respond.
 * @retval DHT11_ERROR_TIMEOUT: If a timeout occurred while reading data.
 * @retval DHT11_ERROR_CHECKSUM: If the checksum does not match.
 *
 */
DHT11_Response DHT11_Read(uint8_t* temperature_int, uint8_t* temperature_dec, uint8_t* humidity_int, uint8_t* humidity_dec) {
    // Create the data array to store the bytes
    uint8_t data[5] = {0}; // 5 bytes = 40 bits

    // 1. Send the start signal
    set_output();

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET); // pull low
    delay_us(18000); // for 18 ms
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET); // pull high
    delay_us(20); // short delay

    // 2. Read response
    set_input();

    // Require LOW (54us) then HIGH (80us) (but exact timing doesn't matter, checksum will be there to see if it was successful)
    if (wait_for_input_state(GPIO_PIN_RESET, 100) == 0xFFFFFFFF) return DHT11_ERROR_NO_RESPONSE; // require LOW
    if (wait_for_input_state(GPIO_PIN_SET, 100) == 0xFFFFFFFF) return DHT11_ERROR_NO_RESPONSE; // require HIGH

    // Require LOW (the start of data)
    if (wait_for_input_state(GPIO_PIN_RESET, 100) == 0xFFFFFFFF) return DHT11_ERROR_TIMEOUT; // require LOW

    // 3. Read 5 bytes = 40 bits
    // Each bit starts with LOW for 54us, then HIGH: 24us is a bit 0, 70us is a bit 1
    for (int i = 0; i < 40; i++) {
        if (wait_for_input_state(GPIO_PIN_SET, 70) == 0xFFFFFFFF) return DHT11_ERROR_TIMEOUT; // Start of bit is LOW, so wait for HIGH (which actually tells us what the bit is)
        uint32_t held = wait_for_input_state(GPIO_PIN_RESET, 100); // Now that the pin is HIGH, wait until it goes LOW and count how long it takes for it to do so

        if (held == 0xFFFFFFFF) return DHT11_ERROR_TIMEOUT;
        uint8_t bit = (held > 40) ? 1 : 0; // if was held for >40us, then its a 1, less than 40us, its a 0

        data[i / 8] <<= 1; // i is current bit index, i / 8 is current byte index (8 bits in 1 byte), <<= 1 shifts the current byte left by 1 to make room for the new byte at LSB
        data[i / 8] |= bit; // bit is either 0 or 1, |= is bitwise OR assignment operator, inserts new bit into the LSB of the byte

        // Example for the above bit writing (writing bits 1, 0, and then 1):
        // data[0] <<= 1;       --> data[0] = 00000000 << 1 = 00000000
        // data[0] |= 1;        --> data[0] = 00000000 | 00000001 = 00000001
        // data[0] <<= 1;       --> data[0] = 00000001 << 1 = 00000010
        // data[0] |= 0;        --> data[0] = 00000010 | 00000000 = 00000010
        // data[0] <<= 1;       --> data[0] = 00000010 << 1 = 00000100
        // data[0] |= 1;        --> data[0] = 00000100 | 00000001 = 00000101
    }

    // 4. Check the checksum to ensure a valid reading
    // Checksum byte should be the sum of all data bytes
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) return DHT11_ERROR_CHECKSUM;

    *humidity_int = data[0];
    *humidity_dec = data[1];
    *temperature_int = data[2];
    *temperature_dec = data[3];

    return DHT11_OK; // Successfully read the DHT11
}
