#ifndef __LCD_CONFIG_H__
#define __LCD_CONFIG_H__
#include "app_config.h"

#define LCD_W 320
#define LCD_H 480

#define LCD_YUV420_DATA_SIZE    LCD_W*LCD_H*3/2
#define LCD_RGB565_DATA_SIZE    LCD_W*LCD_H*2
#define LCD_RGB888_DATA_SIZE    LCD_W*LCD_H*3

#endif

