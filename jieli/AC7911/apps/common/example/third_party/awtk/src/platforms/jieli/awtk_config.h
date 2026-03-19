
/**
 * File:   awtk_config.h
 * Author: AWTK Develop Team
 * Brief:  config
 *
 * Copyright (c) 2018 - 2023  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-09-12 Li XianJing <xianjimli@hotmail.com> created
 */

#ifndef AWTK_CONFIG_H
#define AWTK_CONFIG_H

#include "lcd_config.h"

#ifdef __cplusplus
extern "C" { /* Make sure we have C-declarations in C++ programs */
#endif

extern int printf(const char *fmt, ...);
extern int snprintf(char *buf, unsigned long size, const char *fmt, ...);

#ifdef __cplusplus
}
#endif


#define log_impl(format, args...) printf(format, ##args)

/**
 * 嵌入式系统有自己的main函数时，请定义本宏。
 *
 */
#define USE_GUI_MAIN 1

/**
 * 如果支持png/jpeg图片，请定义本宏
 *
 */
#define WITH_STB_IMAGE 1

/**
 * 如果用 stb 支持 Truetype 字体，请定义本宏
 *
 */
// #define WITH_STB_FONT 1

/**
 * 如果用 freetype 支持 Truetype 字体，请定义本宏。
 *
 */
#define WITH_FT_FONT 1
#define USE_SYSTEM_FREETYPE 1


/**
 * 如果定义本宏，使用标准的UNICODE换行算法，除非资源极为有限，请定义本宏。
 *
 */
#define WITH_UNICODE_BREAK 1


/**
 * 如果定义本宏，将图片解码成BGRA8888格式，否则解码成RGBA8888的格式。
 *
 */
#define WITH_BITMAP_BGRA 1

/**
 * 如果定义本宏，将不透明的PNG图片解码成BGR565/RGB565/BGR888/RGB888格式
 * 和LCD的格式保存一致，可以大幅度提高性能。
 * 如果没有定义 WITH_BITMAP_BGR565 和 WITH_BITMAP_RGB565 和 WITH_BITMAP_BGR888 和 WITH_BITMAP_RGB888宏，默认解析为32位色
 */
#define WITH_BITMAP_BGR565 1
/*#define WITH_BITMAP_RGB565 1*/
/*#define WITH_BITMAP_BGR888 1*/
/*#define WITH_BITMAP_RGB888 1*/


/**
 * 如果FLASH空间较小，不足以放大字体文件时，请定义本宏
 *
 */
//#define WITH_MINI_FONT 1

/**
 * 如果启用STM32 G2D硬件加速，请定义本宏
 *
 */
//#define WITH_STM32_G2D 1

/**
 * 如果启用VGCANVAS，请定义本宏
 *
 */
#define WITH_VGCANVAS 1


/**
* 在没有GPU时，如果启用agge作为nanovg的后端(较agg作为后端：小，快，图形质量稍差)，请定义本宏。
*
*/
#define WITH_NANOVG_AGGE 1

/**
 * 在没有GPU时，如果启用agg作为nanovg的后端(较agge作为后端：大，慢，图形质量好)，请定义本宏。
 * 注意：agg是以GPL协议开源。
 * 注意：目前不支持565格式的图片，请勿定义WITH_BITMAP_BGR565或WITH_BITMAP_RGB565
 *
 */
// #define WITH_NANOVG_AGG 1

/**
 * 在没有GPU时，如果启用pixman作为cairo的后端(大，慢，图形质量好)，请定义本宏。
 */
//#define WITH_VGCANVAS_CAIRO 1

/**
 * 如果启用竖屏，请定义本宏
 *
 */
// #define WITH_LCD_PORTRAIT 1
// #define WITH_FAST_LCD_PORTRAIT 1

/**
 * 如果支持从文件系统加载资源，请定义本宏。
 *
 */
#define WITH_FS_RES 1

/**
 * 如果代码在flash中，而资源在文件系统，请定义本宏指明资源所在的路径。
 *
 */
#define APP_RES_ROOT CONFIG_ROOT_PATH

/**
 * 如果需要配置文件或者使用data_reader/data_writer，请定义本宏。
 */
#define WITH_DATA_READER_WRITER 1

/**
* 如果不支持输入法，请定义本宏。
 *
 */
#define WITH_IME_NULL 1
#define WITH_NULL_IM WITH_IME_NULL


/**
* 如果支持Google拼音输入法，请定义本宏。
 *
 */
// #define WITH_IME_PINYIN 1

/**
 * 启用输入法，但不想启用联想功能，请定义本宏。
 */
// #define WITHOUT_SUGGEST_WORDS 1

/**
 * 启用输入法，但不想启用联想功能，请定义本宏。
 *
 * #define WITHOUT_SUGGEST_WORDS 1
 */

/**
 * 如果有标准的malloc/free/calloc等函数，请定义本宏
 *
 */
#define HAS_STD_MALLOC 1

/**
 * 如果有优化版本的memcpy函数，请定义本宏
 *
 * #define HAS_FAST_MEMCPY 1
 */

/**
 * 进行时间统计。当绘制一帧需要的时间超过 25ms，会打印一些信息
 */
// #define ENABLE_PERFORMANCE_PROFILE 1

//脏矩形的个数
// #define TK_MAX_DIRTY_RECT_NR 10

// #define FRAGMENT_FRAME_BUFFER_SIZE (LCD_W * LCD_H)

#endif/*AWTK_CONFIG_H*/

