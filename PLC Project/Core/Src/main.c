/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_fatfs.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "rtc/rtc_ds3231.h"
#include "automation/automation.h"
#include "rs485/rs485.h"
#include "i2c/display.h"
#include "i2c/i2c.h"
#include "i2c/ina226.h"
#include "i2c/bmp280.h"
#include "sd/sd.h"
#include "io/io_modbus_slaves.h"
#include "io/io_emergency.h"
#include "i2c/eeprom_utils.h"
#include "splash_screens/splash_boot.h"
//#include "sensors/dht11.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

DAC_HandleTypeDef hdac1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_DAC1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

void SPI_ReInit(void) {
	MX_SPI1_Init();
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  // Add Channels
// COILS: Digital R/W: for this controller, all digital outputs
// DISCRETE INPUTS: Digital R: for this controller, all digital inputs
// HOLDING REGISTERS: Analogue R/W: for this controller, all analogue outputs
// INPUT REGISTERS: Analogue R: for this controller, all analogue inputs
  //io_coil_add_channel(GPIOC, GPIO_PIN_6);


  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_DAC1_Init();
  MX_USB_Device_Init();
  MX_SPI1_Init();
  if (MX_FATFS_Init() != APP_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN 2 */
  // SETUP ---------------------------------------------------------------------------------------//
    // Splash Screen
	display_Setup();
	//display_Boot();
	// Display after checking for factory reset

	// Config
	#define DEBOUNCE_DELAY 50 // milliseconds
	#define FACTORY_RESET_HOLD_TIME 5000 // milliseconds
	#define FACTORY_RESET_CHECK_INTERVAL 50 // milliseconds

	// Initialise I2C in case of factory reset
	I2C_Setup(&hi2c1);

	// Initialise SD if installed
	//HAL_Delay(2000);
	SD_Detect();
	SD_Log("System booting");


	// Check for Factory Reset
	GPIO_PinState factoryResetBtn = HAL_GPIO_ReadPin(GPIOB, BTN2_Pin);
	if (factoryResetBtn == GPIO_PIN_SET) {
		uint32_t heldTime = 0;

		SD_Log("Factory reset initiated by user");
		display_FactoryResetPage(0); // main

		while (1) {
			if (HAL_GPIO_ReadPin(GPIOB, BTN2_Pin) == GPIO_PIN_SET) {
				HAL_Delay(FACTORY_RESET_CHECK_INTERVAL);
				heldTime += FACTORY_RESET_CHECK_INTERVAL;

				if (heldTime >= FACTORY_RESET_HOLD_TIME) {
					// Button held for x ms, perform factory reset, and then continue as usual with boot
					if(!automation_factory_reset()) {
						// Reset Failed, display fail screen
						display_FactoryResetPage(2); // failure
						SD_Log("Factory reset failed");
						HAL_Delay(4000);

						// Continue with boot...
						break;
					} else {
						// Display success screen
						display_FactoryResetPage(1); // successful
						SD_Log("Factory reset successful");
						HAL_Delay(4000);

						// Continue with boot
						break;
					}
				}
			} else {
				display_FactoryResetPage(3); // cancelled
				SD_Log("Factory reset aborted by user");
				HAL_Delay(4000);
				// Continue with boot
				break;
			}

		}
	}

	splash_screen_boot_play(GPIOC, BTN1_Pin);

  	// Communication
    modbus_Setup(0x01); // Set modbus slave address
  	RS485_Setup(GPIOA, RS485_DIR_Pin); // changed from PA4 to PA8 to not interfere with DAC1 [RS485_Setup must be called AFTER modbus_setup]

  	// Initialise Devices
  	INA226_Init(&hi2c1);

  	// Setup Coils [HARDWARE]
  	gpio_config coil_0 = {GPIOC, DOUT1_Pin};
	gpio_config coil_1 = {GPIOB, DOUT2_Pin};
	gpio_config coil_2 = {GPIOB, DOUT3_Pin};
	gpio_config coil_3 = {GPIOB, DOUT4_Pin};
  	io_coil_add_channel(hardware_coil_write_func, &coil_0);
  	io_coil_add_channel(hardware_coil_write_func, &coil_1);
  	io_coil_add_channel(hardware_coil_write_func, &coil_2);
  	io_coil_add_channel(hardware_coil_write_func, &coil_3);

  	// Setup Discrete Inputs [HARDWARE]
  	gpio_config discrete_in_0 = {GPIOA, GPIO_PIN_2}; // PA2
  	gpio_config discrete_in_1 = {GPIOA, GPIO_PIN_3}; // PA3
  	gpio_config discrete_in_2 = {GPIOB, GPIO_PIN_13}; // PB13
  	gpio_config discrete_in_3 = {GPIOB, GPIO_PIN_14}; // PB14
  	io_discrete_in_add_channel(hardware_discrete_in_read_func, &discrete_in_0);
  	io_discrete_in_add_channel(hardware_discrete_in_read_func, &discrete_in_1);
  	io_discrete_in_add_channel(hardware_discrete_in_read_func, &discrete_in_2);
  	io_discrete_in_add_channel(hardware_discrete_in_read_func, &discrete_in_3);

  	// Setup Holding Registers [HARDWARE]
  	io_holding_reg_add_channel(dac_write_func, DAC_CHANNEL_1, IO_HOLDING_REG_VOLTAGE);
  	io_holding_reg_add_channel(dac_write_func, DAC_CHANNEL_2, IO_HOLDING_REG_VOLTAGE);


  	// Setup Input register
  	io_input_reg_add_channel(adc_read_func, ADC_CHANNEL_1, IO_INPUT_REG_VOLTAGE);
  	io_input_reg_add_channel(adc_read_func, ADC_CHANNEL_2, IO_INPUT_REG_VOLTAGE);
  	io_input_reg_add_channel(adc_read_func, ADC_CHANNEL_11, IO_INPUT_REG_VOLTAGE);
  	io_input_reg_add_channel(adc_read_func, ADC_CHANNEL_14, IO_INPUT_REG_VOLTAGE);

  	// Hardcoded Status input registers (do not remove)
  	io_input_reg_add_channel(INA226_ReadBusVoltageRaw, &hi2c1, IO_INPUT_REG_VOLTAGE);
  	io_input_reg_add_channel(INA226_ReadCurrentRaw, &hi2c1, IO_INPUT_REG_VOLTAGE);

  	// Initialise Automation
  	// This MUST be done AFTER the physical channels have been added ^^ or else the stored analogue modes (voltage/current) cannot be set
  	automation_Init();

  	// TODO: DEMO ONLY - BMP280
	//BMP280_Init();
	//io_input_reg_add_channel(BMP280_Read_Temp_Func, NULL);

  	// TODO: DEMO ONLY - MODBUS SLAVE
  	/*
  	// Setup the SHT30 as a Modbus slave
  	io_modbus_slave_add_channel(0x1E, MODBUS_REGISTER_INPUT, 0); // SHT30 slave, humidity
  	io_modbus_slave_add_channel(0x1E, MODBUS_REGISTER_INPUT, 1); // SHT30 slave, temperature
  	// Add these input register (buffers) as input registers
  	io_input_reg_add_channel(io_modbus_slave_read, 0); // modbus slave channel 0 = humidity
  	io_input_reg_add_channel(io_modbus_slave_read, 1); // modbus slave channel 1 = temperature
  	// Note, these values need to be / 10 to get actual value.
	*/

  	// Flash on-board LED
  	HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_SET);
  	HAL_Delay(60);
  	HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_RESET);
  	HAL_Delay(60);
  	HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_SET);
  	HAL_Delay(60);
	HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_RESET);

	HAL_Delay(2500);

	SD_Log("System boot complete");

	// TEMP: ->> needs to be in a timer to update every few seconds for eg TODO
	display_StatusPage();

  // ---------------------------------------------------------------------------------------------//
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	uint8_t btn1status = 0;
	uint32_t lastButtonPress = 0;

	uint32_t loopCounter = 0;
	uint32_t lastTimeTick100ms = HAL_GetTick();
	uint32_t lastTimeTick1000ms = HAL_GetTick();
	uint32_t sdSecondCount = 0;
	bool displayingSDStatus = false;

	bool boot0_pressed = false;

  while (1)
  {
	  loopCounter++;

	  /* CHECK FOR EMERGENCY STOP BEGIN*/
	  if (emergencyStop_check()) {
		  break; // Do not continue with main loop.
	  }
	  /* CHECK FOR EMERGENCY STOP END*/

	  /* RS485 Circular Frame Handling BEGIN*/
	  RS485_ProcessPendingFrames();
	  RS485_TransmitPendingFrames();
	  /* RS485 Circular Frame Handling END*/

	  /* MODBUS SLAVE POLLING (US AS MASTER) BEGIN*/
	  io_modbus_slave_poll_all();
	  modbus_master_poll_timeout();
	  /* MODBUS SLAVE POLLING (US AS MASTER) END*/

	  /* AUTOMATION BEGIN*/
	  automation_Tick();
	  /* AUTOMATION END*/


	  /* CHECK INPUTS BEGIN*/
	  // Check display button
	  GPIO_PinState btn1 = HAL_GPIO_ReadPin(GPIOC, BTN1_Pin);
	  if (btn1 == GPIO_PIN_SET) {
		  if (btn1status == 0 && (HAL_GetTick() - lastButtonPress) > DEBOUNCE_DELAY) {
			  display_BtnPress();
			  lastButtonPress = HAL_GetTick();
			  btn1status = 1;
		  }
	  } else if (btn1 == GPIO_PIN_RESET) {
		  btn1status = 0;
	  }
	  /* CHECK INPUTS END*/


	  /* SCHEDULE BEGIN*/
	  // Every 100ms
	  if ((HAL_GetTick() - lastTimeTick100ms) >= 100 || (HAL_GetTick() < lastTimeTick100ms)) { // wraparound-safe comparison
		  lastTimeTick100ms = HAL_GetTick();

		  // Check if BOOT0 button pressed
		  GPIO_PinState boot0 = HAL_GPIO_ReadPin(GPIOB, BOOT0_Pin);
		  if (boot0 == GPIO_PIN_SET) {
			  display_dfu();
			  boot0_pressed = true;
		  } else if (boot0_pressed) {
			  display_StatusPage();
			  boot0_pressed = false;
		  }
	  }


	  // Every second
	  if ((HAL_GetTick() - lastTimeTick1000ms) >= 1000 || (HAL_GetTick() < lastTimeTick1000ms)) { // wraparound-safe comparison
		  lastTimeTick1000ms = HAL_GetTick();

		  loopCounter = 0;

		  if (displayingSDStatus) {
			  usb_serial_println("displayingSDstatus");
			  if (sdSecondCount >= 2) { // show for 2s
				  displayingSDStatus = false;
				  sdSecondCount = 0;

				  // Update display if boot0 is not pressed
				  if (boot0_pressed == false) {
					  display_StatusPage();
				  }
			  } else {
				  sdSecondCount++;
			  }
		  } else {
			  usb_serial_println("sd status triggered 1");
			  // Check for SD card insertion/removal
			  SD_Status sd_status = SD_Detect(); // Will automatically mount/unmount
			  if (sd_status == SD_INSERTED_MOUNT_FAILURE || sd_status == SD_INSERTED_MOUNT_SUCCESS || sd_status == SD_REMOVED) {
				  usb_serial_println("sd status triggered 2");
				  display_sd_status(sd_status);
				  displayingSDStatus = true;
			  } else {
				  // Update display if boot0 is not pressed
				  if (boot0_pressed == false) {
					  display_StatusPage();
				  }
			  }
		  }
	  }

	  // Every 10 seconds since pressing display button, go to main page
	  if ((HAL_GetTick() - lastButtonPress) >= 10000|| (HAL_GetTick() < lastButtonPress)) { // wraparound-safe comparison
		  if (!displayingSDStatus) {
			  display_setPage(0);
		  }
	  }

	  /* SCHEDULE END*/

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  /*HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_SET);
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_RESET);
	  HAL_Delay(500);
		*/

	  // Get current time
	  /*RTC_Time currentTime;
	  char timeStr[16];
	  if (DS3231_ReadTime(&currentTime) == HAL_OK) {
		  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", currentTime.hours, currentTime.minutes, currentTime.seconds);
	  } else {
		  snprintf(timeStr, sizeof(timeStr), "Error!");
	  }

	  // Clear display
	  ssd1306_Fill(Black);

	  // Set cursor position
	  ssd1306_SetCursor(10, 25);  // adjust x, y as needed

	  // Write string to buffer
	  ssd1306_WriteString(timeStr, Font_11x18, White);

	  // Update display with buffer content
	  ssd1306_UpdateScreen();

	  HAL_Delay(200);*/

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
  sConfig.DAC_DMADoubleDataMode = DISABLE;
  sConfig.DAC_SignedFormat = DISABLE;
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_EXTERNAL;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT2 config
  */
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00300617;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_2;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, DOUT1_Pin|LED_Pin|SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DOUT2_Pin|DOUT3_Pin|DOUT4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BTN1_Pin */
  GPIO_InitStruct.Pin = BTN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(BTN1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DIN1_Pin DIN2_Pin */
  GPIO_InitStruct.Pin = DIN1_Pin|DIN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DOUT1_Pin LED_Pin SD_CS_Pin */
  GPIO_InitStruct.Pin = DOUT1_Pin|LED_Pin|SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : DOUT2_Pin DOUT3_Pin DOUT4_Pin */
  GPIO_InitStruct.Pin = DOUT2_Pin|DOUT3_Pin|DOUT4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : DIN3_Pin DIN4_Pin BOOT0_Pin */
  GPIO_InitStruct.Pin = DIN3_Pin|DIN4_Pin|BOOT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RS485_DIR_Pin */
  GPIO_InitStruct.Pin = RS485_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RS485_DIR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_CARD_Pin */
  GPIO_InitStruct.Pin = SD_CARD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SD_CARD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN2_Pin */
  GPIO_InitStruct.Pin = BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(BTN2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
