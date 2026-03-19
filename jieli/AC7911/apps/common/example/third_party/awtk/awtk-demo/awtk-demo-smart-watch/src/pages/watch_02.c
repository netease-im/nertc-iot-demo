#include "awtk.h"
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"

static ret_t on_open_watch_select(void *ctx, event_t *e)
{
    return navigator_replace_with_context("watch_select", "watch_02");
}

static ret_t on_open_message(void *ctx, event_t *e)
{
    return navigator_to("message");
}

static ret_t on_open_app_list(void *ctx, event_t *e)
{
    return navigator_to("app_list");
}

static ret_t on_open_shortcut(void *ctx, event_t *e)
{
    return navigator_to("quick");
}

static ret_t on_open_wathc_03(void *ctx, event_t *e)
{
    return navigator_replace("watch_04");
}

static ret_t on_update_data(const timer_info_t *timer)
{
    widget_t *win = WIDGET(timer->ctx);
    widget_invalidate(win, NULL);

    return RET_REPEAT;
}

static ret_t on_paint_second(void *ctx, event_t *e)
{
    system_info_t *sys_info = system_info();
    widget_t *widget = WIDGET(e->target);
    widget_t *win = WIDGET(ctx);
    paint_event_t *evt = paint_event_cast(e);
    canvas_t *c = evt->c;
    vgcanvas_t *vg = canvas_get_vgcanvas(c);
    float_t r1 = widget->w / 2;
    float_t r2 = r1 - (win->w == 390 ? 7 : 4);
    uint32_t i = 0;
    date_time_t dt;

    date_time_init(&dt);

    vgcanvas_save(vg);
    vgcanvas_set_line_width(vg, 0);
    vgcanvas_set_stroke_color(vg, color_init(0xFF, 0xFF, 0xFF, 0xFF));
    vgcanvas_set_fill_color(vg, color_init(0xFF, 0xFF, 0xFF, 0xFF));
    vgcanvas_set_line_width(vg, 2);
    vgcanvas_translate(vg, win->x + win->w / 2, win->y + win->h / 2);

    for (i = 0; i <= dt.second; i++) {
        float_t a = i / 60.0 * 2 * M_PI - 0.5 * M_PI;
        float_t x1 = r1 * cos(a);
        float_t y1 = r1 * sin(a);
        float_t x2 = r2 * cos(a);
        float_t y2 = r2 * sin(a);

        vgcanvas_begin_path(vg);
        vgcanvas_move_to(vg, x1, y1);
        vgcanvas_line_to(vg, x2, y2);
        vgcanvas_stroke(vg);
    }

    vgcanvas_restore(vg);

    return RET_OK;
}

/**
 * 初始化窗口的子控件
 */
static ret_t visit_init_child(void *ctx, const void *iter)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = WIDGET(iter);
    const char *name = widget->name;

    // 初始化指定名称的控件（设置属性或注册事件），请保证控件名称在窗口上唯一
    if (name != NULL && *name != '\0') {
        if (tk_str_eq(name, "gesture")) {
            widget_on(widget, EVT_LONG_PRESS, on_open_watch_select, win);
            widget_on(widget, EVT_SLIDE_DOWN, on_open_message, win);
            widget_on(widget, EVT_SLIDE_LEFT, on_open_app_list, win);
            widget_on(widget, EVT_SLIDE_UP, on_open_shortcut, win);
            widget_on(widget, EVT_SLIDE_RIGHT, on_open_wathc_03, win);
        } else if (tk_str_eq(name, "canvas")) {
            widget_on(widget, EVT_PAINT, on_paint_second, win);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t watch_02_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);
    widget_add_timer(win, on_update_data, 1000);

    return RET_OK;
}

