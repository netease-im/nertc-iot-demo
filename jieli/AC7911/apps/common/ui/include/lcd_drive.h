#ifndef __ALL_LCD_DRIVER_H__
#define __ALL_LCD_DRIVER_H__

#include "app_config.h"
#include "asm/spi.h"
#include "typedef.h"
#include "asm/port_waked_up.h"

/* LCD 调试等级，
 * 0只打印错误，
 * 1打印错误和警告，
 * 2全部内容都调试内容打印 */
#define LCD_DEBUG_ENABLE    0

//旋转宏角度
#define ROTATE_DEGREE_0  	0
#define ROTATE_DEGREE_90  	1
#define ROTATE_DEGREE_180  	2
#define ROTATE_DEGREE_270 	3

//图像合成特殊颜色
#define FILTER_COLOR 0x000000  //图像合成过滤纯黑色
#define FILTER_COLOR_RGB565 0x0000  //图像合成过滤纯黑色
#define FILTER_COLOR_OUTER_FRAME 0xffff //外框颜色

//rgb565颜色对照表
#define   BLACK     0x0000       //  黑色
#define   NAVY      0x000F       //  深蓝色
#define   DGREEN    0x03E0       //  深绿色
#define   DCYAN     0x03EF       //  深青色
#define   MAROON    0x7800       //  深红色
#define   PURPLE    0x780F       //  紫色
#define   OLIVE     0x7BE0       //  橄榄绿
#define   LGRAY     0xC618       //  灰白色
#define   DGRAY     0x7BEF       //  深灰色
#define   BLUE      0x001F       //  蓝色
#define   GREEN     0x07E0       //  绿色
#define   CYAN      0x07FF       //  青色
#define   RED       0xF800       //  红色
#define   MAGENTA   0xF81F       //  品红
#define   YELLOW    0xFFE0       //  黄色
#define   WHITE     0xFFFF       //  白色

#if (LCD_DEBUG_ENABLE == 0)
#define lcd_d(...)
#define lcd_w(...)
#define lcd_e(fmt, ...)	printf("[LCD ERROR]: "fmt, ##__VA_ARGS__)
#elif (LCD_DEBUG_ENABLE == 1)
#define lcd_d(...)
#define lcd_w(fmt, ...)	printf("[LCD WARNING]: "fmt, ##__VA_ARGS__)
#define lcd_e(fmt, ...)	printf("[LCD ERROR]: "fmt, ##__VA_ARGS__)
#else
#define lcd_d(fmt, ...)	printf("[LCD DEBUG]: "fmt, ##__VA_ARGS__)
#define lcd_w(fmt, ...)	printf("[LCD WARNING]: "fmt, ##__VA_ARGS__)
#define lcd_e(fmt, ...)	printf("[LCD ERROR]: "fmt, ##__VA_ARGS__)
#endif

// 两毫秒延时
// extern void delay_2ms(int cnt);
// #define delay2ms(t) delay_2ms(t)


//全局显示内存 三个整帧屏显
extern u8 *ui_save_buf;
extern u8 *camera_save_buf;
extern u8 *show_buf;

//全局变量 变量的值跟着屏走 用于一套代码适配多个屏
extern u8 lcd_bk_on;
extern u8 lcd_bk_off;
extern u16 lcd_w;
extern u16 lcd_h;
extern u32 lcd_yuv420_data_size;
extern u32 lcd_rgb565_data_size;

enum LCD_COLOR {
    LCD_COLOR_RGB565,
    LCD_COLOR_RGB666,
    LCD_COLOR_RGB888,
    LCD_COLOR_MONO,
};

enum LCD_BL {
#if TCFG_LCD_ILI9488_ENABLE || TCFG_LCD_ILI9481_ENABLE
    BL_ON,
    BL_OFF,
#else
    BL_OFF,
    BL_ON,
#endif
};

//LCD接口类型
// enum LCD_IF {
enum LCD_INTERFACE {
    LCD_SPI,
    LCD_EMI,
    LCD_PAP,
    LCD_IMD,
};

struct lcd_info {
    u16 width;
    u16 height;
    u8 color_format;
    u8 col_align;
    u8 row_align;
};

//LCD板级数据
struct ui_lcd_platform_data {
    u32 rst_pin;
    u32 cs_pin;
    u32 rs_pin;
    u32 bl_pin;
    u32 te_pin;
    u32 check_pin_D6;
    u32 check_pin_D7;
    u32 touch_int_pin;
    u32 touch_reset_pin;
    char *spi_id;
    enum LCD_INTERFACE lcd_if;
};

struct lcd_device {
    char *name;			// 名称
    u16 lcd_width;
    u16 lcd_height;
    u8 color_format;
    u8 column_addr_align;
    u8 row_addr_align;
    void *lcd_priv;//LCD私有协议，比如RGB屏相关配置
    int (*LCD_Init)(void);
    void (*SetDrawArea)(u16, u16, u16, u16);
    void (*LCD_Draw)(u8 *map, u32 size);
    void (*LCD_Draw_1)(u8 *buf, u32 size, int width, int height);
    void (*LCD_Draw_2)(int x, int y, int w, int h, char *img);
    void (*LCD_DrawToDev)(u8 *map, u32 size);
    void (*LCD_Lvgl_Full)(u16 xs, u16 xe, u16 ys, u16 ye, char *img);
    void (*LCD_ClearScreen)(u32 color);
    void (*Reset)(void);
    void (*BackLightCtrl)(u8);
};

struct lcd_interface {
    int (*init)(void *);
    void (*get_screen_info)(struct lcd_info *info);
    void (*buffer_malloc)(u8 **buf, u32 *size);
    void (*buffer_free)(u8 *buf);
    void (*draw)(u8 *buf, u32 len, u8 wait);
    void (*draw_1)(u8 *buf, u32 len, u8 wait);
    void (*draw_2)(int x, int y, int w, int h, char *img);
    void (*draw_lvgl)(u16 xs, u16 xe, u16 ys, u16 ye, char *img);
    void (*set_draw_area)(u16 xs, u16 xe, u16 ys, u16 ye);
    void (*clear_screen)(u32 color);
    int (*backlight_ctrl)(u8 on);
};

#define REGISTER_LCD_DEV(lcd_dev) \
static const struct lcd_device lcd_dev SEC_USED(.lcd)

extern struct lcd_interface lcd_interface_begin[];
extern struct lcd_interface lcd_interface_end[];
extern int lcd_device_begin[];
extern int lcd_device_end[];

#define list_for_each_lcd_driver(lcd_dev) \
	for (lcd_dev = (struct lcd_device*)lcd_device_begin; lcd_dev < (struct lcd_device*)lcd_device_end; lcd_dev++)

#define REGISTER_LCD_INTERFACE(lcd) \
    static const struct lcd_interface lcd sec(.lcd_if_info) __attribute__((used))

struct lcd_interface *lcd_get_hdl();

void msleep(unsigned int ms);
#define lcd_delay(x) msleep(x)

void lcd_rst_pinstate(u8 state);
void lcd_cs_pinstate(u8 state);
void lcd_rs_pinstate(u8 state);
void lcd_bl_pinstate(u8 state);
void touch_int_pinstate(u8 state);
void touch_reset_pinstate(u8 state);
int lcd_send_byte(u8 data);
int lcd_send_map(u8 *map, u32 len);
u8 *lcd_req_buf(u32 *size);
void lcd_req_buf_update(u8 *buf, u32 *size);
int lcd_read_data(u8 *buf, u32 len);
void init_TE(void (*cb)(u8 *buf, u32 buf_size));
void user_lcd_init(void); //只初始化屏
void user_ui_lcd_init(void); //调用该函数完成ui和LCD初始化
void ui_show_frame(u8 *buf, u32 len);//该接口主要用于显示UI数据  RGB数据接口
void lcd_show_frame(u8 *buf, u32 size, int width, int height);//数据为YUV数据
void play_gif_to_lcd(char *gif_path, char play_speed);//播放GIF动图 seepd = 1 = 10ms
void get_te_fps(char *fps); //获取底层发送显示帧数
void lcd_set_draw_area(u16 xs, u16 xe, u16 ys, u16 ye);//
void lcd_interface_set_non_block(u8 block);
void lcd_interface_non_block_wait(void);
bool lcd_interface_get_non_block(void);
void lcd_check_pin_io_dir(u8 dir);
int get_check_pin_io_state(void);
void WriteCOM(u8 cmd);
void WriteDAT_8(u8 dat);
void WriteDAT_DMA(u8 *dat, u32 len);
void ReadDAT_DMA(u8 *dat, u16 len);
u8 ReadDAT_8(void);
u8 ReadDAT_16(void);
int lcd_TE_wakeup_send_data_task_init(void);//数据发送初始化
void lcd_lvgl_full(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img);

#endif

