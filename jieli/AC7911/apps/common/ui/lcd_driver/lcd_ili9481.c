#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"
#include "uart.h"
#include "system/includes.h"
#include "gpio.h"
#include "app_config.h"
#if TCFG_LCD_ILI9481_ENABLE

#if HORIZONTAL_SCREEN /*横屏*/
#define LCD_W     480
#define LCD_H     320
#else                 /*竖屏*/
#define LCD_W     320
#define LCD_H     480
#endif

#define REGFLAG_DELAY 0x45

/*调用lcd_TE_driver完成数据发送*/
void ili9481_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}

void ili9481_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
{
    WriteCOM(0x2A);
    WriteDAT_8(xs >> 8);
    WriteDAT_8(xs);
    WriteDAT_8(xe >> 8);
    WriteDAT_8(xe);
    WriteCOM(0x2B);
    WriteDAT_8(ys >> 8);
    WriteDAT_8(ys);
    WriteDAT_8(ye >> 8);
    WriteDAT_8(ye);
}

void ili9481_clear_screen(u32 color)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);

    u8 *buf = malloc(LCD_W * LCD_H * 2);

    if (!buf) {
        printf("no men in %s \n", __func__);
        return;
    }

    for (u32 i = 0; i < LCD_W * LCD_H; i++) {
        buf[2 * i] = (color >> 8) & 0xff;
        buf[2 * i + 1] = color & 0xff;
    }

    WriteDAT_DMA(buf, LCD_W * LCD_H * 2);
    free(buf);
}

void ili9481_no_wait_Fill(u8 *img, u32 len)
{
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ili9481_Fill(u8 *img, u32 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ili9481_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void ili9481_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

static void ili9481_set_direction(u8 dir)
{
    WriteCOM(0x36);    //扫描方向控制

    if (dir == ROTATE_DEGREE_0) { //正向
#if HORIZONTAL_SCREEN
        WriteDAT_8(0x4c);
#else
        WriteDAT_8(0x4c);
#endif
    } else if (dir == ROTATE_DEGREE_180) { //翻转180
#if HORIZONTAL_SCREEN
#if defined USE_AWTK_UI_DEMO || defined USE_GUIX_UI_DEMO || defined USE_UGFX_UI_DEMO
        WriteDAT_8(0x04 | BIT(3) | BIT(5));
#else
        WriteDAT_8(0x04);
#endif

#else
        WriteDAT_8(0x04);
#endif
    }

    ili9481_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
}

static void ili9481_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void ili9481_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void ili9481_reset(void)
{
    printf("reset \n");
    lcd_rst_pinstate(1);
    lcd_rs_pinstate(1);
    lcd_cs_pinstate(1);
}

typedef struct {
    u8 cmd;
    u8 cnt;
    u8 dat[128];
} InitCode;

static const InitCode code1[] = {


    {0x01, 0},
    {REGFLAG_DELAY, 200},
    {0x13, 1, {0x00}},//开始正常显示模式
    {0x35, 1, {0x00}},//开TE 关TE 0x34
    {0x44, 2, {0x01, 0X50}}, //有关TE时间控制
    {0xc5, 1, {0x04}},//帧率控制寄存器
    {0xc5, 1, {0x07}},
    {0xE4, 1, {0xA0}},
    //屏幕和触摸共用一组3.3v屏拉电流导致触摸不灵敏
    //这里寄存器没有调到最佳 注释一下
    //显示存色的时候可能会出现像素点闪烁
    {0xd0, 3, {0x05, 0x40, 0x08}},
    {0xd1, 3, {0x00, 0x00, 0x10}},
    {0xd2, 2, {0x01, 0x00}},

    //电流够使用这组参数
    /* {0xd0, 3, {0x07, 0x41, 0x16}}, */
    /* {0xd1, 3, {0x00, 0x04, 0x1f}}, */
    /* {0xd2, 2, {0x01, 0x00}}, */

    {0xc0, 5, {0x00, 0x3b, 0x00, 0x02, 0x11}},
    {0xC8, 12, {0X00, 0x26, 0x21, 0x00, 0x00, 0x1f, 0x65, 0x23, 0x77, 0x00, 0x0f, 0x00}},
    /*{0xC8, 12, {0X00, 0x01, 0x47, 0x60, 0x04, 0x16, 0x03, 0x67, 0x67, 0x06, 0x0f, 0x00}},*/
    /*{0xC8,12, {0x00, 0x37, 0x25, 0x06, 0x04, 0x1e, 0x26, 0x42, 0x77, 0x44, 0x0f, 0x12}},*/
    /*{0xc8, 12, {0x00, 0x30, 0x36, 0x45, 0x04, 0x16, 0x37, 0x75, 0x77, 0x54, 0x0f, 0x00}},*/
    {0x2A, 4, {0X00, 0x00, 0x01, 0x3F}},
    {0x2B, 4, {0X00, 0x00, 0x01, 0xDF}},
    {0x3A, 1, {0x55}},
    {0x11, 0},//Sleep out
    {REGFLAG_DELAY, 200},

    {0x29, 0},
};

static void ili9481_init_code(const InitCode *code, u8 cnt)
{
    for (u8 i = 0; i < cnt; i++) {
        if (code[i].cmd == REGFLAG_DELAY) {
            lcd_delay(code[i].cnt);
        } else {
            WriteCOM(code[i].cmd);

            for (u8 j = 0; j < code[i].cnt; j++) {
                WriteDAT_8(code[i].dat[j]);
            }
        }
    }
}

static void ili9481_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    lcd_bl_pinstate(status);
}

static void ILI9481_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
{
    u32 len = 0;
    lcd_interface_non_block_wait();
    len = (xe + 1 - xs) * (ye + 1 - ys) * 2;
    WriteCOM(0x2A);
    WriteDAT_8(xs >> 8);
    WriteDAT_8(xs);
    WriteDAT_8(xe >> 8);
    WriteDAT_8(xe);
    WriteCOM(0x2B);
    WriteDAT_8(ys >> 8);
    WriteDAT_8(ys);
    WriteDAT_8(ye >> 8);
    WriteDAT_8(ye);
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ili9481_test(void)
{
    while (1) {
        ili9481_clear_screen(RED);
        printf("LCD_ILI9481_TSET_GRED\n");
        os_time_dly(100);
    }
}

static int ili9481_init(void)
{
    ili9481_reset();//触摸和显示共用reset脚需要放在一起初始化
#if TCFG_TOUCH_FT6236_ENABLE
    extern int FT6236_task_init(void);
    FT6236_task_init();
#endif
    printf("LCD_ili9481 init_start\n");
    lcd_bl_pinstate(lcd_bk_on);
    ili9481_init_code(code1, sizeof(code1) / sizeof(code1[0]));
#if defined USE_AWTK_UI_DEMO || defined USE_GUIX_UI_DEMO || defined USE_UGFX_UI_DEMO
    ili9481_set_direction(ROTATE_DEGREE_180);
#else
    ili9481_set_direction(ROTATE_DEGREE_0);
#endif
    init_TE(ili9481_Fill);
    /* ili9481_test(); */
    printf("LCD_ili9481 config succes\n");

    return 0;
}

REGISTER_LCD_DEV(LCD_ili9481) = {
    .name              = "ili9481",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = ili9481_init,
    .SetDrawArea       = ili9481_SetRange,
    .LCD_Draw          = ili9481_draw,
    .LCD_Draw_1        = ili9481_draw_1,
    .LCD_DrawToDev     = ili9481_no_wait_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full     = ILI9481_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen   = ili9481_clear_screen,
    .Reset             = ili9481_reset,
    .BackLightCtrl     = ili9481_led_ctrl,
};

#endif
