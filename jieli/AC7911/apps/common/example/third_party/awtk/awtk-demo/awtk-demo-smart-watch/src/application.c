#include "awtk.h"
#include "common/navigator.h"
#include "custom_widgets/custom_widgets.h"
#include "chart_view_register.h"

#ifndef APP_SYSTEM_BAR
#define APP_SYSTEM_BAR ""
#endif /*APP_SYSTEM_BAR*/

#ifndef APP_BOTTOM_SYSTEM_BAR
#define APP_BOTTOM_SYSTEM_BAR ""
#endif /*APP_BOTTOM_SYSTEM_BAR*/

#ifndef APP_START_PAGE
#define APP_START_PAGE "watch_01"
#endif /*APP_START_PAGE*/

/**
 * 注册自定义控件
 */
static ret_t custom_widgets_register(void)
{
    chart_view_register();

    return RET_OK;
}

/**
 * 当程序初始化完成时调用，全局只触发一次。
 */
static ret_t application_on_launch(void)
{
    return RET_OK;
}

/**
 * 当程序退出时调用，全局只触发一次。
 */
static ret_t application_on_exit(void)
{
    return RET_OK;
}

static ret_t on_paint_fg(void *ctx, event_t *e)
{
    canvas_t *c = ((paint_event_t *)e)->c;
    bitmap_t img;
    widget_t *wm = WIDGET(ctx);
    widget_t *win = window_manager_get_top_window(wm);

    widget_load_image(wm, "ico_background2", &img);

    rect_t s = {0, 0, img.w, img.h};

    system_info_t *sys_info = system_info();
    uint32_t lcd_w = sys_info->lcd_w;
    uint32_t lcd_h = sys_info->lcd_h;

    if (win->w == 390) {
        rect_t d = {189, 0, img.w, img.h};
        canvas_draw_image(c, &img, &s, &d);
        canvas_set_fill_color_str(c, "black");
        canvas_fill_rect(c, 0, 0, 189, lcd_h);

        canvas_fill_rect(c, 629, 0, lcd_w - 629, lcd_h);
        canvas_fill_rect(c, 0, 480, lcd_w, lcd_h - 480);
    } else if (win->w == 240) {
        rect_t d = {(480 - 272) / 2 + 6, 0, img.w, img.h};
        canvas_draw_image(c, &img, &s, &d);
        canvas_set_fill_color_str(c, "black");
        canvas_fill_rect(c, 0, 0, (480 - 272) / 2 + 6, 272);
        canvas_fill_rect(c, 480 - (480 - 272) / 2 + 6, 0, (480 - 272) / 2 - 6, 272);
    } else {
        assert(FALSE);
    }

    return RET_OK;
}

/**
 * 初始化程序
 */
ret_t application_init(void)
{
    custom_widgets_register();
    application_on_launch();

    if (strlen(APP_SYSTEM_BAR) > 0) {
        navigator_to(APP_SYSTEM_BAR);
    }

    if (strlen(APP_BOTTOM_SYSTEM_BAR) > 0) {
        navigator_to(APP_BOTTOM_SYSTEM_BAR);
    }

    widget_on(window_manager(), EVT_AFTER_PAINT, on_paint_fg, window_manager());
    custom_widgets_init();

    return navigator_to(APP_START_PAGE);
}

/**
 * 退出程序
 */
ret_t application_exit(void)
{
    application_on_exit();
    log_debug("application_exit\n");

    return RET_OK;
}
