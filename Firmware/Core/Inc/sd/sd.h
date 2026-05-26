#ifndef SD_H
#define SD_H

#include <stdbool.h>
#include "stm32g4xx_hal.h"

/* Notes:
 *
 * SD functionality from kiwih's article: https://01001000.xyz/2020-08-09-Tutorial-STM32CubeIDE-SD-card/
 *
 * IMPORTANT:
 * Need to enable LNF (Long File Name) in FatFS config:
 * 		USE_LFN: Enable with static working buffer...
 * 		MAX_LFN: 255
 *
 */

// SD Detect Card GPIO
#define SD_CARD_PORT GPIOC
#define SD_CARD_PIN GPIO_PIN_10

// TODO: ENABLE ON FINAL PCB
// Remounting when removed and reinserted at run-time will not work if set to false
#define SD_CARD_DETECT_ENABLE true


typedef struct {
	uint32_t totalMB;
	uint32_t freeMB;
	bool success;
} SD_Stats;

typedef enum {
	SD_NO_CARD,					// No card is inserted
	SD_INSERTED_MOUNT_FAILURE,	// Card was just inserted but mount failed
	SD_INSERTED_MOUNT_SUCCESS,	// Card was just inserted and mount succeeded
	SD_REMOVED,					// Card was just removed
	SD_MOUNTED					// Card is inserted and has already been mounted
} SD_Status;


SD_Status SD_Detect(void);

bool SD_IsMounted(void);

bool SD_Log(const char* message);

void SD_Unmount(void);

SD_Stats SD_GetStats(void);

#endif
