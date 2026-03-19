/*===================================================================
 *  该文件实现了图像合成双线程推屏加速
 *  上层通过lcd_show_frame()到达该文件的camera_send_data_ready()
 *  ui数据会自动对接到接口ui_send_data_ready()
 *  所有数据都通过这个函数处理picture_compose_main
 *  最终通过 cb_TE_send_lcd_data()发送数据到屏
 *  屏幕默认为竖屏固定 ui也默认为竖屏 ui为横屏通过宏HORIZONTAL_SCREEN实现
 *===================================================================*/
#include "system/includes.h"
#include "asm/port_waked_up.h"
#include "yuv_to_rgb.h"
#include "yuv_soft_scalling.h"
#include "lcd_data_driver.h"
#include "app_config.h"

#if 0
#define log_info(x, ...)    printf("\n[lcd_deta_driver]>>>>>>>>>>>>>>>>>>>###" x " \n", ## __VA_ARGS__)

#else
#define log_info(...)

#endif
static struct lcd_task_info task[2] = {0};
static struct lcd_compose_info lcd_compose_info_t = {0};
#define __this lcd_compose_info_t

static void Calculation_frame(void)
{
    static u32 tstart = 0, tdiff = 0;
    static u8 fps_cnt = 0;
    fps_cnt++ ;
    if (!tstart) {
        tstart = timer_get_ms();
    } else {
        tdiff = timer_get_ms() - tstart;
        if (tdiff >= 1000) {
            printf("\n [te_MSG]fps_count = %d\n", fps_cnt *  1000 / tdiff);
            tstart = 0;
            fps_cnt = 0;
        }
    }
}
static void Calculation_frame_no_te(void)
{
    static u32 tstart = 0, tdiff = 0;
    static u8 fps_cnt = 0;
    fps_cnt++ ;
    if (!tstart) {
        tstart = timer_get_ms();
    } else {
        tdiff = timer_get_ms() - tstart;
        if (tdiff >= 1000) {
            printf("\n [NO_te_MSG]fps_count = %d\n", fps_cnt *  1000 / tdiff);
            tstart = 0;
            fps_cnt = 0;
        }
    }
}
void set_lcd_data_driver_lcd_h_w(u16 lcd_w, u16 lcd_h)
{
    __this.lcd_w = lcd_w;
    __this.lcd_h = lcd_h;
}

void set_lcd_data_size(u32 size)
{
    __this.lcd_data_size = size;
}

void set_lcd_yuv420_data_size(u32 size)
{
    __this.lcd_yuv420_data_size = size;
}
void set_lcd_bk_on(u8 on)
{
    __this.lcd_bk_on = on;
}
void set_lcd_te_pgio(u32 gpio)
{
    __this.lcd_te_io = gpio;
}
//两张相同尺寸的YUV图像合成 yuv420 合成背景纯黑
void yuv_picture_compose(u8 *in, u8 *out, u16 w, u16 h)
{
    u32 u_len;
    u32 v_len;
    u8 *y_in, *u_in, *v_in;
    u8 *y_out, *u_out, *v_out;
    u32 index_y;
    u32 index_uv;
    y_in = in;
    u_in = in + w * h;
    u_len = w * h / 4;
    v_in = in + w * h * 5 / 4;
    v_len = w * h / 4;
    y_out = out;
    u_out = out + w * h;
    v_out = out + w * h * 5 / 4;
    for (uint16_t j = 0; j < h; j++) {
        for (uint16_t i = 0; i < w; i++) {
            index_y = j * w + i;
            index_uv = j / 2 * w / 2 + i / 2;
            if (y_in[index_y] != 0 || u_in[index_uv] != 0x80 || v_in[index_uv] != 0x80) {
                y_out[index_y] = y_in[index_y];
                u_out[index_uv] = u_in[index_uv];
                v_out[index_uv] = v_in[index_uv];
            }
        }
    }
}
//图像合成逻辑处理
static void picture_compose(u8 *ui_in, u8 *camera_in, u16 in_LCD_W, u16 in_LCD_H)
{
    unsigned int n = 0;
    u16 *ui_data;
    u16 *camera_data;
    u16 *end_addr;
    ui_data = (u16 *)ui_in;
    camera_data = (u16 *)camera_in;
    switch (__this.compose_mode) {
    case 0://两张整图合成 ui数据覆盖到摄像头数据上
        end_addr = camera_data + in_LCD_H * in_LCD_W + 1;
        while (end_addr != camera_data) {
            if (*ui_data != FILTER_COLOR_RGB565) {
                *camera_data = *ui_data;
            }
            ui_data++;
            camera_data++;
        }
        break;
    case 1://UI顶部条状合成
        for (int j = 0; j < __this.user1_y_len; j++) {//Y
            for (int i = 0; i < __this.lcd_w; i++) {//X
                if (ui_in[n] != __this.aim_filter_color_l && ui_in[n + 1] != __this.aim_filter_color_h) {
                    camera_in[n] = ui_in[n];
                    camera_in[n + 1] = ui_in[n + 1];
                }
                n += 2;
            }
        }
        break;
    case 2://ui顶部和底部条状合成
        for (int j = 0; j < __this.user1_y_len; j++) {//Y
            for (int i = 0; i < __this.lcd_w; i++) {//X
                if (ui_in[n] != __this.aim_filter_color_l && ui_in[n + 1] != __this.aim_filter_color_h) {
                    camera_in[n] = ui_in[n];
                    camera_in[n + 1] = ui_in[n + 1];
                }
                n += 2;
            }
        }
        n = __this.user2_y * __this.lcd_w * 2;
        for (int j = 0; j < __this.user2_y_len; j++) {//Y
            for (int i = 0; i < __this.lcd_w; i++) {//X
                if (ui_in[n] != __this.aim_filter_color_l && ui_in[n + 1] != __this.aim_filter_color_h) {
                    camera_in[n] = ui_in[n];
                    camera_in[n + 1] = ui_in[n + 1];
                }
                n += 2;
            }
        }
        break;
    case 3://指定坐标开始 告知 长宽 //任意位置覆盖处理
        for (int j = 0; j < __this.aim_compose_h; j++) {//Y
            for (int i = 0; i < __this.aim_compose_w * 2; i += 2) { //X
                n = __this.start_x * 2 + __this.start_y * __this.lcd_w * 2 + i + j * __this.lcd_w * 2;
                ui_in[n] = camera_in[i + j * __this.aim_compose_w * 2 ];
                ui_in[n + 1] = camera_in[i + j * __this.aim_compose_w * 2 + 1];
            }
        }
        /* os_mutex_post(&mutex_rgb); */
        break;
    case 4://特殊拍照处理 大头贴两个半张显示 摄像头数据覆盖到ui数据上
        if (__this.piece == 0) { //显示上半张全覆盖无需过滤规则
            for (int j = 0; j < __this.lcd_h / 2; j++) { //Y
                for (int i = 0; i < __this.lcd_w; i++) {//X
                    ui_data[j * __this.lcd_w + i] = camera_data[j * __this.lcd_w + i];
                }
            }
        } else {//显示下半张
            for (int j = __this.lcd_h / 2; j < __this.lcd_h; j++) { //Y
                for (int i = 0; i < __this.lcd_w; i++) {//X
                    ui_data[j * __this.lcd_w + i] = camera_data[j * __this.lcd_w + i];
                }
            }
        }
        break;
    case 5://特殊拍照处理 以高三均分图像
        if (__this.piece == 0) {
            for (int j = 0; j < __this.lcd_h / 3; j++) { //Y
                for (int i = 0; i < __this.lcd_w; i++) {//X
                    ui_data[j * __this.lcd_w + i] = camera_data[j * __this.lcd_w + i];
                }
            }
        } else if (__this.piece == 1) {
            for (int j = __this.lcd_h / 3; j < __this.lcd_h * 2 / 3; j++) { //Y
                for (int i = 0; i < __this.lcd_w; i++) {//X
                    ui_data[j * __this.lcd_w + i] = camera_data[j * __this.lcd_w + i];
                }
            }
        } else if (__this.piece == 2) {
            for (int j = __this.lcd_h * 2 / 3; j < __this.lcd_h; j++) { //Y
                for (int i = 0; i < __this.lcd_w; i++) {//X
                    ui_data[j * __this.lcd_w + i] = camera_data[j * __this.lcd_w + i];
                }
            }
        }
        break;
    case 6://特殊拍照处理 对角线两张图像
        u32 k = 0;
        float kp = __this.lcd_w * 1.0 / __this.lcd_h;
        if (__this.piece == 0) {
            for (int j = 0; j < __this.lcd_h; j++) {//Y
                k = j * kp;
                for (int i = 0; i < (__this.lcd_w - k); i++) { //X
                    ui_data[j * __this.lcd_w + i] = camera_data[j * __this.lcd_w + i];
                }
            }
        } else if (__this.piece == 1) {
            for (int j = 0; j < __this.lcd_h; j++) {//Y
                k = __this.lcd_w - j * kp;
                for (int i = k; i < __this.lcd_w; i++) {//X
                    ui_data[j * __this.lcd_w + i] = camera_data[j * __this.lcd_w + i];
                }
            }
        }
        break;
    case 7://特殊拍照处理 不规则图像根据合成规律过滤
        u8 *make_data;//标记数据
        if (__this.init_value) {
            __this.init_value = 0;
            ui_data = (u16 *)ui_in;
            make_data = __this.binarization;//屏幕大小的内存块
            for (int j = 0; j < __this.lcd_h; j++) {//Y
                for (int i = 0; i < __this.lcd_w; i++) {//X	//遍历所有UI数据
                    if (*ui_data == FILTER_COLOR_RGB565) {  //图像合成过滤纯黑色
                        *make_data = 0x00;//黑
                    } else if (*ui_data == FILTER_COLOR_OUTER_FRAME) { //边缘有锯齿 需要范围 测试用  //外框颜色
                        *make_data = 0xff;//白
                    } else {
                        *make_data = 0x55;//其他的颜色
                    }
                    make_data++;
                    ui_data++;
                    camera_data++;
                }
            }
        }
        ui_data = (u16 *)ui_in;
        camera_data = (u16 *)camera_in;
        make_data = __this.binarization;       //地址复位
        end_addr = make_data + __this.lcd_w * __this.lcd_h;//尾部地址
        if (__this.piece == 0) {
            while (end_addr != make_data) {
                if (*make_data == 0x00) {
                    *ui_data = *camera_data;//在纯黑的区域把摄像头数据覆盖到ui数据上
                }
                make_data++;
                ui_data++;
                camera_data++;
            }
        } else if (__this.piece == 1) {
            while (end_addr != make_data) {//在其他颜色的区域把摄像头数据覆盖在ui数据上
                if (*make_data == 0x55) {
                    *ui_data = *camera_data;
                }
                make_data++;
                ui_data++;
                camera_data++;
            }
        }
        break;
    }
}
void (*cb_TE_send_lcd_data)(u8 *buf, u32 buf_size);
void init_TE(void (*cb)(u8 *buf, u32 buf_size))
{
    cb_TE_send_lcd_data = cb;
}
static void get_filter_color(void)
{
    u32 rgb888_color;
    u8 r, g, b;
    u16 rgb565_color;
    rgb888_color = FILTER_COLOR; //将过滤的颜色 RGB888  拆解为RGB565
    r = rgb888_color >> 19;
    g = rgb888_color >> 8;
    g = g >> 2;
    b = rgb888_color;
    b = b >> 3;
    rgb565_color = r;
    rgb565_color <<= 6;
    rgb565_color |= g;
    rgb565_color <<= 5;
    rgb565_color |= b;
    __this.aim_filter_color_l = rgb565_color >> 8;
    __this.aim_filter_color_h = rgb565_color;
}
//申请全局内存
void malloc_lcd_buf(void)
{
    if (__this.ui_save_buf == NULL) {
        __this.ui_save_buf = malloc(__this.lcd_data_size);
    }
    if (__this.camera_save_buf == NULL) {
        __this.camera_save_buf = malloc(__this.lcd_data_size);
    }
    if (__this.show_buf == NULL) {
        __this.show_buf = malloc(__this.lcd_data_size);
    }
    if (__this.ui_save_buf == NULL || __this.camera_save_buf == NULL || __this.show_buf == NULL) {
        printf("\n [ERROR] %s -[malloc lcd buf fail] %d\n", __FUNCTION__, __LINE__);
    }
}
void free_lcd_buf(void)
{
    if (__this.ui_save_buf) {
        free(__this.ui_save_buf);
    }
    if (__this.camera_save_buf) {
        free(__this.camera_save_buf);
    }
    if (__this.show_buf) {
        free(__this.show_buf);
    }
}
//全局lbuf内存申请
void malloc_lcd_lbuf_size(u8 lcd_piece)
{
    ASSERT(__this.lcd_data_size, "__this.lcd_data_size = 0");
    u32 buf_size = __this.lcd_data_size * lcd_piece;
    __this.lbuf = malloc(buf_size);
    __this.lbuf_handle = lbuf_init(__this.lbuf, buf_size, 4, sizeof(__this.lbuf));
    if (__this.lbuf == NULL || __this.lbuf_handle == NULL) {
        printf("\n [ERROR] %s -[no mem lcd_buf] %d\n", __FUNCTION__, __LINE__);
    }
    printf("[msg]>>>>>>>>>>>lbuf_data_size=%dk",  buf_size / 1024);
}
void free_lcd_lbuf(void)
{
    if (__this.lbuf) {
        free(__this.lbuf);
    }
    if (__this.lbuf_handle) {
        lbuf_free(__this.lbuf_handle);
    }
}
void malloc_binarization_buf(void)
{
    if (__this.binarization == NULL) {
        __this.binarization = malloc(__this.lcd_w * __this.lcd_h);
    }
}
void free_malloc_binarization_buf()
{
}
//获取全局UI数据内存指针
u8 *get_ui_buf(void)
{
    return __this.ui_save_buf;
}
//获取全局camera数据内存指针
u8 *get_camera_buf(void)
{
    return __this.camera_save_buf;
}
//获取全局推屏数据内存指针
u8 *get_show_buf(void)
{
    return __this.show_buf;
}
//清除lbuf保存的数据
void clear_te_lbuf(void)
{
    lbuf_clear(__this.lbuf_handle);
}
void get_data_use_flag(void)
{
    os_sem_pend(&task[0].wait_yuv_use_ok_sem, 50);
}
void get_data_use_flag1(void)
{
    os_sem_pend(&task[1].wait_yuv_use_ok_sem, 50);
}
void clear_os_pend_cnt(void)
{
    os_sem_set(&task[0].wait_yuv_use_ok_sem, 0);
    os_sem_set(&task[1].wait_yuv_use_ok_sem, 0);
    os_sem_set(&task[0].yuv_sem, 0);
    os_sem_set(&task[1].yuv_sem, 0);
    os_sem_set(&task[0].compose_sem, 0);
    os_sem_set(&task[1].compose_sem, 0);
    __this.task_time = 0;
}
void set_turn_mode(enum lcd_turn mode)
{
    __this.turn_mode = mode;
}
u8 get_lcd_turn_mode(void)
{
    return __this.turn_mode;
}
void set_photo_compose_mode(u8 mode, u8 choice_show_piece)
{
    __this.compose_mode = mode;
    __this.piece = choice_show_piece;
}
void set_init_value(u8 set_init_value)//设置mode_7中的ui数据读取
{
    __this.init_value = set_init_value;
}
u8 get_lcd_show_deta_mode(void)
{
    return __this.data_mode;
}
void set_compose_mode(u8 mode, int mode1_y_len, int mode2_y, int mode2_y_len)
{
    __this.compose_mode = mode;
    __this.user1_y_len =  mode1_y_len;
    __this.user2_y = mode2_y;
    __this.user2_y_len =  mode2_y_len;
}
void set_compose_mode3(int x, int y, int w, int h)
{
    __this.compose_mode = 3;
    __this.start_x = x;
    __this.start_y = y;
    __this.aim_compose_w = w;
    __this.aim_compose_h = h;
}
void set_lcd_show_data_mode(enum lcd_show_mode choice)
{
    u8 mode;
    mode = choice;
    switch (mode) {
    case UI:
        __this.data_mode = UI;
        printf("[LCD_MODE == ui]");
        break;
    case CAMERA:
        __this.data_mode = UI_CAMERA;
        __this.compose_status = 0;
        clear_os_pend_cnt();

        printf("[LCD_MODE == camera]");
        break;
    case UI_CAMERA:
        __this.data_mode = UI_CAMERA;
        __this.compose_status = 1;
        clear_os_pend_cnt();
        printf("[LCD_MODE == ui_camera]");
        break;
    case UI_USER:
        printf("[LCD_MODE == ui_user]");
        break;
    case UI_QR:
        printf("[LCD_MODE == ui_qr]");
    case UI_RGB:
        printf("[LCD_MODE == ui_rgb]");
        break;
    }
}
void set_lcd_ui_x_y(u16 xstart, u16 xend, u16 ystart, u16 yend)
{
    __this.ui_xstart = xstart;
    __this.ui_xend = xend ;
    __this.ui_ystart = ystart ;
    __this.ui_yend = yend;
}
void user_send_no_data_ready(int x, int y, int w, int h, char *img)
{
    if (__this.data_mode == UI_RGB) {
        set_compose_mode3(x, y, w, h);
        picture_compose(__this.ui_save_buf, img, __this.lcd_w, __this.lcd_h);
    }
}
static void send_data_to_lcd(u8 *buf, u32 size)
{
    static u8 time = 0;
    time = ++time % 2;
    if (buf == NULL) {
        printf("\n [ERROR] %s -[out buf NULL] %d\n", __FUNCTION__, __LINE__);
    }
    __this.out_lcd_buf = buf;
    __this.data_size = size;
    task[time].wait_te_ready = 1;
}
void user_send_no_data_ready1(int x, int y, int w, int h, char *img)
{
    if (__this.data_mode == UI_RGB) {
        set_compose_mode3(x, y, w, h);
        picture_compose(__this.ui_save_buf, img, __this.lcd_w, __this.lcd_h);
#ifdef turn_180
        RGB565_Soft_90(0, __this.show_buf, __this.ui_save_buf, __this.lcd_w,  __this.lcd_h);
#else
        RGB565_Soft_90(1, __this.show_buf, __this.ui_save_buf, __this.lcd_w,  __this.lcd_h);
#endif
        send_data_to_lcd(__this.show_buf, __this.lcd_data_size);
    }
}
void user_send_data_ready(int x, int y, int w, int h, char *img)
{
    if (__this.data_mode == UI_RGB) {
        set_compose_mode3(x, y, w, h);
        picture_compose(__this.ui_save_buf, img, __this.lcd_w, __this.lcd_h);
#ifdef turn_180
        RGB565_Soft_90(0, __this.show_buf, __this.ui_save_buf, __this.lcd_w,  __this.lcd_h);
#else
        RGB565_Soft_90(1, __this.show_buf, __this.ui_save_buf, __this.lcd_w,  __this.lcd_h);
#endif
        send_data_to_lcd(__this.show_buf, __this.lcd_data_size);
        /* os_sem_pend(&send_ok_sem, 0); */
    }
}
void qr_data_updata(u8 *data_buf, u32 data_size)
{
    memcpy(__this.show_buf, data_buf, data_size);
    os_sem_post(&task[0].compose_sem);
}
//lcd发送完成该接口放在显示发送完成中断回调
void lcd_send_ok(void)
{
    for (u8 i = 0; i < 2; i++) {
        if (task[i].send_data_ok_flag) {
            task[i].send_data_ok_flag = 0;
            os_sem_post(&task[i].yuv_sem);
        }
    }
}
//屏幕TE脚触发中断信号
static void TE_interrupt(void)
{
    for (u8 i = 0; i < 2; i++) {
        if (task[i].wait_te_ready) {
            task[i].wait_te_ready = 0;
            os_sem_post(&__this.te_ready_sem);
            break;
        }
    }
}

static void no_Te_sned_post(u8 id)
{
    if (task[id].wait_te_ready) {
        task[id].wait_te_ready = 0;
        os_sem_post(&__this.te_ready_sem);
    }
}
//数据发送线程最终推屏的地方
static void te_send_task(void)
{
    u8 *out_buf_addr;
    static u8 time = 0;
    while (1) {
        os_sem_pend(&__this.te_ready_sem, 0);
        switch (__this.data_mode) {
        case UI://ok
            cb_TE_send_lcd_data(__this.out_lcd_buf, __this.lcd_data_size);
            break;
        case UI_CAMERA:
            for (u8 i = 0; i < 2; i++) {
                if (task[i].data_ok_flag) {
                    task[i].data_ok_flag = 0;
                    task[i].send_data_ok_flag = 1;
                    out_buf_addr = task[i].out_lcd_buf;
                    cb_TE_send_lcd_data(out_buf_addr, __this.lcd_data_size);
                    os_sem_post(&task[i].wait_yuv_use_ok_sem);
                    break;
                }
            }
            break;
        case UI_USER:
            break;
        case UI_QR:
            break;
        }
        if (time == 2) { //显示第二帧才开背光
            lcd_bl_pinstate(__this.lcd_bk_on);
        } else {
            if (time < 2) {
                time++;
            }
        }
#if (USE_LCD_TE == 0)
        Calculation_frame_no_te();
#else
        Calculation_frame();
#endif
    }
}
/*将获取到的UI数据保存在一帧数据中并且是对应位置关系*/ /*ui那边发送的数据会告知开窗坐标数据buf*/ //ui数据处理接口
void ui_send_data_ready(u8 *data_buf, u32 data_size)
{
    u32 n = 0;
    static u8 time = 0;
    time = ++time % 2;
    //过来的ui数据首先由ui工程决定 其次由驱动配置的宽高决定
    //自动识别ui方向 lcdw = 320 lcdh = 480
    //屏幕尺寸固定 w320 h480
    //如果宽大于高为横屏 高大于宽为竖屏
    if (__this.ui_xend - __this.ui_xstart > (__this.lcd_w - 1)) { //横屏的时候 xe-xs = 480
        __this.turn_mode = LCD_TURN_90;
    } else if (__this.ui_yend - __this.ui_ystart > (__this.lcd_w - 1)) { //竖屏的时候 ye-ys = 480
        __this.turn_mode = LCD_TURN_0;
    }
    if (__this.data_mode == UI || __this.data_mode == UI_CAMERA) {
        if (__this.turn_mode == LCD_TURN_0) { //竖屏ui推竖屏
            for (int j = 0; j < (__this.ui_yend - __this.ui_ystart + 1);  j++) {
                for (int i = 0; i < (__this.ui_xend - __this.ui_xstart + 1); i++) {
                    __this.ui_save_buf[n + (__this.lcd_w - __this.ui_xend - 1) * 2 * j + __this.ui_ystart * __this.lcd_w * 2 + __this.ui_xstart * 2 * (j + 1)] = data_buf[n];
                    __this.ui_save_buf[n + 1 + (__this.lcd_w - __this.ui_xend - 1) * 2 * j + __this.ui_ystart * __this.lcd_w * 2 + __this.ui_xstart * 2 * (j + 1)] = data_buf[n + 1];
                    n += 2;
                }
            }
        } else if (__this.turn_mode == LCD_TURN_90 || __this.turn_mode == LCD_TURN_270) { //横屏ui推竖屏需要旋转到对应位置
            //举例 这里屏幕 W 320 H 480  ui w 480 h 320
            //需要将ui过来的小包数据放到对应位置
            u16 *in_buf = (u16 *)data_buf;
            u16 *out_buf = (u16 *)__this.ui_save_buf;
            for (u16 y = __this.ui_ystart; y <= __this.ui_yend; y++) {//j = x   i = y
                for (u16 x = __this.ui_xstart; x <= __this.ui_xend; x++) {
                    out_buf[__this.lcd_w - y + __this.lcd_w * x] =  *in_buf++;
                }
            }
            if (__this.turn_mode == LCD_TURN_270) {
                printf("\n [error] %s -[Time occupation not supported] %d\n", __FUNCTION__, __LINE__);
            }
        }
        if (__this.data_mode == UI) {
            os_sem_post(&task[time].compose_sem);
        }
    } else {
        printf("\n [msg] %s -[have ui data run] %d\n", __FUNCTION__, __LINE__);
    }
}
//摄像头数据处理接口
//如果还想提示性能将缩放函数和旋转转码函数揉在一起减少内存和提升效率 目前效率够用
void camera_send_data_ready(u8 *buf, u32 size, int width, int height)
{
    //YUV数据流程
    if (__this.data_mode == CAMERA || __this.data_mode == UI_CAMERA) {
        //输入尺寸跟屏幕方向大小一致均为竖屏
        if (width == __this.lcd_w && height == __this.lcd_h) {
            __this.camera_data_mode = 1;
            task[__this.task_time].lcd_data = buf;
            os_sem_post(&task[__this.task_time].compose_sem);
            __this.task_time = ++__this.task_time % 2;
            return;
        } else if (width == __this.lcd_h && height == __this.lcd_w) { //传入数据为大小相同方向不同的yuv
            __this.camera_data_mode = 4;
            task[__this.task_time].lcd_data = buf;
            os_sem_post(&task[__this.task_time].compose_sem);
            __this.task_time = ++__this.task_time % 2;
            return;
        }
        //跟屏幕大小不一致 分两种情况 一种是竖屏一种是横屏输入 屏幕都是竖屏
        if (width > height && width > __this.lcd_h && height > __this.lcd_w) { //传入数据为横屏yuv 并且数据大小比屏幕大
            __this.camera_data_mode = 2;
            if (__this.lbuf_handle == NULL) {
                malloc_lcd_lbuf_size(2);
            }
            __this.w_lbuf = lbuf_alloc(__this.lbuf_handle, __this.lcd_yuv420_data_size); //lbuf内申请一块空间
            if (__this.w_lbuf == NULL) {
                log_info("%s >>>lbuf no buf\r\n", __func__);
                return;
            } else {
                YUV420p_Soft_Scaling(buf, __this.w_lbuf, width, height, __this.lcd_h, __this.lcd_w);
                lbuf_push(__this.w_lbuf, BIT(0));//把数据块推送更新到lbuf的通道0
                buf = __this.w_lbuf;
            }
        } else if (width < height && width > __this.lcd_w && height > __this.lcd_h) { //传入数据为竖屏YUV 并且比屏幕大
            __this.camera_data_mode = 3;
            YUV420p_Soft_Scaling(buf, NULL, width, height, __this.lcd_w, __this.lcd_h);
        } else if (width > height && width > __this.lcd_h && height > __this.lcd_w) { //传入数据为横屏yuv 并且比屏幕大
            __this.camera_data_mode = 5;
            YUV420p_Soft_Scaling(buf, NULL, width, height, __this.lcd_h, __this.lcd_w);
            return;
        } else if (width < height && width < __this.lcd_w && height < __this.lcd_h) { //传入数据为竖屏YUV 并且比屏幕小
            printf("\n [ERROR] %s -[Time occupation not supported width < height && width < __this.lcd_w && height < __this.lcd_h] %d\n", __FUNCTION__, __LINE__);
            return;
        }
        task[__this.task_time].lcd_data = buf;
        os_sem_post(&task[__this.task_time].compose_sem);
        __this.task_time = ++__this.task_time % 2;
    }
}
//ui数据,图像合成,经摄像头,用户数据处理线程
void picture_compose_main(u8 id)
{
    u8 time_out;
    while (1) {
start:
        time_out = os_sem_pend(&task[id].compose_sem, 50);//不能加超时等 防止另一个线程超时发包
        if (time_out == OS_TIMEOUT) { //超时就重置  使得每次都是重第一个线程开始
            clear_os_pend_cnt();
            goto start;
        }
        switch (__this.data_mode) {
        case UI://屏数据只有UI数据
            send_data_to_lcd(__this.ui_save_buf, __this.lcd_data_size);
#if (USE_LCD_TE == 0)
            no_Te_sned_post(id);
#endif
            break;
        case UI_CAMERA://屏数据有实时更新的摄像头数据已经ui数据 二者做图像合成
            if (task[id].lcd_data != NULL) {
                task[id].in_lcd_buf = task[id].lcd_data;
                if (id == 0) {
                    task[id].out_lcd_buf = __this.camera_save_buf;
                } else {
                    task[id].out_lcd_buf = __this.show_buf;
                }
                if (__this.camera_data_mode == 1 || __this.camera_data_mode == 3) {//输入尺寸跟屏幕方向大小一致均为竖屏 //传入数据为竖屏yuv且比屏幕大
                    yuv420p_quto_rgb565(task[id].in_lcd_buf, task[id].out_lcd_buf, __this.lcd_w, __this.lcd_h, 1);
                } else if (__this.camera_data_mode == 4 || __this.camera_data_mode == 5) {//传入数据为大小相同方向不同的yuv
                    //将同尺寸的yuv旋转转码为rgb
#ifdef CONFIG_UVC_YUV422_ENABLE
                    if (get_jpeg2yuv_yuv_type() == 4) {
                        yuv420p_quto_rotate_rgb565(task[id].in_lcd_buf, task[id].out_lcd_buf, __this.lcd_h, __this.lcd_w);
                    } else {
                        yuv422p_quto_rotate_rgb565(task[id].in_lcd_buf, task[id].out_lcd_buf, __this.lcd_h, __this.lcd_w);
                    }
#else
                    yuv420p_quto_rotate_rgb565(task[id].in_lcd_buf, task[id].out_lcd_buf, __this.lcd_h, __this.lcd_w);
#endif
                } else if (__this.camera_data_mode == 2) { //传入数据为横屏yuv且比屏幕大
                    if (__this.lbuf_handle != NULL) { //没有加缩放转码旋转接口 需要 用多两个buf
                        task[id].r_lbuf = lbuf_pop(__this.lbuf_handle, BIT(0));//从lbuf的通道0读取数据块
                        if (task[id].r_lbuf == NULL) {
                            log_info("%s >>>note lbuf r_lbuf == NULL\r\n", __func__);
                            goto start;
                        } else {
                            task[id].in_lcd_buf = task[id].r_lbuf;
                        }
                        yuv420p_quto_rotate_rgb565(task[id].in_lcd_buf, task[id].out_lcd_buf, __this.lcd_h, __this.lcd_w);
                        lbuf_free(task[id].r_lbuf);
                    } else {
                        printf("\n [ERROR] %s -[__this.lbuf_handle != NULL] %d\n", __FUNCTION__, __LINE__);
                    }
                }
                if (__this.compose_status) { //图像数据是否要跟ui数据进行合成
                    //图像合成处理将ui_save_buf覆盖到out_lcd_buf
                    picture_compose(__this.ui_save_buf, task[id].out_lcd_buf, __this.lcd_w, __this.lcd_h);
                }
                task[id].data_ok_flag = 1;//用于区分是那个线程的数据处理好了
                task[id].wait_te_ready = 1;//告知TE中断数据已经处理好可以发数据
#if (USE_LCD_TE == 0)
                no_Te_sned_post(id);
#endif
                os_sem_pend(&task[id].yuv_sem, 0); //等数据被发送出去后才能再进 防止数据追尾
            }
            break;
        case UI_USER: //屏数据为使用者进行查看照片更新UI时需要切换的模式
            break;
        case UI_QR://屏数据有实时更新的摄像头数据已经ui数据 二者做图像合成
            break;
        }
    }
}
//图像处理数据线程0
static void picture_compose_task_0(void *priv)
{
    picture_compose_main(0);
}
//图像处理数据线程1
static void picture_compose_task_1(void *priv)
{
    picture_compose_main(1);
}
/* 线程创建 */
void picture_compose_task_init(void)
{
    if (!__this.lcd_te_io || !__this.lcd_w || !__this.lcd_h || !__this.lcd_data_size || !__this.lcd_yuv420_data_size) {
        printf("\n [ERROR] %s -[you_need_init lcd_te_io, lcd_w, lcd_h, lcd_data_size, lcd_yuv420_data_size ] %d\n", __FUNCTION__, __LINE__);
    }
#if USE_LCD_TE
    //初始化TE中断
    int ret = port_wakeup_reg(EVENT_IO_1, __this.lcd_te_io, EDGE_POSITIVE, TE_interrupt);
    if (ret) {
        printf("port_wakeup_reg success.\n");
    } else {
        printf("port_wakeup_reg fail.\n");
    }
#else
    lcd_bl_pinstate(__this.lcd_bk_on);
#endif
    os_sem_create(&__this.te_ready_sem, 0);
    for (u8 i = 0; i < 2; i++) {
        os_sem_create(&task[i].compose_sem, 0);
        os_sem_create(&task[i].yuv_sem, 0);
        os_sem_create(&task[i].wait_yuv_use_ok_sem, 0);
    }
    //计算一遍过滤颜色
    get_filter_color();
    //初始化模式为ui模式
    __this.compose_mode = 0;
    __this.data_mode = UI;
    __this.turn_mode = LCD_TURN_0;
    //双核加速流程思想  数据交替分配给两个线程进行数据处理 让系统自己调度
    task_create(picture_compose_task_0, 0, "lcd_task_0");
    task_create(picture_compose_task_1, 0, "lcd_task_1");
    task_create(te_send_task, 0, "te_task");
}


