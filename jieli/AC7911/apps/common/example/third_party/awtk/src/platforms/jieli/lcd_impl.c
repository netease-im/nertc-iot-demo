#include "awtk.h"
#include "lcd/lcd_mem_bgr565.h"

typedef uint16_t pixel_t;

#ifndef FRAGMENT_FRAME_BUFFER_SIZE

static pixel_t s_framebuffers[2][LCD_W * LCD_H];

#define DUAL_THREAD_ACCELERATION  //利用双核双线程加速,目前分割工作量实测可加速44%-66%, 但是也会占据另外一个核的44%-66%性能

#define FB_SWAP_OR_FLUSH 1  //0:flush,  1:swap

#define SUPPORT_DIRTY_RECT

#ifdef DUAL_THREAD_ACCELERATION
#include "os/os_api.h"
static OS_SEM mcu_lcd_physics_flush_sem, mcu_lcd_physics_flush_end_sem;
#endif


#if FB_SWAP_OR_FLUSH==0
#define fbaddr  s_framebuffers[0]
static ret_t (*lcd_mem_flush_defalut)(lcd_t *lcd);
#else
static pixel_t *fbaddr;
#endif

static ret_t lcd_mem_wait_vbi(void *ctx)
{
#ifdef DUAL_THREAD_ACCELERATION
    os_sem_pend(&mcu_lcd_physics_flush_end_sem, 0);
#else
    lcd_interface_non_block_wait();
#endif
}

static void _mcu_lcd_physics_flush(lcd_t *lcd)
{
    pixel_t color;
    uint8_t *data;

#ifdef SUPPORT_DIRTY_RECT
    if (lcd->support_dirty_rect == TRUE) { //如果支持脏矩形，软件转换会改到干净矩形的区域,所以暂时先多加一个fb，后面还需要优化成只修改脏矩形区域，否则浪费一块fb内存和性能
        static pixel_t fb2[LCD_W * LCD_H];
        data = fb2;
    } else
#endif
    {
        data = (uint8_t *)fbaddr;
    }
    for (int i = 0; i < LCD_W * LCD_H; i++) { //芯片EMI硬件不支持小端输出， 只能够软件转换, 正常是不允许修改fb，否则把绘制内容给改掉
        pixel_t color = fbaddr[i] ;
        data[2 * i] = (color >> 8) & 0xff;
        data[2 * i + 1] = color & 0xff;
    }
    lcd_lvgl_full(0, LCD_W - 1, 0, LCD_H - 1, data);
}

static void mcu_lcd_physics_flush(lcd_t *lcd)
{
#ifdef DUAL_THREAD_ACCELERATION
    os_sem_post(&mcu_lcd_physics_flush_sem);
#else
    _mcu_lcd_physics_flush(lcd);
#endif
}

#ifdef DUAL_THREAD_ACCELERATION
void awtk_mcu_lcd_physics_flush_task(void *p)
{
    while (1) {
        os_sem_pend(&mcu_lcd_physics_flush_sem, 0);
        _mcu_lcd_physics_flush(p);
        os_sem_post(&mcu_lcd_physics_flush_end_sem);
    }
}
#endif

#if FB_SWAP_OR_FLUSH==0
static ret_t mcu_lcd_mem_flush(lcd_t *lcd)
{
    /*return RET_OK; */
    if (lcd_mem_flush_defalut) { //双显存重新实现
        lcd_mem_flush_defalut(lcd);
        mcu_lcd_physics_flush(lcd); //整屏刷新,可修改为脏矩形刷新
    } else { //单显存
        mcu_lcd_physics_flush(lcd);//单显存，底层硬件务必是阻塞的
    }

    return RET_OK;
}
#endif

#if FB_SWAP_OR_FLUSH==1
static ret_t lcd_mem_swap(lcd_t *lcd)
{
    /*return RET_OK; */

    lcd_mem_t *mem = (lcd_mem_t *)lcd;
    uint8_t *tmp_fb = mem->offline_fb;

    /* 调用系统API加入垂直同步等待来保证画面没有撕裂 */
#ifdef DUAL_THREAD_ACCELERATION
    os_sem_pend(&mcu_lcd_physics_flush_end_sem, 0);
#endif

    /*
     * 调用系统API把 offline_fb 设置为系统 LCD 使用的显存地址,
     * 随后交换 offline_fb 和 online_fb 地址。
     */
    fbaddr = mem->offline_fb;

    mcu_lcd_physics_flush(lcd);

    lcd_mem_set_offline_fb(mem, mem->online_fb);
    lcd_mem_set_online_fb(mem, tmp_fb);

    return RET_OK;
}
#endif


lcd_t *platform_create_lcd(wh_t w, wh_t h)
{
    printf("awtk platform_create_lcd w=%d,h=%d \r\n", w, h);

    /*lcd_t *lcd = lcd_mem_bgr565_create_single_fb(w, h, s_framebuffers);*/
    lcd_t *lcd = lcd_mem_bgr565_create_double_fb(w, h, s_framebuffers[0], s_framebuffers[1]);

#if FB_SWAP_OR_FLUSH==1
    lcd->swap = lcd_mem_swap;         /* 重载 swap 函数 */
#ifndef SUPPORT_DIRTY_RECT
    lcd->support_dirty_rect = FALSE;  /* 脏矩形机制 */
#endif
#else
    lcd_mem_flush_defalut = lcd->flush;//lcd_mem_flush
    lcd->flush = mcu_lcd_mem_flush;
#endif

    lcd_mem_set_wait_vbi(lcd, lcd_mem_wait_vbi, lcd);

#ifdef DUAL_THREAD_ACCELERATION
    lcd_interface_set_non_block(0);// mcu lcd硬件接口要求阻塞释放cpu,否则提前释放信号有概率花屏
    os_sem_create(&mcu_lcd_physics_flush_sem, 0);
    os_sem_create(&mcu_lcd_physics_flush_end_sem, 1);
    thread_fork("awtk_mcu_lcd_flush_task", 29, 200, 0, NULL, awtk_mcu_lcd_physics_flush_task, lcd);
#endif

    return lcd;
}

#else

#include "lcd/lcd_mem_fragment.h"

lcd_t *platform_create_lcd(wh_t w, wh_t h)
{
    printf("awtk platform_create_lcd w=%d,h=%d \r\n", w, h);

    return lcd_mem_fragment_create(w, h);
}

#define LCD_FORMAT BITMAP_FMT_BGR565

#define pixel_from_rgb(r, g, b)                                                \
  ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))

#define pixel_from_rgba(r, g, b, a)                                            \
  ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))

#define pixel_to_rgba(p)                                                       \
  { (0xff & ((p >> 11) << 3)), (0xff & ((p >> 5) << 2)), (0xff & (p << 3)) }

static void _lcd_draw_bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, pixel_t *img)
{
//    printf("draw x=%d, y=%d, w=%d, h=%d ",x, y, w, h);

    uint8_t *buf = img;
    for (int i = 0; i < w * h; i++) { //大小端
        pixel_t color = img[i] ;
        buf[2 * i] = (color >> 8) & 0xff;
        buf[2 * i + 1] = color & 0xff;
    }

    lcd_lvgl_full(x, x + w - 1, y, y + h - 1, img);
}

#define lcd_draw_bitmap_impl(x, y, w, h, p) _lcd_draw_bitmap(x, y, w, h, (unsigned char*)p)
#include "base/pixel.h"
#include "blend/pixel_ops.inc"
#include "lcd/lcd_mem_fragment.inc"
#endif
