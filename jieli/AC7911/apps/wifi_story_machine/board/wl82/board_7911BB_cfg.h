#ifndef CONFIG_BOARD_7911BB_CFG_H
#define CONFIG_BOARD_7911BB_CFG_H

#ifdef CONFIG_BOARD_7911BB

#define __FLASH_SIZE__    (8 * 1024 * 1024)
#define __SDRAM_SIZE__    (8 * 1024 * 1024)

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
