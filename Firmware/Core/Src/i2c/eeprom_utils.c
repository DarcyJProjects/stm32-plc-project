#include "i2c/eeprom_utils.h"
#include "i2c/eeprom.h"
#include "string.h"
#include "usb_serial.h"

bool EEPROM_Dump(void) {
	uint8_t buffer[EEPROM_PAGE_SIZE];
	char msg[128];

	usb_serial_println("--- EEPROM DUMP BEGIN ---");

	for (uint16_t addr = 0; addr < EEPROM_SIZE_BYTES; addr += EEPROM_PAGE_SIZE) {
		if (!EEPROM_Read(addr, buffer, EEPROM_PAGE_SIZE)) {
			usb_serial_println("EEPROM Read Error, aborting.");
			return false;
		}

		int n = snprintf(msg, sizeof(msg), "%04X: ", addr);
		usb_serial_print(msg);

		for (int i = 0; i < EEPROM_PAGE_SIZE; i++) {
			n = snprintf(msg, sizeof(msg), "%02X ", buffer[i]);
			usb_serial_print(msg);
		}

		usb_serial_println("");
	}
	usb_serial_print("--- EEPROM DUMP END ---");
	return true;
}

bool EEPROM_Wipe(uint16_t upToAddr) {
	if (upToAddr > EEPROM_SIZE_BYTES) upToAddr = EEPROM_SIZE_BYTES;

	uint8_t blank[EEPROM_PAGE_SIZE];
	memset(blank, 0xFF, EEPROM_PAGE_SIZE);

	uint16_t addr = 0;
	while (addr < upToAddr) {
		uint16_t writeLen = (upToAddr - addr > EEPROM_PAGE_SIZE) ? EEPROM_PAGE_SIZE : (upToAddr - addr);
		if (!EEPROM_Write(addr, blank, writeLen)) {
			usb_serial_println("EEPROM Wipe Error");
			return false;
		}
		addr += writeLen;
	}

	usb_serial_println("EEPROM Wipe Complete");
	return true;
}
