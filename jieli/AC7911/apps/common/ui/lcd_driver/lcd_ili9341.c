
#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "app_config.h"
#include "lcd_drive.h"
#include "lcd_config.h"


#if TCFG_LCD_ILI9341_ENABLE

#if HORIZONTAL_SCREEN /*横屏*/
#define LCD_W     320
#define LCD_H     240
#else                 /*竖屏*/
#define LCD_W     240
#define LCD_H     320
#endif

static void ILI9341_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}

static void ILI9341_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
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

static void ILI9341_io_init()
{
    lcd_rs_pinstate(0);
    lcd_cs_pinstate(1);
    lcd_rst_pinstate(1);
}


void ILI9341_set_direction(u8 dir)
{
    WriteCOM(0x36);    //扫描方向控制

    if (dir == ROTATE_DEGREE_0) { //正向
#if HORIZONTAL_SCREEN
        WriteDAT_8(0xd8);
#else
        WriteDAT_8(0x58);
#endif
    } else if (dir == ROTATE_DEGREE_180) { //翻转180
#if HORIZONTAL_SCREEN
        WriteDAT_8(0x18);
#else
        WriteDAT_8(0x98);
#endif
    }
    ILI9341_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
}

void ILI9341_clear_screen(u16 color)
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

void ILI9341_Fill(u8 *img, u16 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ILI9341_Sleep_In_Mode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void ILI9341_Sleep_Out_Mode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

void st7735_shown_image(u8 *buff, u16 x_addr, u16 y_addr, u16 width, u16 height)
{
    lcd_interface_non_block_wait();
    u32 i = 0;
    ILI9341_set_xy_addr(x_addr, y_addr, width, height);
    WriteDAT_DMA(buff, width * height * 2);
}

static void ILI9341_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void ILI9341_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void ILI9341_draw_2(int x, int y, int w, int h, char *img)//摄像头层任意位置修改数据
{
    user_send_data_ready(x, y, w, h, img);
}

static void ILI9341_reset(void)
{
    printf("lcd reset  \n");
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

static void ILI9341_led_ctrl(u8 status)
{
    lcd_bl_pinstate(status);
}

void ILI9341_clr(void)
{
    ILI9341_clear_screen(0x00);
}

static void ILI9341_reg_cfg(void)
{
    lcd_rst_pinstate(1);
    lcd_delay(50);
    lcd_rst_pinstate(0);
    lcd_delay(20);
    lcd_rst_pinstate(1);
    lcd_delay(120);

    WriteCOM(0x01);
    WriteDAT_8(0x01);
    WriteCOM(0x11);
    WriteDAT_8(0x00);
    lcd_delay(120);
    lcd_delay(120);

    /*********************************/
    WriteCOM(0x35);//开TE
    WriteDAT_8(0x00);

    WriteCOM(0xb1);//调节屏幕自身刷新帧率  这个直接影响到一个TE周期的时间
    WriteDAT_8(0x00);
    WriteDAT_8(0x1f); //60fps
    /*********************************/

    WriteCOM(0xcf);
    WriteDAT_8(0x00);
    WriteDAT_8(0xaa);
    WriteDAT_8(0xe0);
    WriteCOM(0xed);
    WriteDAT_8(0x67);
    WriteDAT_8(0x03);
    WriteDAT_8(0x12);
    WriteDAT_8(0x81);
    WriteCOM(0xe8);
    WriteDAT_8(0x85);
    WriteDAT_8(0x11);
    WriteDAT_8(0x78);
    WriteCOM(0xcb);
    WriteDAT_8(0x39);
    WriteDAT_8(0x2c);
    WriteDAT_8(0x00);
    WriteDAT_8(0x34);
    WriteDAT_8(0x02);
    WriteCOM(0xea);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);

    WriteCOM(0xC0); //Power control
    WriteDAT_8(0x21); //VRH[5:0]

    WriteCOM(0xC1); //Power control
    WriteDAT_8(0x11); //SAP[2:0];BT[3:0]

    WriteCOM(0xC5); //VCM control
    WriteDAT_8(0x24);
    WriteDAT_8(0x3c);

    WriteCOM(0xc7);
    WriteDAT_8(0xb7);

    WriteCOM(0x36);
    WriteDAT_8(0x48);

    WriteCOM(0x3a); // Memory Access Control
    WriteDAT_8(0x55);

    WriteCOM(0xf2); // Memory Access Control
    WriteDAT_8(0x00);

    WriteCOM(0x26);
    WriteDAT_8(0x01);

    WriteCOM(0xb7); // Display Function Control
    WriteDAT_8(0x06);

    WriteCOM(0xE0); //Set Gamma
    WriteDAT_8(0x0f);
    WriteDAT_8(0x1b);
    WriteDAT_8(0x19);
    WriteDAT_8(0x0c);
    WriteDAT_8(0x0d);
    WriteDAT_8(0x07);
    WriteDAT_8(0x44);
    WriteDAT_8(0xa9);
    WriteDAT_8(0x2f);
    WriteDAT_8(0x08);
    WriteDAT_8(0x0d);
    WriteDAT_8(0x03);
    WriteDAT_8(0x10);
    WriteDAT_8(0x0b);
    WriteDAT_8(0x00);

    WriteCOM(0XE1); //Set Gamma
    WriteDAT_8(0x00);
    WriteDAT_8(0x24);
    WriteDAT_8(0x26);
    WriteDAT_8(0x03);
    WriteDAT_8(0x12);
    WriteDAT_8(0x08);
    WriteDAT_8(0x3c);
    WriteDAT_8(0x56);
    WriteDAT_8(0x50);
    WriteDAT_8(0x07);
    WriteDAT_8(0x12);
    WriteDAT_8(0x0c);
    WriteDAT_8(0x2f);
    WriteDAT_8(0x34);
    WriteDAT_8(0x0f);

    WriteCOM(0X13); //
    WriteDAT_8(0x00);

    lcd_delay(150);
    WriteCOM(0X29); //Set Gamma
    lcd_delay(50);

}

static void ILI9341_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, char *img)
{
    lcd_interface_non_block_wait();
    u32 len = 0;
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

static void ILI9341_test(void)
{
    lcd_bl_pinstate(lcd_bk_on);
    while (1) {
        os_time_dly(100);
        ILI9341_clear_screen(RED);
        printf("LCD_ILI9341_TSET_BLUE\n");
        os_time_dly(100);
        ILI9341_clear_screen(GREEN);
        printf("LCD_ILI9341_TSET_GRED\n");
        os_time_dly(100);
        ILI9341_clear_screen(BLUE);
        printf("LCD_ILI9341_TSET_BRRED\n");
        os_time_dly(100);
        ILI9341_clear_screen(YELLOW);
        printf("LCD_ILI9341_TSET_YELLOW\n");
    }
}

static int ILI9341_init(void)
{
    ILI9341_io_init();
    /* ILI9341_reset(); */

    printf("LCD_ILI9341 config...\n");
    ILI9341_reg_cfg();
    ILI9341_set_direction(ROTATE_DEGREE_0);
    init_TE(ILI9341_Fill);
    lcd_bl_pinstate(lcd_bk_on);
    /* ILI9341_test(); */
    printf("LCD_ILI9341 config succes\n");
    return 0;
}

// *INDENT-OFF*
REGISTER_LCD_DEV(LCD_ILI9341) = {
    .name              = "ili9341",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = ILI9341_init,
    .SetDrawArea       = ILI9341_SetRange,
    .LCD_Draw          = ILI9341_draw,
    .LCD_Draw_1        = ILI9341_draw_1,
    .LCD_Draw_2        = ILI9341_draw_2,
    .LCD_DrawToDev     = ILI9341_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full     = ILI9341_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen   = ILI9341_clear_screen,
    .Reset             = ILI9341_reset,
    .BackLightCtrl     = ILI9341_led_ctrl,
};
// *INDENT-ON*

#endif

