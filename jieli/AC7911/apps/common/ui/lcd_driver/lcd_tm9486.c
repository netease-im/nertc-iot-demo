#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"
#include "gpio.h"
/*  HX9486驱动说明 该驱动测试时使用的 wl80 79系列
 *  由于该IC推屏能力不够强 推屏的帧数较低 大概在25帧左右
 *  在推屏过程中需要使用TE屏幕帧中断 不然会有条纹
 *  由于HX9486横屏配置无法调出没有条纹的配置
 *  所有只能使用竖屏加RGB旋转来实现UI横屏显示
 */

/* //pap的这个三个配置如下 在板级文件中进行修改
    .timing_setup 	= 0,					//具体看pap.h
    .timing_hold  	= 0,					//具体看pap.h
    .timing_width 	= 1,					//具体看pap.h
*/

#if TCFG_LCD_TM9486X_ENABLE
//版本号：图传：CS_16TC_TM9486X_38TND_230415A
//       对讲：CS_16DJ_TM9486X_38TND_230415A
//分辨率：320*480
//IC型号：68042-147/9481-147通用， 7916 新主控   TM9486新 金星壹

#define LCD_W 480
#define LCD_H 800


#define ROTATE_DEGREE_0  	0
#define ROTATE_DEGREE_180       1

#define WHITE         	 0xFFFF
#define BLACK         	 0x0000
#define BLUE         	 0x001F
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色i

#define READ_ID 	0x04
#define REGFLAG_DELAY 0x45

#define DMA_DSPLY	1 //DMA operation

static u8 ui_data_ready = 0;

/*调用lcd_TE_driver完成数据发送*/
extern void send_date_ready();
void msleep(unsigned int ms);

#define lcd_delay(x) msleep(x)

static void WriteCOM_2bit(unsigned int cmd)
{
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(0);//cmd
    lcd_send_byte((cmd >> 8) & 0xff);
    lcd_send_byte(cmd & 0xff);
    lcd_cs_pinstate(1);
}

void WriteDAT_2bit(u16 dat)
{
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    /* lcd_send_byte(dat); */
    lcd_send_byte((dat >> 8) & 0xff);
    lcd_send_byte(dat & 0xff);
    lcd_cs_pinstate(1);
}

static u32 Read_ID(u8 index)
{
    u32 id = 0;
    WriteCOM_2bit(index);
    /* id = ReadDAT_16() & 0xff00; */
    id = ReadDAT_16();
    id |= ReadDAT_16() << 16;
    return id;
}

void HX8357_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void HX8357_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
{
//    WriteCOM_2bit(0x2A);
//    WriteDAT_2bit(ys >> 8);
//    WriteDAT_2bit(ys);
//    WriteDAT_2bit(ye >> 8);
//    WriteDAT_2bit(ye);
//    WriteCOM_2bit(0x2B);
//    WriteDAT_2bit(xs >> 8);
//    WriteDAT_2bit(xs);
//    WriteDAT_2bit(xe >> 8);
//    WriteDAT_2bit(xe);

}

void HX8357_clear_screen(u32 color)
{
    lcd_interface_non_block_wait();
    WriteCOM_2bit(0x2c00);

    u8 *buf = malloc(800 * 480 * 2);
    if (!buf) {
        printf("no men in %s \n", __func__);
        return;
    }
    for (u32 i = 0; i < 800 * 480; i++) {
        buf[2 * i] = (color >> 8) & 0xff;
        buf[2 * i + 1] = color & 0xff;
    }
    WriteDAT_DMA(buf, 800 * 480 * 2);
    free(buf);
}

void HX8357_Fill(u8 *img, u32 len)
{
    lcd_interface_non_block_wait();
    /* WriteCOM_2bit(0x2c00); */
    WriteDAT_DMA(img, len);
}

void HX8357_SleepInMode(void)
{
//    WriteCOM_2bit(0x10); //Sleep in
//    lcd_delay(120); //Delay 120ms
}

void HX8357_SleepOutMode(void)
{
//    WriteCOM_2bit(0x11); //Sleep out
//    lcd_delay(120);  //Delay 120ms
}

void st7789_shown_image(u8 *buff, u16 x_addr, u16 y_addr, u16 width, u16 height)
{
    HX8357_SetRange(x_addr, y_addr, width, height);
    WriteDAT_DMA(buff, width * height * 2);
}

static void HX8357_set_direction(u8 dir)
{
    WriteCOM_2bit(0x36);    //扫描方向控制

    if (dir == ROTATE_DEGREE_0) { //正向
#if HORIZONTAL_SCREEN
        WriteDAT_2bit(0x40);
#else
        WriteDAT_2bit(0x00);
#endif
        HX8357_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
    } else if (dir == ROTATE_DEGREE_180) { //翻转180
#if HORIZONTAL_SCREEN
        WriteDAT_2bit(0xc0);
#else
        WriteDAT_2bit(0x80);
#endif
    }
}

static void HX8357_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void HX8357_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void HX8357_reset(void)
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
static void  lcd_35510_init()
{
    WriteCOM_2bit(0xF000);
    WriteDAT_2bit(0xAA);
    WriteCOM_2bit(0xF001);
    WriteDAT_2bit(0x55);
    WriteCOM_2bit(0xF002);
    WriteDAT_2bit(0x52);
    WriteCOM_2bit(0xF003);
    WriteDAT_2bit(0x08);
    WriteCOM_2bit(0xF004);
    WriteDAT_2bit(0x01);

    WriteCOM_2bit(0xB000);
    WriteDAT_2bit(0x09);
    WriteCOM_2bit(0xB001);
    WriteDAT_2bit(0x09);
    WriteCOM_2bit(0xB002);
    WriteDAT_2bit(0x09);

    WriteCOM_2bit(0xB600);
    WriteDAT_2bit(0x34);
    WriteCOM_2bit(0xB601);
    WriteDAT_2bit(0x34);
    WriteCOM_2bit(0xB602);
    WriteDAT_2bit(0x34);

    WriteCOM_2bit(0xB100);
    WriteDAT_2bit(0x09);
    WriteCOM_2bit(0xB101);
    WriteDAT_2bit(0x09);
    WriteCOM_2bit(0xB102);
    WriteDAT_2bit(0x09);

    WriteCOM_2bit(0xB700);
    WriteDAT_2bit(0x24);
    WriteCOM_2bit(0xB701);
    WriteDAT_2bit(0x24);
    WriteCOM_2bit(0xB702);
    WriteDAT_2bit(0x24);

    WriteCOM_2bit(0xBF00);
    WriteDAT_2bit(0x01);

    WriteCOM_2bit(0xB200);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xB201);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xB202);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xB300);
    WriteDAT_2bit(0x05);
    WriteCOM_2bit(0xB301);
    WriteDAT_2bit(0x05);
    WriteCOM_2bit(0xB302);
    WriteDAT_2bit(0x05);

    WriteCOM_2bit(0xB900);
    WriteDAT_2bit(0x24);
    WriteCOM_2bit(0xB901);
    WriteDAT_2bit(0x24);
    WriteCOM_2bit(0xB902);
    WriteDAT_2bit(0x24);

    WriteCOM_2bit(0xB500);
    WriteDAT_2bit(0x0b);
    WriteCOM_2bit(0xB501);
    WriteDAT_2bit(0x0b);
    WriteCOM_2bit(0xB502);
    WriteDAT_2bit(0x0b);

    WriteCOM_2bit(0xBA00);
    WriteDAT_2bit(0x34);
    WriteCOM_2bit(0xBA01);
    WriteDAT_2bit(0x24);
    WriteCOM_2bit(0xBA02);
    WriteDAT_2bit(0x24);

    WriteCOM_2bit(0xBC00);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xBC01);
    WriteDAT_2bit(0xA3);
    WriteCOM_2bit(0xBC02);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xBD00);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xBD01);
    WriteDAT_2bit(0xa3);
    WriteCOM_2bit(0xBD02);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xBE00);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xBE01);
    WriteDAT_2bit(0x40);

    WriteCOM_2bit(0xD100);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD101);
    WriteDAT_2bit(0x2D);
    WriteCOM_2bit(0xD102);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD103);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD104);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD105);
    WriteDAT_2bit(0x66);
    WriteCOM_2bit(0xD106);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD107);
    WriteDAT_2bit(0x7F);
    WriteCOM_2bit(0xD108);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD109);
    WriteDAT_2bit(0x94);
    WriteCOM_2bit(0xD10A);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD10B);
    WriteDAT_2bit(0xB4);
    WriteCOM_2bit(0xD10C);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD10D);
    WriteDAT_2bit(0xD0);
    WriteCOM_2bit(0xD10E);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD10F);
    WriteDAT_2bit(0xFE);
    WriteCOM_2bit(0xD110);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD111);
    WriteDAT_2bit(0x22);
    WriteCOM_2bit(0xD112);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD113);
    WriteDAT_2bit(0x59);
    WriteCOM_2bit(0xD114);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD115);
    WriteDAT_2bit(0x87);
    WriteCOM_2bit(0xD116);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD117);
    WriteDAT_2bit(0xD1);
    WriteCOM_2bit(0xD118);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD119);
    WriteDAT_2bit(0x0C);
    WriteCOM_2bit(0xD11A);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD11B);
    WriteDAT_2bit(0x0E);
    WriteCOM_2bit(0xD11C);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD11D);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD11E);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD11F);
    WriteDAT_2bit(0x7E);
    WriteCOM_2bit(0xD120);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD121);
    WriteDAT_2bit(0xA4);
    WriteCOM_2bit(0xD122);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD123);
    WriteDAT_2bit(0xD7);
    WriteCOM_2bit(0xD124);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD125);
    WriteDAT_2bit(0xFA);
    WriteCOM_2bit(0xD126);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD127);
    WriteDAT_2bit(0x28);
    WriteCOM_2bit(0xD128);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD129);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD12A);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD12B);
    WriteDAT_2bit(0x61);
    WriteCOM_2bit(0xD12C);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD12D);
    WriteDAT_2bit(0x6E);
    WriteCOM_2bit(0xD12E);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD12F);
    WriteDAT_2bit(0x78);
    WriteCOM_2bit(0xD130);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD131);
    WriteDAT_2bit(0x7E);
    WriteCOM_2bit(0xD132);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD133);
    WriteDAT_2bit(0x7F);

    WriteCOM_2bit(0xD200);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD201);
    WriteDAT_2bit(0x2D);
    WriteCOM_2bit(0xD202);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD203);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD204);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD205);
    WriteDAT_2bit(0x66);
    WriteCOM_2bit(0xD206);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD207);
    WriteDAT_2bit(0x7F);
    WriteCOM_2bit(0xD208);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD209);
    WriteDAT_2bit(0x94);
    WriteCOM_2bit(0xD20A);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD20B);
    WriteDAT_2bit(0xB4);
    WriteCOM_2bit(0xD20C);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD20D);
    WriteDAT_2bit(0xD0);
    WriteCOM_2bit(0xD20E);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD20F);
    WriteDAT_2bit(0xFE);
    WriteCOM_2bit(0xD210);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD211);
    WriteDAT_2bit(0x22);
    WriteCOM_2bit(0xD212);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD213);
    WriteDAT_2bit(0x59);
    WriteCOM_2bit(0xD214);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD215);
    WriteDAT_2bit(0x87);
    WriteCOM_2bit(0xD216);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD217);
    WriteDAT_2bit(0xD1);
    WriteCOM_2bit(0xD218);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD219);
    WriteDAT_2bit(0x0C);
    WriteCOM_2bit(0xD21A);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD21B);
    WriteDAT_2bit(0x0E);
    WriteCOM_2bit(0xD21C);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD21D);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD21E);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD21F);
    WriteDAT_2bit(0x7E);
    WriteCOM_2bit(0xD220);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD221);
    WriteDAT_2bit(0xA4);
    WriteCOM_2bit(0xD222);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD223);
    WriteDAT_2bit(0xD7);
    WriteCOM_2bit(0xD224);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD225);
    WriteDAT_2bit(0xFA);
    WriteCOM_2bit(0xD226);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD227);
    WriteDAT_2bit(0x28);
    WriteCOM_2bit(0xD228);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD229);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD22A);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD22B);
    WriteDAT_2bit(0x61);
    WriteCOM_2bit(0xD22C);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD22D);
    WriteDAT_2bit(0x6E);
    WriteCOM_2bit(0xD22E);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD22F);
    WriteDAT_2bit(0x78);
    WriteCOM_2bit(0xD230);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD231);
    WriteDAT_2bit(0x7E);
    WriteCOM_2bit(0xD232);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD233);
    WriteDAT_2bit(0x7F);

    WriteCOM_2bit(0xD300);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD301);
    WriteDAT_2bit(0x2D);
    WriteCOM_2bit(0xD302);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD303);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD304);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD305);
    WriteDAT_2bit(0x66);
    WriteCOM_2bit(0xD306);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD307);
    WriteDAT_2bit(0x7F);
    WriteCOM_2bit(0xD308);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD309);
    WriteDAT_2bit(0x94);
    WriteCOM_2bit(0xD30A);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD30B);
    WriteDAT_2bit(0xB4);
    WriteCOM_2bit(0xD30C);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD30D);
    WriteDAT_2bit(0xD0);
    WriteCOM_2bit(0xD30E);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD30F);
    WriteDAT_2bit(0xFE);

    WriteCOM_2bit(0xD310);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD311);
    WriteDAT_2bit(0x22);
    WriteCOM_2bit(0xD312);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD313);
    WriteDAT_2bit(0x59);
    WriteCOM_2bit(0xD314);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD315);
    WriteDAT_2bit(0x87);
    WriteCOM_2bit(0xD316);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD317);
    WriteDAT_2bit(0xD1);
    WriteCOM_2bit(0xD318);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD319);
    WriteDAT_2bit(0x0C);
    WriteCOM_2bit(0xD31A);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD31B);
    WriteDAT_2bit(0x0E);
    WriteCOM_2bit(0xD31C);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD31D);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD31E);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD31F);
    WriteDAT_2bit(0x7E);

    WriteCOM_2bit(0xD320);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD321);
    WriteDAT_2bit(0xA4);
    WriteCOM_2bit(0xD322);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD323);
    WriteDAT_2bit(0xD7);
    WriteCOM_2bit(0xD324);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD325);
    WriteDAT_2bit(0xFA);
    WriteCOM_2bit(0xD326);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD327);
    WriteDAT_2bit(0x28);
    WriteCOM_2bit(0xD328);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD329);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD32A);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD32B);
    WriteDAT_2bit(0x61);
    WriteCOM_2bit(0xD32C);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD32D);
    WriteDAT_2bit(0x6E);
    WriteCOM_2bit(0xD32E);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD32F);
    WriteDAT_2bit(0x78);

    WriteCOM_2bit(0xD330);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD331);
    WriteDAT_2bit(0x7E);
    WriteCOM_2bit(0xD332);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD333);
    WriteDAT_2bit(0x7F);

    WriteCOM_2bit(0xD400);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD401);
    WriteDAT_2bit(0x2D);
    WriteCOM_2bit(0xD402);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD403);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD404);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD405);
    WriteDAT_2bit(0x66);
    WriteCOM_2bit(0xD406);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD407);
    WriteDAT_2bit(0x7F);
    WriteCOM_2bit(0xD408);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD409);
    WriteDAT_2bit(0x94);
    WriteCOM_2bit(0xD40A);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD40B);
    WriteDAT_2bit(0xB4);
    WriteCOM_2bit(0xD40C);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD40D);
    WriteDAT_2bit(0xD0);
    WriteCOM_2bit(0xD40E);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD40F);
    WriteDAT_2bit(0xFE);

    WriteCOM_2bit(0xD410);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD411);
    WriteDAT_2bit(0x22);
    WriteCOM_2bit(0xD412);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD413);
    WriteDAT_2bit(0x59);
    WriteCOM_2bit(0xD414);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD415);
    WriteDAT_2bit(0x87);
    WriteCOM_2bit(0xD416);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD417);
    WriteDAT_2bit(0xD1);
    WriteCOM_2bit(0xD418);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD419);
    WriteDAT_2bit(0x0C);
    WriteCOM_2bit(0xD41A);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD41B);
    WriteDAT_2bit(0x0E);
    WriteCOM_2bit(0xD41C);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD41D);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD41E);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD41F);
    WriteDAT_2bit(0x7E);

    WriteCOM_2bit(0xD420);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD421);
    WriteDAT_2bit(0xA4);
    WriteCOM_2bit(0xD422);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD423);
    WriteDAT_2bit(0xD7);
    WriteCOM_2bit(0xD424);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD425);
    WriteDAT_2bit(0xFA);
    WriteCOM_2bit(0xD426);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD427);
    WriteDAT_2bit(0x28);
    WriteCOM_2bit(0xD428);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD429);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD42A);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD42B);
    WriteDAT_2bit(0x61);
    WriteCOM_2bit(0xD42C);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD42D);
    WriteDAT_2bit(0x6E);
    WriteCOM_2bit(0xD42E);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD42F);
    WriteDAT_2bit(0x78);

    WriteCOM_2bit(0xD430);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD431);
    WriteDAT_2bit(0x7E);
    WriteCOM_2bit(0xD432);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD433);
    WriteDAT_2bit(0x7F);

    WriteCOM_2bit(0xD500);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD501);
    WriteDAT_2bit(0x2D);
    WriteCOM_2bit(0xD502);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD503);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD504);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD505);
    WriteDAT_2bit(0x66);
    WriteCOM_2bit(0xD506);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD507);
    WriteDAT_2bit(0x7F);
    WriteCOM_2bit(0xD508);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD509);
    WriteDAT_2bit(0x94);
    WriteCOM_2bit(0xD50A);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD50B);
    WriteDAT_2bit(0xB4);
    WriteCOM_2bit(0xD50C);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD50D);
    WriteDAT_2bit(0xD0);
    WriteCOM_2bit(0xD50E);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD50F);
    WriteDAT_2bit(0xFE);

    WriteCOM_2bit(0xD510);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD511);
    WriteDAT_2bit(0x22);
    WriteCOM_2bit(0xD512);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD513);
    WriteDAT_2bit(0x59);
    WriteCOM_2bit(0xD514);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD515);
    WriteDAT_2bit(0x87);
    WriteCOM_2bit(0xD516);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD517);
    WriteDAT_2bit(0xD1);
    WriteCOM_2bit(0xD518);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD519);
    WriteDAT_2bit(0x0C);
    WriteCOM_2bit(0xD51A);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD51B);
    WriteDAT_2bit(0x0E);
    WriteCOM_2bit(0xD51C);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD51D);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD51E);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD51F);
    WriteDAT_2bit(0x7E);

    WriteCOM_2bit(0xD520);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD521);
    WriteDAT_2bit(0xA4);
    WriteCOM_2bit(0xD522);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD523);
    WriteDAT_2bit(0xD7);
    WriteCOM_2bit(0xD524);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD525);
    WriteDAT_2bit(0xFA);
    WriteCOM_2bit(0xD526);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD527);
    WriteDAT_2bit(0x28);
    WriteCOM_2bit(0xD528);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD529);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD52A);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD52B);
    WriteDAT_2bit(0x61);
    WriteCOM_2bit(0xD52C);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD52D);
    WriteDAT_2bit(0x6E);
    WriteCOM_2bit(0xD52E);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD52F);
    WriteDAT_2bit(0x78);

    WriteCOM_2bit(0xD530);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD531);
    WriteDAT_2bit(0x7E);
    WriteCOM_2bit(0xD532);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD533);
    WriteDAT_2bit(0x7F);

    WriteCOM_2bit(0xD600);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD601);
    WriteDAT_2bit(0x2D);
    WriteCOM_2bit(0xD602);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD603);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD604);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD605);
    WriteDAT_2bit(0x66);
    WriteCOM_2bit(0xD606);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD608);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD609);
    WriteDAT_2bit(0x94);
    WriteCOM_2bit(0xD60A);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD60B);
    WriteDAT_2bit(0xB4);
    WriteCOM_2bit(0xD60C);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD60D);
    WriteDAT_2bit(0xD0);
    WriteCOM_2bit(0xD60E);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xD60F);
    WriteDAT_2bit(0xFE);

    WriteCOM_2bit(0xD610);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD612);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD613);
    WriteDAT_2bit(0x59);
    WriteCOM_2bit(0xD614);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD615);
    WriteDAT_2bit(0x87);
    WriteCOM_2bit(0xD616);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xD617);
    WriteDAT_2bit(0xD1);
    WriteCOM_2bit(0xD618);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD619);
    WriteDAT_2bit(0x0C);
    WriteCOM_2bit(0xD61A);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD61B);
    WriteDAT_2bit(0x0E);
    WriteCOM_2bit(0xD61C);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD61D);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD61E);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD61F);
    WriteDAT_2bit(0x7E);

    WriteCOM_2bit(0xD620);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD621);
    WriteDAT_2bit(0xA4);
    WriteCOM_2bit(0xD622);
    WriteDAT_2bit(0x02);
    WriteCOM_2bit(0xD623);
    WriteDAT_2bit(0xD7);
    WriteCOM_2bit(0xD624);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD625);
    WriteDAT_2bit(0xFA);
    WriteCOM_2bit(0xD626);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD627);
    WriteDAT_2bit(0x28);
    WriteCOM_2bit(0xD628);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD629);
    WriteDAT_2bit(0x45);
    WriteCOM_2bit(0xD62A);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD62B);
    WriteDAT_2bit(0x61);
    WriteCOM_2bit(0xD62C);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD62D);
    WriteDAT_2bit(0x6E);
    WriteCOM_2bit(0xD62E);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD62F);
    WriteDAT_2bit(0x78);

    WriteCOM_2bit(0xD630);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD631);
    WriteDAT_2bit(0x7E);
    WriteCOM_2bit(0xD632);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xD633);
    WriteDAT_2bit(0x7F);

    WriteCOM_2bit(0xF000);
    WriteDAT_2bit(0x55);
    WriteCOM_2bit(0xF001);
    WriteDAT_2bit(0xAA);
    WriteCOM_2bit(0xF003);
    WriteDAT_2bit(0x08);
    WriteCOM_2bit(0xF004);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xB600);
    WriteDAT_2bit(0x0A);
    WriteCOM_2bit(0xB700);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xB701);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xB800);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xB801);
    WriteDAT_2bit(0x05);
    WriteCOM_2bit(0xB802);
    WriteDAT_2bit(0x05);
    WriteCOM_2bit(0xB803);
    WriteDAT_2bit(0x05);

    //WriteCOM_2bit(0xBA00 ); //WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xBC00);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xBC01);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xBC02);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xBD00);
    WriteDAT_2bit(0x02);/////////////////////////////////fps
    WriteCOM_2bit(0xBD01);
    WriteDAT_2bit(0x90);
    WriteCOM_2bit(0xBD02);
    WriteDAT_2bit(0x07);
    WriteCOM_2bit(0xBD03);
    WriteDAT_2bit(0x31);
    WriteCOM_2bit(0xBD04);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xBE00);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xBE01);
    WriteDAT_2bit(0x84);
    WriteCOM_2bit(0xBE02);
    WriteDAT_2bit(0x07);
    WriteCOM_2bit(0xBE03);
    WriteDAT_2bit(0x31);
    WriteCOM_2bit(0xBE04);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xBF00);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xBF01);
    WriteDAT_2bit(0x84);
    WriteCOM_2bit(0xBF02);
    WriteDAT_2bit(0x07);
    WriteCOM_2bit(0xBF03);
    WriteDAT_2bit(0x31);
    WriteCOM_2bit(0xBF04);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xCC00);
    WriteDAT_2bit(0x03);
    WriteCOM_2bit(0xCC01);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xCC02);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xB100);
    WriteDAT_2bit(0xF8);
    WriteCOM_2bit(0xB101);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xF000);
    WriteDAT_2bit(0x55);
    WriteCOM_2bit(0xF001);
    WriteDAT_2bit(0xAA);
    WriteCOM_2bit(0xF002);
    WriteDAT_2bit(0x52);
    WriteCOM_2bit(0xF003);
    WriteDAT_2bit(0x08);
    WriteCOM_2bit(0xF004);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xB400);
    WriteDAT_2bit(0x10);

    WriteCOM_2bit(0xF000);
    WriteDAT_2bit(0xAA);

    WriteCOM_2bit(0xFF01);
    WriteDAT_2bit(0x55);
    WriteCOM_2bit(0xFF02);
    WriteDAT_2bit(0x25);
    WriteCOM_2bit(0xFF03);
    WriteDAT_2bit(0x01);
    WriteCOM_2bit(0xFF04);
    WriteDAT_2bit(0x00);

    WriteCOM_2bit(0xF901);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0xF902);
    WriteDAT_2bit(0x0F);
    WriteCOM_2bit(0xF903);
    WriteDAT_2bit(0x23);
    WriteCOM_2bit(0xF904);
    WriteDAT_2bit(0x37);
    WriteCOM_2bit(0xF905);
    WriteDAT_2bit(0x4B);
    WriteCOM_2bit(0xF906);
    WriteDAT_2bit(0x5F);
    WriteCOM_2bit(0xF907);
    WriteDAT_2bit(0x73);
    WriteCOM_2bit(0xF908);
    WriteDAT_2bit(0x87);
    WriteCOM_2bit(0xF909);
    WriteDAT_2bit(0x98);

    WriteCOM_2bit(0xF90A);
    WriteDAT_2bit(0xAF);

    WriteCOM_2bit(0x2A00);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0x2A01);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0x2A02);
    WriteDAT_2bit(0x01);//01
    WriteCOM_2bit(0x2A03);
    WriteDAT_2bit(0xDF);//DF

    WriteCOM_2bit(0x2B00);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0x2B01);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0x2B02);
    WriteDAT_2bit(0x03);//03
    WriteCOM_2bit(0x2B03);
    WriteDAT_2bit(0x1F);//1F

    /* WriteCOM_2bit(0x3600); WriteDAT_2bit(0x00); */
    /* WriteCOM_2bit(0x4400); WriteDAT_2bit(0x04); */
    /* WriteCOM_2bit(0x4401); WriteDAT_2bit(0x1f); */

    /* WriteCOM_2bit(0xb200); WriteDAT_2bit(0xff); */
    /* WriteCOM_2bit(0xb201); WriteDAT_2bit(0x01); */
    /* WriteCOM_2bit(0xb202); WriteDAT_2bit(0x00); */
    /* WriteCOM_2bit(0xb203); WriteDAT_2bit(0xff); */
    /* WriteCOM_2bit(0xb204); WriteDAT_2bit(0xff); */

    /* {0xB2, 5, {0x7f, 0x01, 0x00, 0xff, 0xff}},//有关TE时间控制 */

    WriteCOM_2bit(0x3500);
    WriteDAT_2bit(0x00);
    WriteCOM_2bit(0x3A00);
    WriteDAT_2bit(0x55);
    //
    //
    //
    WriteCOM_2bit(0x1100);
    delay_2ms(20);
    WriteCOM_2bit(0x2900);
    delay_2ms(10);
    WriteCOM_2bit(0x2C00);
    //delay_2ms(60);
}

static void HX8357_init_code(const InitCode *code, u8 cnt)
{
    for (u8 i = 0; i < cnt; i++) {
        if (code[i].cmd == REGFLAG_DELAY) {
            lcd_delay(code[i].cnt);
        } else {
            WriteCOM_2bit(code[i].cmd);
            for (u8 j = 0; j < code[i].cnt; j++) {
                WriteDAT_2bit(code[i].dat[j]);
            }
        }
    }
}

void HX8357_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
{
    u32 len = 0;
    /* lcd_interface_non_block_wait(); */
    len = (xe + 1 - xs) * (ye + 1 - ys) * 2;
    WriteCOM_2bit(0x2A00);
    WriteDAT_2bit(xs >> 8);
    WriteCOM_2bit(0x2A01);
    WriteDAT_2bit(xs);
    WriteCOM_2bit(0x2A02);
    WriteDAT_2bit(xe >> 8);
    WriteCOM_2bit(0x2A03);
    WriteDAT_2bit(xe);

    WriteCOM_2bit(0x2B00);
    WriteDAT_2bit(ys >> 8);
    WriteCOM_2bit(0x2B01);
    WriteDAT_2bit(ys);
    WriteCOM_2bit(0x2B02);
    WriteDAT_2bit(ye >> 8);
    WriteCOM_2bit(0x2B03);
    WriteDAT_2bit(ye);

    WriteCOM_2bit(0x2C00);
    WriteDAT_DMA(img, len);
}

static void HX8357_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    /*lcd_bl_pinstate(status);*/
}

void HX8357_test(void)
{
    lcd_bl_pinstate(BL_ON);
    while (1) {
        os_time_dly(100);
        HX8357_clear_screen(BLUE);
        printf("LCD_HX8357_TSET_BLUE\n");
        os_time_dly(100);
        HX8357_clear_screen(GRED);
        printf("LCD_HX8357_TSET_GRED\n");
        os_time_dly(100);
        HX8357_clear_screen(BRRED);
        printf("LCD_HX8357_TSET_BRRED\n");
        os_time_dly(100);
        HX8357_clear_screen(YELLOW);
        printf("LCD_HX8357_TSET_YELLOW\n");
    }
}

static int HX8357_init(void)
{
    printf("LCD_HX8357 init_start\n");
    gpio_direction_output(IO_PORTC_09, 1);
    gpio_direction_output(IO_PORTH_00, 0);
    HX8357_reset();
    //HX8357_set_direction(ROTATE_DEGREE_0);
    lcd_35510_init();
    init_TE(HX8357_Fill);
    /* HX8357_test(); */
    printf("LCD_HX8357 config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_HX8357) = {
    .name              = "HX8357",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = HX8357_init,
    .SetDrawArea       = HX8357_SetRange,
    .LCD_Draw          = HX8357_draw,
    .LCD_Draw_1        = HX8357_draw_1,
    .LCD_DrawToDev     = HX8357_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full     = HX8357_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen   = HX8357_clear_screen,
    .Reset             = HX8357_reset,
    .BackLightCtrl     = HX8357_led_ctrl,
};

#endif



