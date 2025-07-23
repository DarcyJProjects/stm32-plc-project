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
#define SD_CARD_DETECT_ENABLE false


typedef struct {
	uint32_t totalMB;
	uint32_t freeMB;
	bool success;
} SD_Stats;


bool SD_Detect(void);

bool SD_IsMounted(void);

bool SD_Log(const char* message);

void SD_Unmount(void);

SD_Stats SD_GetStats(void);

#endif
