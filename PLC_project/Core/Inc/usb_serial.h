//	First header file
//		Declares what the functions are so other files (like main.c) can know about them and call them

#ifndef USB_SERIAL_H // If not defined, it will continue to define USB_SERIAL_H. If it comes across it again, it skips this file as its already defined. (stops redefinition errors)
#define USB_SERIAL_H // USB_SERIAL_H is just a unique identifier for this header file

#include "usbd_cdc_if.h"  // Required for CDC_Transmit_FS() which sends data over USB CDC

void usb_serial_print(const char *msg);

#endif // Closes the include guard that started with #ifndef
