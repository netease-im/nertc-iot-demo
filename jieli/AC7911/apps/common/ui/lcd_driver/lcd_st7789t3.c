#if 0
//说明该文件为测试文件 为测试SPI双线SPI 9bit spi屏目前WL82不支持9bit spi
#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "app_config.h"
#include "lcd_config.h"
#include "gpio.h"
#include "device/device.h"//u8
#include "storage_device.h"//SD
#include "server/video_dec_server.h"//fopen
#include "lcd_drive.h"
#include "sys_common.h"
#include "yuv_soft_scalling.h"
#include "asm/jpeg_codec.h"
#include "get_yuv_data.h"


#define COL             240
#define ROW             320

#define LCD_SDA  		IO_PORTA_04
#define LCD_SCL  		IO_PORTA_03
#define LCD_RS   		IO_PORTA_02
#define LCD_REST 		IO_PORTA_05
#define LCD_CS 		    IO_PORTA_06

#define SDA(x)      	gpio_direction_output(LCD_SDA , x)
#define SCL(x)      	gpio_direction_output(LCD_SCL , x)
#define RS(x)       	gpio_direction_output(LCD_RS  , x)
#define REST(x)     	gpio_direction_output(LCD_REST, x)
#define CS(x)       	gpio_direction_output(LCD_CS  , x)

#define SDA_R()   	    gpio_read(LCD_SDA)
#define SET_SDA_IN() 	gpio_direction_input(LCD_SDA)
#define SET_SDA_OUT() 	gpio_direction_output(LCD_SDA, 0)

void msleep(unsigned int ms);
#define Delay(x) msleep(x)

//----------------------------------------------------------------------
static void ST7789T3_io_init()
{
    SDA(0);
    SCL(0);

    CS(1);
    RS(1);

    REST(1);
    Delay(50);
    REST(0);
    Delay(20);
    REST(1);
    Delay(120);
}
static u32 spi_read_id(void)
{
    u16 bit_ctr;

    u8 data = 0x04;//id cmd
    u32 get_id;
    SCL(0);
    CS(0);
    RS(0);

    SDA(0);
    SCL(1);

    for (u8 i = 0; i < 8; i++) {
        SCL(0);
        if ((data & 0x80) == 0x80) {
            SDA(1);
        } else {
            SDA(0);
        }
        SCL(1);
        data = (data << 1);
    }
    /* SCL(0);//dummy clock_t cycle */

    SET_SDA_IN();
    for (u8 i = 0; i < 24; i++) {
        SCL(0);
        get_id |=  SDA_R();
        get_id <<= 1;
        SCL(1);
    }
    SCL(0);

    RS(1);
    CS(1);
    SET_SDA_OUT();
    return get_id;
}

void SendDataSPI(unsigned char dat)
{
    unsigned char i;

    for (i = 0; i < 8; i++) {
        if ((dat & 0x80) != 0) {
            gpio_direction_output(LCD_SDA, 1);
        } else {
            gpio_direction_output(LCD_SDA, 0);
        }

        dat <<= 1;

        gpio_direction_output(LCD_SCL, 0);
        gpio_direction_output(LCD_SCL, 1);
    }
}

void WriteComm(unsigned char i)
{
    char k;
    gpio_direction_output(LCD_CS, 0);
    gpio_direction_output(LCD_SDA, 0);
    gpio_direction_output(LCD_SCL, 0);
    gpio_direction_output(LCD_SCL, 1);

    for (k = 8; k > 0; k--) {
        if ((i & 0x80) != 0) {
            gpio_direction_output(LCD_SDA, 1);
        } else {
            gpio_direction_output(LCD_SDA, 0);
        }

        i <<= 1;

        gpio_direction_output(LCD_SCL, 0);
        gpio_direction_output(LCD_SCL, 1);
    }

    gpio_direction_output(LCD_CS, 1);
}

void WriteData(unsigned char i)
{
    char k;
    gpio_direction_output(LCD_CS, 0);
    gpio_direction_output(LCD_SDA, 1);
    gpio_direction_output(LCD_SCL, 0);
    gpio_direction_output(LCD_SCL, 1);

    for (k = 8; k > 0; k--) {
        if ((i & 0x80) != 0) {
            gpio_direction_output(LCD_SDA, 1);
        } else {
            gpio_direction_output(LCD_SDA, 0);
        }

        i <<= 1;

        gpio_direction_output(LCD_SCL, 0);
        gpio_direction_output(LCD_SCL, 1);
    }
    gpio_direction_output(LCD_CS, 1);
}


void write_color(unsigned int color)
{
    unsigned char i, DATA_H, DATA_L;
    DATA_H = color >> 8;
    DATA_L = color;
    gpio_direction_output(LCD_CS, 0);
    gpio_direction_output(LCD_SDA, 1);
    gpio_direction_output(LCD_RS, 1);
    gpio_direction_output(LCD_SCL, 0);
    gpio_direction_output(LCD_SCL, 1);

    for (i = 8; i > 0; i--) {
        if ((DATA_H & 0x80) != 0) {
            gpio_direction_output(LCD_SDA, 1);
        } else {
            gpio_direction_output(LCD_SDA, 0);
        }
        DATA_H <<= 1;

        if ((DATA_L & 0x80) != 0) {
            gpio_direction_output(LCD_RS, 1);
        } else {
            gpio_direction_output(LCD_RS, 0);
        }
        DATA_L <<= 1;

        gpio_direction_output(LCD_SCL, 0);
        gpio_direction_output(LCD_SCL, 1);
    }
    gpio_direction_output(LCD_CS, 1);
}

void BlockWrite(unsigned int Xstart, unsigned int Xend, unsigned int Ystart, unsigned int Yend)
{
    WriteComm(0x2A);
    WriteData(Xstart >> 8);
    WriteData(Xstart);
    WriteData(Xend >> 8);
    WriteData(Xend);

    WriteComm(0x2B);
    WriteData(Ystart >> 8);
    WriteData(Ystart);
    WriteData(Yend >> 8);
    WriteData(Yend);

    WriteComm(0x2c);
}


void DispColor(unsigned int color)
{
    unsigned int i;
    unsigned char j, k, DATA_H, DATA_L;
    /* CLKSEL = 0x03; */

    gpio_direction_output(LCD_CS, 0);

    for (i = 0; i < ROW; i++) {
        for (j = 0; j < COL; j++) {
            DATA_H = color >> 8;
            DATA_L = color;
            gpio_direction_output(LCD_SDA, 1);
            gpio_direction_output(LCD_RS, 1);
            gpio_direction_output(LCD_SCL, 0);
            gpio_direction_output(LCD_SCL, 1);

            for (k = 8; k > 0; k--) {
                if ((DATA_H & 0x80) != 0) {
                    gpio_direction_output(LCD_SDA, 1);
                } else {
                    gpio_direction_output(LCD_SDA, 0);
                }
                DATA_H <<= 1;

                if ((DATA_L & 0x80) != 0) {
                    gpio_direction_output(LCD_RS, 1);
                } else {
                    gpio_direction_output(LCD_RS, 0);
                }
                DATA_L <<= 1;

                gpio_direction_output(LCD_SCL, 0);
                gpio_direction_output(LCD_SCL, 1);
            }
        }

    }

    gpio_direction_output(LCD_CS, 1);
}


void LCD_Init(void)
{

    gpio_direction_output(LCD_REST, 1);
    Delay(80);
    gpio_direction_output(LCD_REST, 0);
    Delay(80);
    gpio_direction_output(LCD_REST, 1);
    Delay(480);

    WriteComm(0x11);
    Delay(120);

    /* ///RGB??
    WriteComm(0xB0);
    WriteData(0x11);
    WriteData(0xF0);

    WriteComm(0xB1);
    WriteData(0x42);    //4E
    WriteData(0x10);     //18
    WriteData(0x12);     //1A
    */

    WriteComm(0x36);
    WriteData(0x00);   //RGB


    WriteComm(0x3A);
    WriteData(0x55);   //16BIT

    WriteComm(0xB2);
    WriteData(0x05);
    WriteData(0x05);
    WriteData(0x00);
    WriteData(0x33);
    WriteData(0x33);

    WriteComm(0xB7);
    WriteData(0x35);

    WriteComm(0xBB);
    WriteData(0x20);

    WriteComm(0xC0);
    WriteData(0x2C);

    WriteComm(0xC2);
    WriteData(0x01);

    WriteComm(0xC3);
    WriteData(0x0B);

    WriteComm(0xE7);     //2??
    WriteData(0x10);

    WriteComm(0xC4);
    WriteData(0x20);

    WriteComm(0xC6);
    WriteData(0x0F);   //60HZ dot inversion

    WriteComm(0xD0);
    WriteData(0xA4);
    WriteData(0xA1);

    WriteComm(0xD6);
    WriteData(0xA1);

    WriteComm(0xE0);
    WriteData(0xD0);
    WriteData(0x04);
    WriteData(0x08);
    WriteData(0x0A);
    WriteData(0x09);
    WriteData(0x05);
    WriteData(0x2D);
    WriteData(0x43);
    WriteData(0x49);
    WriteData(0x09);
    WriteData(0x16);
    WriteData(0x15);
    WriteData(0x26);
    WriteData(0x2B);

    WriteComm(0xE1);
    WriteData(0xD0);
    WriteData(0x03);
    WriteData(0x09);
    WriteData(0x0A);
    WriteData(0x0A);
    WriteData(0x06);
    WriteData(0x2E);
    WriteData(0x44);
    WriteData(0x40);
    WriteData(0x3A);
    WriteData(0x15);
    WriteData(0x15);
    WriteData(0x26);
    WriteData(0x2A);

    WriteComm(0x21);

    WriteComm(0x29);
    WriteComm(0x2C);
    Delay(100);
}

static void spi_task(void *priv)
{
    os_time_dly(100);

    ST7789T3_io_init();
    u32 get_cld_id;
    get_cld_id = spi_read_id();
    printf("[msg]>>>>>>>>>>get_cld_id = 0x%x\n", get_cld_id);

    LCD_Init();

    while (1) {
        DispColor(WHITE);
        os_time_dly(100);
        DispColor(BLACK);
        os_time_dly(100);
        DispColor(BLUE);
        os_time_dly(100);
        DispColor(BRED);
        os_time_dly(100);
        DispColor(GRED);
        os_time_dly(100);
        DispColor(GBLUE);
        os_time_dly(100);
        DispColor(RED);
        os_time_dly(100);
        DispColor(MAGENTA);
        os_time_dly(100);
        DispColor(GREEN);
        os_time_dly(100);
        DispColor(CYAN);
        os_time_dly(100);
        DispColor(YELLOW);
        os_time_dly(100);
        DispColor(BROWN);
        os_time_dly(100);
        DispColor(BRRED);
        os_time_dly(100);
        DispColor(GRAY);
        os_time_dly(100);
        DispColor(RED);
        os_time_dly(100);
        printf("\n [ERROR] %s -[yuyu] %d\n", __FUNCTION__, __LINE__);
    }
}
static int spi_task_init(void)
{
    puts("spi_task_init \n\n");
    return thread_fork("spi_task", 11, 1024, 32, 0, spi_task, NULL);
}
late_initcall(spi_task_init);
#endif
