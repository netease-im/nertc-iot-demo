#ifndef _LCD_DATA_DRIVER_H_
#define _LCD_DATA_DRIVER_H_

#if 0
#define log_info(x, ...)    printf("\n[lcd_deta_driver]>>>>>>>>>>>>>>>>>>>###" x " \n", ## __VA_ARGS__)
#else
#define log_info(...)
#endif

#define FILTER_COLOR 0x000000  //图像合成过滤纯黑色
#define FILTER_COLOR_RGB565 0x0000  //图像合成过滤纯黑色
#define FILTER_COLOR_OUTER_FRAME 0xffff //外框颜色

enum lcd_turn {
    LCD_TURN_90 = 0,
    LCD_TURN_270,
    LCD_TURN_0,
    LCD_TURN_180,
};

enum lcd_show_mode {
    UI,
    CAMERA,
    UI_CAMERA,
    UI_USER,
    UI_QR,
    UI_RGB,
};

struct lcd_task_info {
    OS_SEM yuv_sem;
    OS_SEM compose_sem;
    OS_SEM wait_yuv_use_ok_sem;
    u8 yuv_data_use_flag;
    u8 data_ok_flag;
    u8 send_data_ok_flag;
    u8 wait_te_ready;
    u8 *lcd_data;
    u8 *in_lcd_buf;
    u8 *out_lcd_buf;
    u8 *r_lbuf;
};

struct lcd_compose_info {
    OS_SEM te_ready_sem;
    u8 data_mode;
    u8 turn_mode;
    u8 piece;
    u8 init_value;
    u8 compose_mode;
    u8 compose_status;
    u8 aim_filter_color_l;
    u8 aim_filter_color_h;
    u8 lcd_bk_on;
    u8 lcd_te_io;
    u8 camera_data_mode;
    u16 user1_y_len;
    u16 user2_y;
    u16 user2_y_len;
    u16 start_x;//起始坐标
    u16 start_y;
    u16 aim_compose_w;//合成长宽
    u16 aim_compose_h;
    u16 ui_xstart;
    u16 ui_xend;
    u16 ui_ystart;
    u16 ui_yend;
    u16 lcd_w;
    u16 lcd_h;
    u32 data_size;
    u32 lcd_data_size;
    u32 lcd_yuv420_data_size;
    u8 *ui_save_buf;
    u8 *camera_save_buf;
    u8 *show_buf;
    u8 *out_lcd_buf;
    u8 *binarization;
    u8 *lbuf;
    u8 *w_lbuf;
    u8 task_time;
    struct lbuff_head *lbuf_handle;
};

void set_lcd_data_driver_lcd_h_w(u16 lcd_w, u16 lcd_h);
void set_lcd_data_size(u32 size);
void set_lcd_yuv420_data_size(u32 size);
void set_lcd_bk_on(u8 on);
void set_lcd_te_pgio(u32 gpio);
//获取全局UI数据内存指针
u8 *get_ui_buf(void);
//获取全局camera数据内存指针
u8 *get_camera_buf(void);
//获取全局推屏数据内存指针
u8 *get_show_buf(void);
void get_data_use_flag(void);
void get_data_use_flag1(void);
void camera_send_data_ready(u8 *buf, u32 size, int width, int height);
void ui_send_data_ready(u8 *data_buf, u32 data_size);
void yuv_picture_compose(u8 *in, u8 *out, u16 w, u16 h);
void set_photo_compose_mode(u8 mode, u8 choice_show_piece);
void set_init_value(u8 set_init_value);//设置mode_7中的ui数据读取
void set_compose_mode(u8 mode, int mode1_y_len, int mode2_y, int mode2_y_len);
void set_compose_mode3(int x, int y, int w, int h);
void set_lcd_show_data_mode(enum lcd_show_mode choice);
u8 get_lcd_show_deta_mode(void);
void set_lcd_ui_x_y(u16 xstart, u16 xend, u16 ystart, u16 yend);
void set_turn_mode(enum lcd_turn mode);
u8 get_lcd_turn_mode(void);
void init_TE(void (*cb)(u8 *buf, u32 buf_size));
void picture_compose_task_init(void);
void lcd_data_free_camera_lbuf(void);

extern int get_jpeg2yuv_yuv_type(void);

#endif
