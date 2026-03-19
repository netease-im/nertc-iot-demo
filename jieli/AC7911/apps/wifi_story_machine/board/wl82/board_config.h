#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/*
 *  板级配置选择，需要删去app_config.h中前面跟此头文件重复的宏定义，不然此头文件宏定义无效,只能打开一个, 不然会报错
 */

//7911B
#define CONFIG_BOARD_7911BA
// #define CONFIG_BOARD_7911BB
// #define CONFIG_BOARD_7911B0
// #define CONFIG_BOARD_7911B8
// #define CONFIG_BOARD_7915AA
// #define CONFIG_BOARD_7915B6A
// #define CONFIG_BOARD_7912AB

//7911D
// #define CONFIG_BOARD_7911D

//7912D
// #define CONFIG_BOARD_7912D

//7913A
// #define CONFIG_BOARD_7913A
// #define CONFIG_BOARD_7913A6

//7916A
// #define CONFIG_BOARD_7916AA
//#define CONFIG_BOARD_7916AB



/*
 *  工程配置选择,只能打开一个, 不然会报错
 */
// #define CONFIG_BOARD_DUI
// #define CONFIG_BOARD_DEMO
//#define CONFIG_BOARD_DEV_KIT
#define CONFIG_BOARD_DEVELOP



//7911B
#if defined CONFIG_BOARD_7911BA || defined CONFIG_BOARD_7911BB || defined CONFIG_BOARD_7911B0 || defined CONFIG_BOARD_7911B8 || defined CONFIG_BOARD_7912AB || defined CONFIG_BOARD_7915AA || defined CONFIG_BOARD_7915B6A
#define CONFIG_BOARD_7911B
#endif

//7916A
#if defined CONFIG_BOARD_7916AA || defined CONFIG_BOARD_7916AB
#define CONFIG_BOARD_7916A
#endif

//7913A
#ifdef CONFIG_BOARD_7913A6
#define CONFIG_BOARD_7913A
#endif



//7911B
#include "board_7911BA_cfg.h"
// #include "board_7911BB_cfg.h"
// #include "board_7911B0_cfg.h"
// #include "board_7911B8_cfg.h"

// #include "board_7915AA_cfg.h"
// #include "board_7915B6A_cfg.h"

// #include "board_7912AB_cfg.h"

// //7911D
// #include "board_7911D_cfg.h"

// //7912D
// #include "board_7912D_cfg.h"

// //7913A
// #include "board_7913A_cfg.h"
// #include "board_7913A6_cfg.h"

// //7916A
// #include "board_7916AA_cfg.h"
// #include "board_7916AB_cfg.h"


#endif
