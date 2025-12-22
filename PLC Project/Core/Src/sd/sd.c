#include "sd/sd.h"
#include "rtc/rtc_ds3231.h"
#include <stdio.h>
#include <string.h>
#include "app_fatfs.h"
#include "main.h" // for SPI_ReInit()
#include "usb_serial.h"
#include "ff_gen_drv.h"

// Defined in app_fatfs.c by CubeMX
extern char USERPath[4];
extern Diskio_drvTypeDef USER_Driver;

static FATFS fatFs;
static FIL logFile;
static bool isMounted = false;
static bool lastKnownInsertedStatus = false; // false = not inserted, true = inserted

static char logFilename[32];


static bool SD_Mount(void) {
	// Clear any previous driver link
	SD_Unmount();

	// Relink the driver
	if (FATFS_LinkDriver(&USER_Driver, USERPath) != 0) {
		return false;
	}

	// Reset SPI Hardware
	SPI_ReInit();

	// Force mount
	FRESULT res = f_mount(&fatFs, (TCHAR const*)USERPath, 1);
	if (res != FR_OK) {
		isMounted = false;
		return false;
	}

	lastKnownInsertedStatus = true;

	isMounted = true;
	return true;
}

void SD_Unmount(void) {
	// Unmount
	f_mount(NULL, (TCHAR const*)USERPath, 0);

	// Unlink the driver
	FATFS_UnLinkDriver(USERPath);

	isMounted = false;
}

// Detects if card is installed and tries to mount automatically
SD_Status SD_Detect(void) {
    if (SD_CARD_DETECT_ENABLE == true) {
        if (HAL_GPIO_ReadPin(SD_CARD_PORT, SD_CARD_PIN) == GPIO_PIN_RESET) {
        	// Card is detected

        	lastKnownInsertedStatus = true;

            if (!isMounted) {
                return SD_Mount() ? SD_INSERTED_MOUNT_SUCCESS : SD_INSERTED_MOUNT_FAILURE;
            }

            return SD_MOUNTED;
        } else {
            // Card is not detected
            isMounted = false;

            if (lastKnownInsertedStatus) {
            	SD_Unmount();
            	lastKnownInsertedStatus = false;
            	return SD_REMOVED;
            }

            lastKnownInsertedStatus = false;
            return SD_NO_CARD;
        }
    } else {
        // No card detect pin, just try mounting every time.
    	// Remounting will not work. This is for test only.
        return SD_Mount() ? SD_MOUNTED : SD_NO_CARD;
    }
}

bool SD_IsMounted(void) {
	return isMounted;
}

bool SD_Log(const char* message) {
	if (!isMounted) return false;

	// Get today's file name, timestamp
	RTC_Time time;
	char timestamp[16];

	if (DS3231_ReadTime(&time) != HAL_OK) {
		snprintf(logFilename, sizeof(logFilename), "unknown_date.log");
		snprintf(timestamp, sizeof(timestamp), "[--:--:--] ");
	} else {
		snprintf(logFilename, sizeof(logFilename), "%04d-%02d-%02d.log", 2000 + time.year, time.month, time.day);
		snprintf(timestamp, sizeof(timestamp), "[%02d:%02d:%02d] ", time.hours, time.minutes, time.seconds);
	}

	// Open the logfile in write mode, and create if it doesn't exist
	FRESULT res = f_open(&logFile, logFilename, FA_WRITE | FA_OPEN_ALWAYS);

	/* If having issues can use this to get error code
	 * if (res != FR_OK) {
		char msg[128];
	    sprintf(msg, "f_open failed: %d\n", res);
	    usb_serial_println(msg);
	    return false;
	}*/

	if (res != FR_OK) return false;

	// Move the file pointer to the end of the file to append, rather than overwrite
	res = f_lseek(&logFile, f_size(&logFile));
	if (res != FR_OK) {
		f_close(&logFile);
		return false;
	}

	// Generate log line with timestamp, message, line breaks
	char line[256];
	snprintf(line, sizeof(line), "%s%s\r\n", timestamp, message);

	// Write the line, and close the file after.
	UINT bytesWritten;
	res = f_write(&logFile, line, strlen(line), &bytesWritten);
	f_close(&logFile);

	return (res == FR_OK && bytesWritten == strlen(line));
}

SD_Stats SD_GetStats(void) {
	SD_Stats stats = {0};

	if (!isMounted) {
		stats.success = false;
		return stats;
	}

	DWORD free_clusters, total_sectors, free_sectors;
	FATFS *fs_ptr = &fatFs;

	FRESULT res = f_getfree("", &free_clusters, &fs_ptr);
	if (res != FR_OK) {
		stats.success = false;
		return stats;
	}

	// From Chan's FatFs documentation:
	// total_sectors = (fs->n_fatent - 2) * fs->csize;
	// free_sectors  = free_clusters * fs->csize;
	total_sectors = (fs_ptr->n_fatent - 2) * fs_ptr->csize;
	free_sectors  = free_clusters * fs_ptr->csize;

	// Sector size is 512 bytes -> 1 sector = 0.5 KiB
	// Convert to MB: (sectors / 2) / 1024 = MB
	stats.totalMB = total_sectors / 2048;
	stats.freeMB  = free_sectors / 2048;
	stats.success = true;

	return stats;
}
