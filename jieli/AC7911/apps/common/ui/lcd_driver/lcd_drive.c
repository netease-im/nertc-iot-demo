#include "app_config.h"
#include "ui_api.h"
#include "system/includes.h"
#include "device/device.h"
#include "asm/pap.h"
#include "asm/emi.h"
#include "asm/spi.h"
#include "lcd_data_driver.h"

#ifdef CONFIG_CPU_WL82
#include "asm/WL82_imd.h"
#endif

#include "lcd_drive.h"

#ifdef CONFIG_UI_ENABLE


/* 屏幕驱动的接口 */
static struct lcd_device *lcd_dev;
static const struct ui_lcd_platform_data *lcd_pdata = NULL;
static void *lcd_hdl = NULL;

u8 lcd_bk_on = 1;
u8 lcd_bk_off = 0;
u16 lcd_w = 0;
u16 lcd_h = 0;
u32 lcd_yuv420_data_size = 0;
u32 lcd_data_size = 0;

#define CONFIG_ILI9341_ID 0
#define CONFIG_ILI9481_ID 1
#define CONFIG_ILI9488_ID 2
#define CONFIG_TM9486X_ID 3

void *get_lcd_hdl(void)
{
    return lcd_hdl;
}
// io口操作
void lcd_rst_pinstate(u8 state)
{
    if (lcd_pdata->rst_pin != (u32) - 1) {
        gpio_direction_output(lcd_pdata->rst_pin, state);
    }
}

void lcd_cs_pinstate(u8 state)
{
    if (lcd_pdata->cs_pin != (u32) - 1) {
        gpio_direction_output(lcd_pdata->cs_pin, state);
    }
}
void lcd_check_pin_io_dir(u8 dir)
{
    if ((lcd_pdata->check_pin_D6 != (u32) - 1) && (lcd_pdata->check_pin_D7 != (u32) - 1)) {
        if (dir) { //设置GPIO为输入模式
            gpio_set_direction(lcd_pdata->check_pin_D6, 1);
            gpio_set_direction(lcd_pdata->check_pin_D7, 1);
        } else { //恢复GPIO为输出模式
            gpio_set_direction(lcd_pdata->check_pin_D6, 0);
            gpio_set_direction(lcd_pdata->check_pin_D7, 0);
        }
    } else {
        printf("\n [ERROR] %s -[you_need_set_check_io] %d\n", __FUNCTION__, __LINE__);
    }
}
int get_check_pin_io_state(void)
{
    //先设置输出高
    lcd_check_pin_io_dir(0);
    gpio_set_direction(lcd_pdata->check_pin_D6, 1);
    gpio_set_direction(lcd_pdata->check_pin_D7, 1);

    //再去读一下io电平
    lcd_check_pin_io_dir(1);
    if ((lcd_pdata->check_pin_D6 != (u32) - 1) && (lcd_pdata->check_pin_D7 != (u32) - 1)) {
        //读到两个io为高
        if (gpio_read(lcd_pdata->check_pin_D6) && gpio_read(lcd_pdata->check_pin_D7)) {
            lcd_check_pin_io_dir(0);
            return CONFIG_ILI9488_ID;
            //读到两个为低
        } else if (!gpio_read(lcd_pdata->check_pin_D6) && !gpio_read(lcd_pdata->check_pin_D7)) {
            lcd_check_pin_io_dir(0);
            return CONFIG_ILI9481_ID;
        } else {
            lcd_check_pin_io_dir(0);
            return CONFIG_ILI9481_ID;
        }
    } else {
        printf("\n [ERROR] %s -[you_need_set_check_io] %d\n", __FUNCTION__, __LINE__);
        lcd_check_pin_io_dir(0);
        return CONFIG_ILI9481_ID;
    }
}
void lcd_rs_pinstate(u8 state)
{
    if (lcd_pdata->rs_pin != (u32) - 1) {
        gpio_direction_output(lcd_pdata->rs_pin, state);
    }
}

void lcd_bl_pinstate(u8 state)
{
    if (lcd_pdata->bl_pin != (u32) - 1) {
        gpio_direction_output(lcd_pdata->bl_pin, state);
    }
}

void touch_int_pinstate(u8 state)
{
    if (lcd_pdata->touch_int_pin != (u32) - 1) {
        gpio_direction_output(lcd_pdata->touch_int_pin, state);
    }
}

void touch_reset_pinstate(u8 state)
{
    if (lcd_pdata->touch_reset_pin != (u32) - 1) {
        gpio_direction_output(lcd_pdata->touch_reset_pin, state);
    }
}

int lcd_send_byte(u8 data)
{
    if (lcd_hdl) {
        return dev_write(lcd_hdl, &data, 1);
    }

    return -1;
}

int lcd_send_map(u8 *map, u32 len)
{
    if (lcd_hdl) {
        return dev_write(lcd_hdl, map, len);
    }

    return -1;
}

int lcd_read_data(u8 *buf, u32 len)
{
    if (lcd_hdl) {
        return dev_read(lcd_hdl, buf, len);
    }

    return -1;
}
u8 *lcd_req_buf(u32 *size)
{
    if (lcd_pdata && lcd_pdata->lcd_if == LCD_IMD && lcd_hdl) {
#ifdef CONFIG_CPU_WL82
        u8 *buf = NULL;
        u32 buf_size = 0;
        dev_ioctl(lcd_hdl, IOCTL_IMD_GET_FRAME_BUFF, (u32)&buf);
        dev_ioctl(lcd_hdl, IOCTL_IMD_GET_FRAME_BUFF_SIZE, (u32)&buf_size);
        if (buf && buf_size) {
            *size = buf_size;
            return buf;
        }
#endif
    }
    *size = 0;
    return NULL;
}
void lcd_req_buf_update(u8 *buf, u32 *size)
{
    if (lcd_pdata && lcd_pdata->lcd_if == LCD_IMD && lcd_hdl) {
#ifdef CONFIG_CPU_WL82
        dev_ioctl(lcd_hdl, IOCTL_IMD_UPDATE_BUFF, (u32)buf);
#endif
    }
}

void WriteCOM(u8 cmd)
{
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(0);//cmd
    lcd_send_byte(cmd);
    lcd_cs_pinstate(1);
}

void WriteDAT_8(u8 dat)
{
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    lcd_send_byte(dat);
    lcd_cs_pinstate(1);
}

void WriteDAT_DMA(u8 *dat, u32 len)
{
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    lcd_send_map(dat, len);
    lcd_cs_pinstate(1);
}

void ReadDAT_DMA(u8 *dat, u16 len)
{
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    lcd_read_data(dat, len);
    lcd_cs_pinstate(1);
}

u8 ReadDAT_8(void)
{
    u8 dat = 0;
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    lcd_read_data(&dat, 1);
    lcd_cs_pinstate(1);
    return dat;
}

u8 ReadDAT_16(void)
{
    u16 dat = 0;
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    lcd_read_data((u8 *)&dat, 2);
    lcd_cs_pinstate(1);
    return dat;
}

extern void lcd_send_ok(void);
void get_lcd_send_ok()
{
    lcd_send_ok();
}

static int lcd_interface_init(void)
{
    void *priv = lcd_dev->lcd_priv;
    if (lcd_pdata->lcd_if == LCD_SPI) {
        lcd_hdl = dev_open(lcd_pdata->spi_id, NULL);
        dev_ioctl(lcd_hdl, IOCTL_SPI_NON_BLOCK, 1);//设置为非阻塞发送数据包
    } else if (lcd_pdata->lcd_if == LCD_IMD) {
        lcd_hdl = dev_open("imd", priv);
        if (lcd_hdl) {
            dev_ioctl(lcd_hdl, IOCTL_IMD_SET_CB_FUNC, get_lcd_send_ok);
        }
    } else if (lcd_pdata->lcd_if == LCD_PAP) {
        lcd_hdl = dev_open("pap", NULL);
    } else if (lcd_pdata->lcd_if == LCD_EMI) {
        lcd_hdl = dev_open("emi", NULL);
        if (lcd_hdl) {
            dev_ioctl(lcd_hdl, EMI_SET_ISR_CB, get_lcd_send_ok);
            dev_ioctl(lcd_hdl, EMI_USE_SEND_SEM, 1);
            dev_ioctl(lcd_hdl, IOCTL_EMI_WRITE_NON_BLOCK, 1);
        }
    }

    if (lcd_hdl == NULL) {
        lcd_e("open lcd interface failed.\n");
        return -1;
    }

    return 0;
}


bool lcd_interface_get_non_block(void)
{
    bool non_block;
    void *priv = lcd_dev->lcd_priv;
    if (lcd_pdata->lcd_if == LCD_SPI) {
        dev_ioctl(lcd_hdl, IOCTL_SPI_GET_NON_BLOCK, &non_block);
    } else if (lcd_pdata->lcd_if == LCD_EMI) {
        dev_ioctl(lcd_hdl, IOCTL_EMI_GET_NON_BLOCK, &non_block);
    }
    return non_block;
}

void lcd_interface_set_non_block(u8 block)
{
    void *priv = lcd_dev->lcd_priv;
    if (lcd_pdata->lcd_if == LCD_SPI) {
        dev_ioctl(lcd_hdl, IOCTL_SPI_NON_BLOCK, block);
    } else if (lcd_pdata->lcd_if == LCD_EMI) {
        dev_ioctl(lcd_hdl, IOCTL_EMI_WRITE_NON_BLOCK, block);
    }
}
void lcd_interface_non_block_wait(void)
{
    void *priv = lcd_dev->lcd_priv;
    if (lcd_pdata->lcd_if == LCD_SPI) {
        dev_ioctl(lcd_hdl, IOCTL_SPI_WRITE_NON_BLOCK_FLUSH, 0);//等待上一次发包完成
    } else if (lcd_pdata->lcd_if == LCD_EMI) {
        dev_ioctl(lcd_hdl, IOCTL_EMI_WRITE_NON_BLOCK_FLUSH, 0);//等待上一次发包完成
    }
}


static int lcd_init(void *p)
{
    printf("\n [lcd_init] %s -[init start] %d\n", __FUNCTION__, __LINE__);

    u8 lcd_choice = CONFIG_ILI9481_ID;

    if (!lcd_hdl) {
        struct ui_devices_cfg *cfg = (struct ui_devices_cfg *)p;
        u8 find_lcd = 0;
        lcd_pdata = (const struct ui_lcd_platform_data *)cfg->private_data;
        ASSERT(lcd_pdata, "Error! lcd io not config");
        //如果有触摸通过触摸来区分屏 没有触摸通过check io来识别
        //原理是在D6 D7接上下拉 读io 来实现
#if TCFG_LCD_ILI9341_ENABLE
        int check_gt911(void);//通过触摸来识别屏
        if (check_gt911() == 0) {
            printf("[msg]>>>>>>>>gt911_ok");
            lcd_choice = CONFIG_ILI9341_ID;
            lcd_w = 240;
            lcd_h = 320;
            lcd_bk_on = 1;
            lcd_bk_off = 0;
        }
#endif

#if TCFG_LCD_ILI9481_ENABLE || TCFG_LCD_ILI9488_ENABLE || TCFG_LCD_ILI9341_ENABLE
        if (lcd_choice != CONFIG_ILI9341_ID) {
            lcd_choice = get_check_pin_io_state();//通过io检查来识别屏
            lcd_w = 320;
            lcd_h = 480;
            lcd_bk_on = 0;
            lcd_bk_off = 1;
            if (lcd_choice == CONFIG_ILI9481_ID) {
                printf("\n [lcd_drive] %s -[find_check_ili9481] %d\n", __FUNCTION__, __LINE__);
            } else if (lcd_choice == CONFIG_ILI9488_ID) {
                printf("\n [lcd_drive] %s -[find_check_ili9488] %d\n", __FUNCTION__, __LINE__);
            }
        }
#endif

#if TCFG_LCD_TM9486X_ENABLE//客户屏放最后
        lcd_w = 480;
        lcd_h = 800;
        lcd_bk_on = 1;
        lcd_bk_off = 0;
        lcd_choice = CONFIG_TM9486X_ID;
        printf("\n [lcd_drive] %s -[find_check_TM9486X] %d\n", __FUNCTION__, __LINE__);
#endif

        //初始化全局显示内存配置
        lcd_yuv420_data_size = lcd_w * lcd_h * 3 / 2;
#if TCFG_LCD_480x272_8BITS //rgb888
        lcd_w = 480;
        lcd_h = 272;
        lcd_bk_on = 1;
        lcd_bk_off = 0;
        lcd_data_size = lcd_w * lcd_h * 3;
#else  //rgb565
        lcd_data_size = lcd_w * lcd_h * 2;
#endif

        //配置lcd_data_driver一些参数
        set_lcd_data_driver_lcd_h_w(lcd_w, lcd_h);
        set_lcd_data_size(lcd_data_size);
        set_lcd_yuv420_data_size(lcd_yuv420_data_size);
        set_lcd_bk_on(lcd_bk_on);
        set_lcd_te_pgio(lcd_pdata->te_pin);

        //申请全局lcd显示buf
        extern void malloc_lcd_buf(void);
        malloc_lcd_buf();


        //遍历注册的设备驱动跟ID做比较判断出使用那个屏
        list_for_each_lcd_driver(lcd_dev) {
            if (lcd_dev && lcd_dev->LCD_Init) {
#if TCFG_LCD_ILI9481_ENABLE || TCFG_LCD_ILI9488_ENABLE || TCFG_LCD_ILI9341_ENABLE || TCFG_LCD_TM9486X_ENABLE
                if (lcd_dev->name == "ili9341" && lcd_choice == CONFIG_ILI9341_ID) {
                    printf("[msg]>>>>>>>>find_lcd_ili9341");
                    find_lcd = true;
                    break;
                }
                if (lcd_dev->name == "ili9481" && lcd_choice == CONFIG_ILI9481_ID) {
                    printf("[msg]>>>>>>>>find_lcd_ili9481");
                    find_lcd = true;
                    break;
                }
                if (lcd_dev->name == "ili9488" && lcd_choice == CONFIG_ILI9488_ID) {
                    printf("[msg]>>>>>>>>find_lcd_ili9488");
                    find_lcd = true;
                    break;
                }
                if (lcd_dev->name == "HX8357" && lcd_choice == CONFIG_TM9486X_ID) {
                    printf("[msg]>>>>>>>>find_lcd_TM9486X");
                    find_lcd = true;
                    break;
                }
#else
                printf("LCD find %s\n", lcd_dev->name);
                find_lcd = true;
                break;
#endif
            }

        }
        if (!find_lcd) {
            printf("err no lcd_dev\n");
            return -1;
        }
        if (lcd_interface_init() != 0) {
            printf("err unknow lcd port \n\n\n");
            return -1;
        }
        if (lcd_dev && lcd_dev->LCD_Init) {
            lcd_dev->LCD_Init();
            //图像合成线程初始化
            picture_compose_task_init();
            return 0;
        }
    }
    return 0;
}

void user_lcd_init(void)
{
    if (!lcd_hdl) {
        extern const struct ui_devices_cfg ui_cfg_data;
        lcd_init(&ui_cfg_data);//ui_cfg_data参数可以在版籍配置
    }
}

void user_ui_lcd_init(void)
{
#if TCFG_USE_SD_ADD_UI_FILE
    int storage_device_ready(void);
    while (!storage_device_ready()) {
        os_time_dly(1);
    }
#endif
    extern const struct ui_devices_cfg ui_cfg_data;
    int lcd_ui_init(void *arg);
    lcd_ui_init(&ui_cfg_data);
}
static void lcd_get_screen_info(struct lcd_info *info)
{
    if (!lcd_dev) {
        return ;
    }
    info->width        = lcd_dev->lcd_width;
    info->height       = lcd_dev->lcd_height;
    info->color_format = lcd_dev->color_format;
    info->col_align    = lcd_dev->column_addr_align;
    info->row_align    = lcd_dev->row_addr_align;
}

static void lcd_buffer_malloc(u8 **buf, u32 *size)
{
    if (!lcd_dev) {
        return ;
    }
    *buf = malloc(lcd_dev->lcd_width * lcd_dev->lcd_height * 2);
    if (NULL == *buf) {
        *size = 0;
        return;
    }
    *size = lcd_dev->lcd_width * lcd_dev->lcd_height * 2;
}

static void lcd_buffer_free(u8 *buf)
{
    free(buf);
}

static void ui_draw(u8 *buf, u32 len, u8 wait)
{
    if (!lcd_dev) {
        return ;
    }
    if (lcd_dev->LCD_Draw) {
        lcd_dev->LCD_Draw(buf, len);
    }
}
static void lcd_draw(u8 *buf, u32 size, int width, int height)
{
    if (!lcd_dev) {
        return ;
    }
    if (lcd_dev->LCD_Draw_1) {
        lcd_dev->LCD_Draw_1(buf, size, width, height);
    }
}

static void user_draw(int x, int y, int w, int h, char *img)
{
    if (!lcd_dev) {
        return ;
    }
    if (lcd_dev->LCD_Draw_2) {
        lcd_dev->LCD_Draw_2(x, y, w, h, img);
    }
}

static void lcd_draw_to_dev(u8 *buf, u32 len)
{
    if (!lcd_dev) {
        return ;
    }
    if (lcd_dev->LCD_DrawToDev) {
        lcd_dev->LCD_DrawToDev(buf, len);
    }
}
void lcd_set_draw_area(u16 xs, u16 xe, u16 ys, u16 ye)
{
    if (!lcd_dev) {
        return ;
    }
    if (lcd_dev->SetDrawArea) {
        lcd_dev->SetDrawArea(xs, xe, ys, ye);
    }
}

static void lcd_clear_screen(u16 color)
{
    if (!lcd_dev) {
        return ;
    }
    if (lcd_dev->LCD_ClearScreen) {
        lcd_dev->LCD_ClearScreen(color);
    }
}

static int lcd_backlight_power(u8 on)
{
    static u8 first_power_on = true;
    if (first_power_on) {
        lcd_clear_screen(0x0000);
        first_power_on = false;
    }

    if (!lcd_dev) {
        return -EINVAL;
    }
    if (lcd_dev->BackLightCtrl) {
        lcd_dev->BackLightCtrl(on);
    }

    return 0;
}

int lcd_backlight_ctrl(u8 on)
{
    return 0;
}


int lcd_sleep_control(u8 enter)
{
    return 0;
}

int lcd_backlight_status()
{
    return 1;
}

void lcd_get_width_height(int *width, int *height)
{
    if (!lcd_dev) {
        *width = 0;
        *height = 0;
        return ;
    }
    *width = lcd_dev->lcd_width;
    *height = lcd_dev->lcd_height;
}

struct lcd_interface *lcd_get_hdl(void)
{
    struct lcd_interface *p;

    ASSERT(lcd_interface_begin != lcd_interface_end, "don't find lcd interface!");
    for (p = lcd_interface_begin; p < lcd_interface_end; p++) {
        return p;
    }
    return NULL;
}
int lcd_get_color_format_rgb24(void)
{
    if (!lcd_dev) {
        return 0;
    }
    return (lcd_dev->color_format == LCD_COLOR_RGB888);
}

void lcd_show_frame_to_dev(u8 *buf, u32 len)//该接口显示数据直接推送数据到LCD设备接口，不分数据格式，慎用！
{
    lcd_draw_to_dev(buf, len);
}
void ui_show_frame(u8 *buf, u32 len)//该接口主要用于显示UI数据  传入数据RGB
{
    ui_draw(buf, len, 0);
}

void lcd_show_frame(u8 *buf, u32 size, int width, int height)//该接口主要用于用户显示数据 //列如摄像头数据 视频数据 YUV
{
    lcd_draw(buf, size, width, height);
}

void user_show_frame(int x, int y, int w, int h, u8 *img)//该接口主要用于修改摄像头层数据 传入RGB数据
{
    user_draw(x, y, w, h, img);
}

void lcd_lvgl_full(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
{
    lcd_dev->LCD_Lvgl_Full(xs, xe, ys, ye, img);
}

REGISTER_LCD_INTERFACE(lcd) = {
    .init = lcd_init,
    .get_screen_info = lcd_get_screen_info,
    .buffer_malloc = lcd_buffer_malloc,
    .buffer_free = lcd_buffer_free,
    .draw = ui_draw,
    .draw_1 = lcd_draw,
    .draw_2 = user_draw,
    .draw_lvgl = lcd_lvgl_full,
    .set_draw_area = lcd_set_draw_area,
    .backlight_ctrl = lcd_backlight_power,
};

#endif



