/**
 * file:   main_loop_stm32_raw.c
 * author: li xianjing <xianjimli@hotmail.com>
 * brief:  main loop for stm32
 *
 * copyright (c) 2018 - 2018 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * license file for more details.
 *
 */

/**
 * history:
 * ================================================================
 * 2018-05-11 li xianjing <xianjimli@hotmail.com> created
 *
 */


#include "main_loop/main_loop_simple.h"
#include "awtk.h"

static ret_t platform_disaptch_touch_events(main_loop_t *loop)
{
    uint16_t x, y;
    uint8_t pressed;

    get_touch_x_y_status(&x, &y, &pressed);

    main_loop_post_pointer_event(loop, pressed, x, y);


    return RET_OK;
}

static ret_t platform_disaptch_key_events(main_loop_t *loop)
{

//    main_loop_post_key_event(loop, TRUE, TK_KEY_TAB);

    return RET_OK;
}

static ret_t platform_disaptch_input(main_loop_t *loop)
{
    platform_disaptch_key_events(loop);
    platform_disaptch_touch_events(loop);

    return RET_OK;
}

#include "main_loop/main_loop_raw.inc"












static void systick_test(void)
{
    int64_t start = get_time_ms64();
    sleep_ms(1000);
    int64_t end = get_time_ms64();
    int64_t duration = end - start;

    assert(duration == 1000);
}

static void awtk_thread(tk_thread_t *args)
{
    /* sqlite3_demo(CONFIG_ROOT_PATH"data/test.db"); */

    gui_app_start(LCD_W, LCD_H);
}

static ret_t awtk_start_ui_thread(void)
{

    tk_thread_t *ui_thread = tk_thread_create(awtk_thread, NULL);
    return_value_if_fail(ui_thread != NULL, RET_BAD_PARAMS);

    tk_thread_set_priority(ui_thread, TK_THREAD_PRIORITY_NORMAL);
    tk_thread_set_name(ui_thread, "awtk");
    tk_thread_set_stack_size(ui_thread, 580 * 1024);

    return tk_thread_start(ui_thread);
}

void awtk_test(void)
{
#if 0
//    systick_test();

    platform_prepare();

    system_info_init(APP_MOBILE, "app", NULL);

    lcd_t *lcd = platform_create_lcd(LCD_W, LCD_H);

    rect_t r = rect_init(0, 0, 100, 100);

    color_t red = color_init(0xff, 0, 0, 0xff);
    color_t green  = color_init(0, 0xff, 0, 0xff);
    color_t blue = color_init(0, 0, 0xff, 0xff);


    dirty_rects_t tmp_dirty_rects;
    dirty_rects_init(&(tmp_dirty_rects));
    dirty_rects_add(&(tmp_dirty_rects), (const rect_t *)&r);


    do {

        lcd_begin_frame(lcd, &tmp_dirty_rects, LCD_DRAW_NORMAL);

        lcd_set_fill_color(lcd, red);
        lcd_fill_rect(lcd, 0, 0, 60, 20);

        lcd_set_fill_color(lcd, green);
        lcd_fill_rect(lcd, 60, 20, 20, 20);

        lcd_set_fill_color(lcd, blue);
        lcd_fill_rect(lcd, 80, 40, 20, 60);

        lcd_end_frame(lcd);
    } while (0);
#else
    platform_prepare();
    awtk_start_ui_thread();

#endif
}
