#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"
#include "gpio.h"

#if TCFG_LCD_ILI9488_ENABLE

#if HORIZONTAL_SCREEN /*横屏*/
#define LCD_W     480
#define LCD_H     320
#else                 /*竖屏*/
#define LCD_W     320
#define LCD_H     480
#endif

#define REGFLAG_DELAY 0x45


void ili9488_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void ili9488_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
{
    WriteCOM(0x2A);
    WriteDAT_8(ys >> 8);
    WriteDAT_8(ys);
    WriteDAT_8(ye >> 8);
    WriteDAT_8(ye);
    WriteCOM(0x2B);
    WriteDAT_8(xs >> 8);
    WriteDAT_8(xs);
    WriteDAT_8(xe >> 8);
    WriteDAT_8(xe);

}

void ili9488_clear_screen(u32 color)
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

void ili9488_no_wait_Fill(u8 *img, u32 len)
{
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ili9488_Fill(u8 *img, u16 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ili9488_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void ili9488_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

static void ili9488_set_direction(u8 dir)
{
    WriteCOM(0x36);    //扫描方向控制

    if (dir == ROTATE_DEGREE_0) { //
#if HORIZONTAL_SCREEN
        WriteDAT_8(0x40);
#else
        WriteDAT_8(0x00);
#endif
        ili9488_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
    } else if (dir == ROTATE_DEGREE_180) { //翻转180
#if HORIZONTAL_SCREEN
        WriteDAT_8(0xc0);
#else
        WriteDAT_8(0x80);
#endif
    }
}

static void ili9488_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void ili9488_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void ili9488_reset(void)
{
    printf("reset \n");
    lcd_rst_pinstate(1);
    lcd_rs_pinstate(1);
    lcd_cs_pinstate(1);

    lcd_rst_pinstate(1);
    lcd_delay(60);
    lcd_rst_pinstate(0);
    lcd_delay(10);
    lcd_rst_pinstate(1);
    lcd_delay(100);
}

typedef struct {
    u8 cmd;
    u8 cnt;
    u8 dat[128];
} InitCode;

static const InitCode code1[] = {
    {0x21, 1, {0x00}},//开启数据翻转模式 开源板新屏配置有点奇怪 需要翻转数据 2023-5-15  如果需要调试同一个驱动型号的屏 把该行注释掉
    {0x13, 1, {0x00}},//开始正常显示模式
    {0x35, 1, {0x00}},//开启te 0x34关TE
    {0xB1, 2, {0x90, 0x11}}, //A0 11 60fps
    {0x36, 1, {0xd8}},//可以控制旋转/flip/rota  //b7 y b6 x b5 v b4 l b3 rgb b2 h
    {0xC1, 1, {0x41}},
    {0XF7, 4, {0xA9, 0x51, 0x2C, 0x82}},
    {0xC1, 1, {0x41}},
    {0xC0, 2, {0x0f, 0x0f}},
    {0xC2, 1, {0x22}},
    {0XC5, 3, {0x00, 0x53, 0x80}},
    {0xB4, 1, {0x02}},
    {0xB7, 1, {0xc6}},
    {0xB6, 2, {0x02, 0x42}},
    {0xBE, 2, {0x00, 0x04}},
    {0xE9, 1, {0x00}},
    {0x3a, 1, {0x55}},
    {0x11, 0},//Sleep out
    {REGFLAG_DELAY, 200},
    {0x29, 0},//开显示
};

static void ili9488_init_code(const InitCode *code, u8 cnt)
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

static void ILI9488_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
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

static void ili9488_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    /*lcd_bl_pinstate(status);*/
}

void ili9488_test(void)
{
    lcd_bl_pinstate(lcd_bk_on);
    while (1) {
        os_time_dly(100);
        ili9488_clear_screen(WHITE);
        printf("LCD_ili9488_TSET_WHITE\n");
        os_time_dly(100);
        ili9488_clear_screen(BLACK);
        printf("LCD_ili9488_TSET_BLACK\n");
        os_time_dly(100);
        ili9488_clear_screen(RED);
        printf("LCD_ili9488_TSET_RED\n");
        os_time_dly(100);
        ili9488_clear_screen(YELLOW);
        printf("LCD_ili9488_TSET_YELLOW\n");
    }
}

static int ili9488_init(void)
{
    ili9488_reset();
#if TCFG_TOUCH_FT6236_ENABLE
    extern int FT6236_task_init(void);
    FT6236_task_init();
#endif
    printf("LCD_ili9488 init_start\n");
    ili9488_init_code(code1, sizeof(code1) / sizeof(code1[0]));
    /*ili9488_set_direction(ROTATE_DEGREE_0);*/
    init_TE(ili9488_Fill);
    /* ili9488_test(); */
    printf("LCD_ili9488 config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_ili9488) = {
    .name              = "ili9488",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = ili9488_init,
    .SetDrawArea       = ili9488_SetRange,
    .LCD_Draw          = ili9488_draw,
    .LCD_Draw_1        = ili9488_draw_1,
    .LCD_DrawToDev     = ili9488_no_wait_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full     = ILI9488_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen   = ili9488_clear_screen,
    .Reset             = ili9488_reset,
    .BackLightCtrl     = ili9488_led_ctrl,
};

#endif


