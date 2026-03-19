#ifndef CONFIG_BOARD_7911B8_CFG_H
#define CONFIG_BOARD_7911B8_CFG_H

#ifdef CONFIG_BOARD_7911B8

#define __FLASH_SIZE__    (1 * 1024 * 1024)
#define __SDRAM_SIZE__    (0 * 1024 * 1024)

#define CONFIG_FLASH_FOUR_WIDTH_ENABLE                 //FLASH四线模式
#undef CONFIG_VIDEO_ENABLE
#undef CONFIG_AUDIO_ENABLE

//7911B
#include "board_7911B_develop_cfg.h"
#include "board_7911B_dui_cfg.h"

#ifdef CONFIG_BOARD_MUTEX
#error "board config can not enable at the same time, just select one!!!"
#else
#define CONFIG_BOARD_MUTEX
#endif

#endif

#endif
