
#ifndef __OLED_H
#define __OLED_H

#include "system/includes.h"
#include "generic/log.h"
#include "os/os_api.h"
#include "device/gpio.h"

//中景元原厂OLED，OLED数据线IO配置
#define OLED_CLK_IO	IO_PORTA_09
#define OLED_SDA_IO IO_PORTA_10

#define OLED_MODE 0
#define SIZE 8
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF
#define X_WIDTH 	128
#define Y_WIDTH 	64

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

void oled_gpio_init(void);
void oled_sclk_init(void);
void oled_sdi_init(void);
void oled_sdi_set(char val);
void oled_sclk_set(char val);

#define OLED_GPIO_INIT()	oled_gpio_init()

#define OLED_SCLK_INIT()	oled_sclk_init()
#define OLED_SCLK_Set() 	oled_sclk_set(1)
#define OLED_SCLK_Clr() 	oled_sclk_set(0)

#define OLED_SDIN_INIT()	oled_sdi_init()
#define OLED_SDIN_Clr() 	oled_sdi_set(0)
#define OLED_SDIN_Set() 	oled_sdi_set(1)

#define Delay_50ms()    os_time_dly(5) //延时函数

//OLED控制用函数
void OLED_WR_Byte(unsigned char dat, unsigned char cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x, u8 y, u8 t);
void OLED_Fill(u8 x1, u8 y1, u8 x2, u8 y2, u8 dot);
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 Char_Size);
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size);
void OLED_ShowString(u8 x, u8 y, u8 *p, u8 Char_Size);
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHineseString(u8 x, u8 y, u8 start_num, u8 num);
void OLED_Show_Chinese(u16 x, u16 y, u8 *str, u8 size);
void OLED_Show_Chinese_Just(u16 x, u16 y, u8 *str, u8 size);
void OLED_ShowCHinese(u8 x, u8 y, u8 no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]);
void fill_picture(unsigned char fill_Data);
void Picture(void);
void IIC_Start(void);
void IIC_Stop(void);
void Write_IIC_Command(unsigned char IIC_Command);
void Write_IIC_Data(unsigned char IIC_Data);
void Write_IIC_Byte(unsigned char IIC_Byte);
void IIC_Wait_Ack(void);
void OLED_Task(void);
void OLED_ShowSpectrum(u8 x, u8 y);

#endif





