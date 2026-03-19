#ifndef CONFIG_BOARD_7915B6A_CFG_H
#define CONFIG_BOARD_7915B6A_CFG_H

#ifdef CONFIG_BOARD_7915B6A

#define __FLASH_SIZE__    (2 * 1024 * 1024)
#define __SDRAM_SIZE__    (2 * 1024 * 1024)

#define CONFIG_ALL_USB_ENABLE
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
