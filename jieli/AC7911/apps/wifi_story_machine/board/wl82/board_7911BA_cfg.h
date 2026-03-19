#ifndef CONFIG_BOARD_7911BA_CFG_H
#define CONFIG_BOARD_7911BA_CFG_H

#ifdef CONFIG_BOARD_7911BA

#define __FLASH_SIZE__    (8 * 1024 * 1024)
#define __SDRAM_SIZE__    (2 * 1024 * 1024)

//7911B
#include "board_7911B_develop_cfg.h"
// #include "board_7911B_gizwits_cfg.h"

#ifdef CONFIG_BOARD_MUTEX
#error "board config can not enable at the same time, just select one!!!"
#else
#define CONFIG_BOARD_MUTEX
#endif

#endif

#endif
