/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.io/license.html
 */

#include "lcd_config.h"

#define DUAL_THREAD_ACCELERATION  //利用双核双线程加速,目前分割工作量实测可加速33%, 但是也会占据另外一个核的33%性能

#ifdef DUAL_THREAD_ACCELERATION
#include "os/os_api.h"
static OS_SEM mcu_lcd_physics_flush_sem, mcu_lcd_physics_flush_end_sem;
#endif


// Set this to your frame buffer pixel format.
#ifndef GDISP_LLD_PIXELFORMAT
#define GDISP_LLD_PIXELFORMAT		GDISP_PIXELFORMAT_RGB565
#endif

#ifdef GDISP_DRIVER_VMT

static LLDCOLOR_TYPE frame_buffer[2][LCD_W * LCD_H];
static void _mcu_lcd_physics_flush(GDisplay *g)
{
    fbInfo *fbi = &(PRIV(g)->fbi);

    static LLDCOLOR_TYPE fb2[LCD_W * LCD_H]; //由于脏矩形，下面转换会改到干净矩形的区域,所以需要多申请一块
    unsigned char *data = (unsigned char *)fb2;
    for (int i = 0; i < LCD_W * LCD_H; i++) {
        unsigned short color = frame_buffer[!PRIV(g)->fbi.idx][i] ;
        data[2 * i] = (color >> 8) & 0xff;
        data[2 * i + 1] = color & 0xff;
    }
#if 1
    lcd_lvgl_full(0, LCD_W - 1, 0, LCD_H - 1, data);
#else //只刷新脏矩形部分

    static LLDCOLOR_TYPE fb3[LCD_W * LCD_H];
    unsigned short 	x1 = fbi->x1[!fbi->idx];
    unsigned short 	y1 = fbi->y1[!fbi->idx];
    unsigned short 	x2 = fbi->x2[!fbi->idx];
    unsigned short 	y2 = fbi->y2[!fbi->idx];
    unsigned int 	pos = 0;
    unsigned short cx = fbi->x2[!fbi->idx] - fbi->x1[!fbi->idx] + 1;

#define xyaddr(x, y)			((x) + LCD_W * (y) )
    for (LLDCOLOR_TYPE *c = &fb2[xyaddr(x1, y1)] ; y1 <= y2 ; c += LCD_W, y1 += 1, pos += LCD_W) {
        memcpy(&fb3[pos], c, cx * 2);
    }

    lcd_lvgl_full(fbi->x1[!fbi->idx], fbi->x2[!fbi->idx] - 1, fbi->y1[!fbi->idx], fbi->y2[!fbi->idx] - 1, fb3);
#endif

#if 1
    static int fps_time_hdl;
    static unsigned int fps;
    ++fps;
    if (time_lapse(&fps_time_hdl, 1000)) {
        printf("UGFX FPS=%d\r\n", fps);
        fps = 0;
    }
#endif
}
static void mcu_lcd_physics_flush(GDisplay *g)
{
#ifdef DUAL_THREAD_ACCELERATION
    os_sem_pend(&mcu_lcd_physics_flush_end_sem, 0);
    os_sem_post(&mcu_lcd_physics_flush_sem);
#else
    _mcu_lcd_physics_flush(g);
#endif
}

#ifdef DUAL_THREAD_ACCELERATION
static void ugfx_mcu_lcd_physics_flush_task(GDisplay *g)
{
    while (1) {
        os_sem_pend(&mcu_lcd_physics_flush_sem, 0);
        _mcu_lcd_physics_flush(g);
        os_sem_post(&mcu_lcd_physics_flush_end_sem);
    }
}
#endif

static void board_init(GDisplay *g, fbInfo *fbi)
{
    // TODO: Initialize your frame buffer device here

    // TODO: Set the details of the frame buffer
    g->g.Width = LCD_W;
    g->g.Height = LCD_H;
    g->g.Backlight = 100;
    g->g.Contrast = 50;
    fbi->linelen = g->g.Width * sizeof(LLDCOLOR_TYPE);				// bytes per row

    fbi->pixels[0] = frame_buffer[0];									// pointer to the memory frame buffer
    fbi->pixels[1] = frame_buffer[1];									// pointer to the memory frame buffer

    fbi->x1[0] = g->g.Width;
    fbi->y1[0] = g->g.Height;
    fbi->x2[0] = -1;
    fbi->y2[0] = -1;

    fbi->x1[1] = g->g.Width;
    fbi->y1[1] = g->g.Height;
    fbi->x2[1] = -1;
    fbi->y2[1] = -1;

    fbi->idx = 0;
#ifdef DUAL_THREAD_ACCELERATION
    lcd_interface_set_non_block(0);// mcu lcd硬件接口要求阻塞释放cpu,否则提前释放信号有概率花屏
    os_sem_create(&mcu_lcd_physics_flush_sem, 0);
    os_sem_create(&mcu_lcd_physics_flush_end_sem, 1);
    thread_fork("ugfx_mcu_lcd_flush_task", 26, 200, 0, NULL, ugfx_mcu_lcd_physics_flush_task, g);
#endif
}

#if GDISP_HARDWARE_FLUSH
static void board_flush(GDisplay *g)
{
    fbInfo *fbi = &(PRIV(g)->fbi);

#if 1 //TODO: 还需要实现 swap buf
#if 0
    unsigned short 	x1 = fbi->x1[fbi->idx];
    unsigned short 	y1 = fbi->y1[fbi->idx];
    unsigned short 	x2 = fbi->x2[fbi->idx];
    unsigned short 	y2 = fbi->y2[fbi->idx];
    unsigned int 	pos = 0;
    unsigned short cx = fbi->x2[fbi->idx] - fbi->x1[fbi->idx] + 1;

#define xyaddr(x, y)			((x) + LCD_W * (y) )
    for (LLDCOLOR_TYPE *c = &fbi->pixels[fbi->idx][xyaddr(x1, y1)] ; y1 <= y2 ; c += LCD_W, y1 += 1, pos += LCD_W) {
        memcpy(&fbi->pixels[!fbi->idx][pos], c, cx * 2);
    }
#else
    memcpy(fbi->pixels[!fbi->idx], fbi->pixels[fbi->idx], g->g.Width * g->g.Height * sizeof(LLDCOLOR_TYPE));
#endif
#endif

    fbi->idx = !fbi->idx; //swap fb

    mcu_lcd_physics_flush(g);
}
#endif

#if GDISP_NEED_CONTROL
static void board_backlight(GDisplay *g, gU8 percent)
{
    // TODO: Can be an empty function if your hardware doesn't support this
    (void) g;
    (void) percent;
}

static void board_contrast(GDisplay *g, gU8 percent)
{
    // TODO: Can be an empty function if your hardware doesn't support this
    (void) g;
    (void) percent;
}

static void board_power(GDisplay *g, gPowermode pwr)
{
    // TODO: Can be an empty function if your hardware doesn't support this
    (void) g;
    (void) pwr;
}
#endif

#endif /* GDISP_LLD_BOARD_IMPLEMENTATION */
