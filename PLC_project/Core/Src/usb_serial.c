//	First source file
//		Defines how the functions work (i.e., the actual implementation).

#include "usb_serial.h" // Includes the header file so this source file knows about the function declared

void usb_serial_print(const char *msg) { // Actual implementation of the method thats in the header file
    uint16_t len = strlen(msg);	// Calculates the length of the string so we know how many bytes to send
    CDC_Transmit_FS((uint8_t*)msg, len); // Sends the message over USB as serial data. (uint8_t*) casts the string to a byte array as required by the method. len is just how many bytes to actually send
}

void usb_serial_println(const char *msg) {
	char buffer[128];
	snprintf(buffer, sizeof(buffer), "%s\r\n", msg);
	usb_serial_print(buffer);
}
