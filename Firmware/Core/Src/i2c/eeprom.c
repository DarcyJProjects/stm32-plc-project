#include "i2c/eeprom.h"
#include "usb_serial.h"


bool EEPROM_Write(uint16_t memAddr, uint8_t* data, uint16_t len) {
	while (len > 0) {
		uint8_t pageOffset = memAddr % EEPROM_PAGE_SIZE;
		uint8_t spaceInPage = EEPROM_PAGE_SIZE - pageOffset;
		uint8_t writeLen = (len < spaceInPage) ? len : spaceInPage;

		uint8_t buffer[2 + EEPROM_PAGE_SIZE];
		buffer[0] = memAddr >> 8; // high byte
		buffer[1] = memAddr & 0xFF; // low byte
		memcpy(&buffer[2], data, writeLen);

		if (I2C_Transmit(EEPROM_I2C_ADDR << 1, buffer, writeLen + 2) != HAL_OK) {
			return false;
		}

		HAL_Delay(5); // wait for EEPROM write cycle to complete

		memAddr += writeLen;
		data += writeLen;
		len -= writeLen;
	}

	return true;
}


bool EEPROM_Read(uint16_t memAddr, uint8_t* data, uint16_t len) {
	uint8_t addrBytes[2] = { memAddr >> 8, memAddr & 0xFF };

	if (I2C_Transmit(EEPROM_I2C_ADDR << 1, addrBytes, 2) != HAL_OK) {
		return false;
	}

	if (I2C_Receive((EEPROM_I2C_ADDR << 1) | 1, data, len) != HAL_OK) {
		return false;
	}

	return true;
}


bool EEPROM_WriteBlock(uint16_t memAddr, const void* data, uint16_t len) {
	if (len == 0) return true;
	return EEPROM_Write(memAddr, (uint8_t*)data, len);
}


bool EEPROM_LoadBlock(uint16_t memAddr, void* data, uint16_t len) {
	return EEPROM_Read(memAddr, (uint8_t*)data, len);
}
