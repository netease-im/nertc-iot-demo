/**
 * @file lv_port_disp.c
 *
 */
/*********************
 *      INCLUDES
 *********************/
#define BOOL_DEFINE_CONFLICT
#include "lv_port_disp.h"
#include "src/display/lv_display_private.h"
#include <stdbool.h>
#include "lcd_drive.h"

/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
#warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
#define MY_DISP_HOR_RES    320
#endif

#ifndef MY_DISP_VER_RES
#warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen height, default value 240 is used for now.
#define MY_DISP_VER_RES    240
#endif

/**********************
 *      TYPEDEFS
 **********************/
#if LV_COLOR_DEPTH==16
#define LV_PIXEL_COLOR_T lv_color16_t
#endif

/**********************
 *  STATIC PROTOTYPES
 **********************/
extern void lcd_lvgl_full(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img);

static void disp_init(void);

static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/
/* #define THREE_FB_ACCELERATION  //利用双核双线程+3FB加速 ,某些场景能够提升1-2帧  */

#ifdef THREE_FB_ACCELERATION
#include "os/os_api.h"
#include "spinlock.h"
static OS_SEM mcu_lcd_physics_flush_sem, mcu_lcd_physics_flush_end_sem;
static LV_PIXEL_COLOR_T buf_3_3[MY_DISP_HOR_RES * MY_DISP_VER_RES] __attribute__((aligned(32)));            /*3th screen sized buffer*/

struct lv_fb_t {
    LV_PIXEL_COLOR_T *fb;
    lv_area_t area;
};
static struct lv_fb_t cur_disp;/*当前 LCD 的显示显存*/
volatile static struct lv_fb_t next_disp;/* next_fb地址 */
static struct lv_fb_t next_offline_disp;/* 上一帧online_fb，也是下一帧offline_fb */
static spinlock_t fb_lock; //双核互斥锁
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
#if LV_COLOR_DEPTH==16
static void *lv_color_16_swap(const lv_area_t *area, uint8_t *px_map)
{
    /*printf("lv_color_16_swap -> %d, %d, %d, %d, 0x%x",area->x1,area->x2,area->y1,area->y2,px_map);*/
    static LV_PIXEL_COLOR_T buf_swap[MY_DISP_HOR_RES * MY_DISP_VER_RES] __attribute__((aligned(32)));
    uint8_t *data = buf_swap;

    if (LV_GLOBAL_DEFAULT()->disp_refresh->render_mode != LV_DISPLAY_RENDER_MODE_FULL) {
        for (int i = 0; i < (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1); i++) { //芯片EMI硬件不支持小端输出， 只能够软件转换, 正常是不允许修改fb，否则把绘制内容给改掉
            uint16_t color = ((uint16_t *)px_map)[i];
            data[2 * i] = (color >> 8) & 0xff;
            data[2 * i + 1] = color & 0xff;
        }
    } else {
        for (int i = 0; i < MY_DISP_HOR_RES * MY_DISP_VER_RES; i++) { //芯片EMI硬件不支持小端输出， 只能够软件转换, 正常是不允许修改fb，否则把绘制内容给改掉
            uint16_t color = ((uint16_t *)px_map)[i];
            data[2 * i] = (color >> 8) & 0xff;
            data[2 * i + 1] = color & 0xff;
        }
    }

    return data;
}
#endif

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t *disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);
#if 0
    /* Example 1
     * One buffer for partial rendering*/
    static LV_PIXEL_COLOR_T buf_1_1[MY_DISP_HOR_RES * 10];                          /*A buffer for 10 rows*/
    lv_display_set_buffers(disp, buf_1_1, NULL, sizeof(buf_1_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif
#if 0
    /* Example 2
     * Two buffers for partial rendering
     * In flush_cb DMA or similar hardware should be used to update the display in the background.*/
    static LV_PIXEL_COLOR_T buf_2_1[MY_DISP_HOR_RES * 10];
    static LV_PIXEL_COLOR_T buf_2_2[MY_DISP_HOR_RES * 10];
    lv_display_set_buffers(disp, buf_2_1, buf_2_2, sizeof(buf_2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

    /* Example 3
     * Two buffers screen sized buffer for double buffering.
     * Both LV_DISPLAY_RENDER_MODE_DIRECT and LV_DISPLAY_RENDER_MODE_FULL works, see their comments*/
    static LV_PIXEL_COLOR_T buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES] __attribute__((aligned(32)));            /*A screen sized buffer*/
    static LV_PIXEL_COLOR_T buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES] __attribute__((aligned(32)));            /*Another screen sized buffer*/

#ifdef THREE_FB_ACCELERATION
    lv_display_set_buffers(disp, buf_3_1, buf_3_2, buf_3_3, sizeof(buf_3_1), LV_DISPLAY_RENDER_MODE_FULL);
    /*lv_display_set_buffers(disp, buf_3_1, buf_3_2, buf_3_3, sizeof(buf_3_1), LV_DISPLAY_RENDER_MODE_PARTIAL);*/
#else
    lv_display_set_buffers(disp, buf_3_1, buf_3_2, NULL, sizeof(buf_3_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

#ifdef THREE_FB_ACCELERATION
    /* next_offline_fb 变量保存下一帧的 offline_fb 地址  */
    next_offline_disp.fb = buf_3_3;
    /* 调用系统API把 online_fb 的地址设置为当前 LCD 的显示显存 */
    cur_disp.fb = buf_3_2;

    lcd_interface_set_non_block(0); //因为有专用线程, 因此使用阻塞方式释放cpu
    os_sem_create(&mcu_lcd_physics_flush_sem, 0);
    os_sem_create(&mcu_lcd_physics_flush_end_sem, 1);
    void lvgl_lcd_draw_task(void *p);
    thread_fork("lvgl_lcd_draw_task", 29, 200, 0, NULL, lvgl_lcd_draw_task, NULL);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#ifdef THREE_FB_ACCELERATION
static void lv_lcd_draw_trig(const lv_area_t *area)
{
    /* 等到s_next_fb空闲同步已完成 */
    os_sem_pend(&mcu_lcd_physics_flush_end_sem, 0);
    /*while (next_disp.fb != NULL) {};*/
    spin_lock(&fb_lock);
    /* 将绘制好的 offline_fb 设置为 next_fb */
    next_disp.fb = LV_GLOBAL_DEFAULT()->disp_refresh->buf_act->data;
    if (LV_GLOBAL_DEFAULT()->disp_refresh->render_mode != LV_DISPLAY_RENDER_MODE_FULL) {
        memcpy((void *)&next_disp.area, area, sizeof(lv_area_t));
    }
    /* 把上一帧的 online_fb 作为下一帧的 offline_fb */
    LV_GLOBAL_DEFAULT()->disp_refresh->buf_act->data = next_offline_disp.fb;
    spin_unlock(&fb_lock);

    /*os_sem_set(&mcu_lcd_physics_flush_sem,0);*/
    os_sem_post(&mcu_lcd_physics_flush_sem);
}

void lvgl_lcd_draw_task(void *p)
{
    LV_PIXEL_COLOR_T *pixel;
    while (1) {
        os_sem_pend(&mcu_lcd_physics_flush_sem, 0);

        while (next_disp.fb != NULL) {
            spin_lock(&fb_lock);
            /* next_offline_fb 变量保存上一帧的 online_fb 地址 */
            next_offline_disp.fb = cur_disp.fb;
            /* 调用系统API把 online_fb 的地址设置为当前 LCD 的显示显存 */
            cur_disp.fb = next_disp.fb;
            if (LV_GLOBAL_DEFAULT()->disp_refresh->render_mode != LV_DISPLAY_RENDER_MODE_FULL) {
                memcpy(&cur_disp.area, (const void *)&next_disp.area, sizeof(lv_area_t));
            }
            next_disp.fb = NULL;
            spin_unlock(&fb_lock);
            os_sem_post(&mcu_lcd_physics_flush_end_sem);
#if LV_COLOR_DEPTH==16
            pixel = lv_color_16_swap(&cur_disp.area, cur_disp.fb);
#else
            pixel = cur_disp.fb;
#endif
            if (LV_GLOBAL_DEFAULT()->disp_refresh->render_mode != LV_DISPLAY_RENDER_MODE_FULL) {
                lcd_lvgl_full(cur_disp.area.x1, cur_disp.area.x2, cur_disp.area.y1, cur_disp.area.y2, pixel);
            } else {
                lcd_lvgl_full(0, MY_DISP_HOR_RES, 0, MY_DISP_VER_RES, pixel);
            }
        }
    }
}
#endif

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
    user_lcd_init();
}
bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/

static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map)
{
    if (disp_flush_enabled) {
        if (LV_GLOBAL_DEFAULT()->disp_refresh->flushing_last) {
#if 1
            static u32 time_lapse_hdl;
            static u8 fps_cnt;
            ++fps_cnt;
            u32 tdiff = time_lapse(&time_lapse_hdl, 1000);
            if (tdiff) {
                printf("lv draw %d fps\n", fps_cnt *  1000 / tdiff);
                fps_cnt = 0;
            }
            /*return;//仅统计软件渲染性能*/
#endif
        }

#ifdef THREE_FB_ACCELERATION
        lv_lcd_draw_trig(area);
#else
#if LV_COLOR_DEPTH==16
        px_map = lv_color_16_swap(area, px_map);
#endif
        lcd_lvgl_full(area->x1, area->x2, area->y1, area->y2, px_map);
#endif
    }
    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}
