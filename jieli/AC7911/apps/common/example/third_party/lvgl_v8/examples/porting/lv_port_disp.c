/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#define BOOL_DEFINE_CONFLICT
#include "lv_port_disp.h"
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

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/

/**********************
 *      MACROS
 **********************/
/* #define THREE_FB_ACCELERATION  //利用双核双线程+3FB加速 ,防止mcu lcd刷新完还空隙闲着,某些场景能够提升1-2帧    */

#ifdef THREE_FB_ACCELERATION
#include "os/os_api.h"
#include "spinlock.h"
static OS_SEM mcu_lcd_physics_flush_sem, mcu_lcd_physics_flush_end_sem;
static lv_color_t buf_3_3[MY_DISP_HOR_RES * MY_DISP_VER_RES] __attribute__((aligned(4)));            /*3th screen sized buffer*/

struct lv_fb_t {
    lv_color_t *fb;
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

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */
#if 0
    /* Example for 1) */
    static lv_disp_draw_buf_t draw_buf_dsc_1;
    static lv_color_t buf_1[MY_DISP_HOR_RES * 10] __attribute__((aligned(4)));    /*A buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 10);   /*Initialize the display buffer*/
#endif

#if 0
    /* Example for 2) */
    static lv_disp_draw_buf_t draw_buf_dsc_2;
    static lv_color_t buf_2_1[MY_DISP_HOR_RES * 10] __attribute__((aligned(4)));                        /*A buffer for 10 rows*/
    static lv_color_t buf_2_2[MY_DISP_HOR_RES * 10] __attribute__((aligned(4)));                        /*An other buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * 10);   /*Initialize the display buffer*/
#endif

#if 1
    static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES] __attribute__((aligned(4)));            /*A screen sized buffer*/
    static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES] __attribute__((aligned(4)));            /*Another screen sized buffer*/
    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
    static lv_disp_draw_buf_t draw_buf_dsc_3;

#ifndef THREE_FB_ACCELERATION
#define  buf_3_3 NULL
#endif
    lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2, buf_3_3,
                          MY_DISP_HOR_RES * MY_DISP_VER_RES);   /*Initialize the display buffer*/
#endif
    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_3;

    /*Required for Example 3)*/
#ifdef THREE_FB_ACCELERATION //3FB BUG NEED FIX
    disp_drv.full_refresh = 1; //0:局部区域重绘,适合MCU屏, 1:适合RGB屏,整帧刷
#endif

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);

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


volatile bool disp_flush_enabled = true;

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

#ifdef THREE_FB_ACCELERATION
static void lv_lcd_draw_trig(const lv_area_t *area)
{
    /* 等到s_next_fb空闲同步已完成 */
    os_sem_pend(&mcu_lcd_physics_flush_end_sem, 0);
    /*while (next_disp.fb != NULL) {};*/

    spin_lock(&fb_lock);
    /* 将绘制好的 offline_fb 设置为 next_fb */
    lv_disp_draw_buf_t *draw_buf = lv_disp_get_draw_buf(_lv_refr_get_disp_refreshing());
    next_disp.fb = draw_buf->buf_act;
    if (disp_drv.full_refresh == 0) {
        memcpy((void *)&next_disp.area, area, sizeof(lv_area_t));
    }
    /* 把上一帧的 online_fb 作为下一帧的 offline_fb */
    draw_buf->buf_act = next_offline_disp.fb;
    spin_unlock(&fb_lock);

    /*os_sem_set(&mcu_lcd_physics_flush_sem,0);*/
    os_sem_post(&mcu_lcd_physics_flush_sem);
}

void lvgl_lcd_draw_task(void *p)
{
    while (1) {
        os_sem_pend(&mcu_lcd_physics_flush_sem, 0);

        while (next_disp.fb != NULL) {
            spin_lock(&fb_lock);
            /* next_offline_fb 变量保存上一帧的 online_fb 地址 */
            next_offline_disp.fb = cur_disp.fb;
            /* 调用系统API把 online_fb 的地址设置为当前 LCD 的显示显存 */
            cur_disp.fb = next_disp.fb;
            if (disp_drv.full_refresh == 0) {
                memcpy(&cur_disp.area, (const void *)&next_disp.area, sizeof(lv_area_t));
            }
            next_disp.fb = NULL;
            spin_unlock(&fb_lock);
            os_sem_post(&mcu_lcd_physics_flush_end_sem);
            if (disp_drv.full_refresh == 0) {
                lcd_lvgl_full(cur_disp.area.x1, cur_disp.area.x2, cur_disp.area.y1, cur_disp.area.y2, cur_disp.fb);
            } else {
                lcd_lvgl_full(0, MY_DISP_HOR_RES, 0, MY_DISP_VER_RES, cur_disp.fb);
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

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (disp_flush_enabled) {
        lv_disp_draw_buf_t *draw_buf = lv_disp_get_draw_buf(_lv_refr_get_disp_refreshing());
        if (draw_buf->flushing_last) {
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
        lcd_lvgl_full(area->x1, area->x2, area->y1, area->y2, color_p);
#endif

        /*IMPORTANT!!!
         *Inform the graphics library that you are ready with the flushing*/
        lv_disp_flush_ready(disp_drv);
    }
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
