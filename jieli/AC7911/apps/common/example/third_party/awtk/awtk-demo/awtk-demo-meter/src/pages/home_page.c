#include "awtk.h"
#include "../common/navigator.h"

#define CENTER_POINT_RADIUS "center_point_radius"

/**
 * Label文本随机显示int
 */
static int32_t get_random_int(int32_t min_num, int32_t max_num)
{
    return fmod(rand(), max_num - min_num) + min_num;
}

/**
 * 更新LED灯的状态
 */
static ret_t on_update_led_status(const timer_info_t *timer)
{
    widget_t *win = WIDGET(timer->ctx);
    widget_t *label = NULL;
    char label_name[32] = {0};
    int32_t label_index = get_random_int(0, 4);

    tk_snprintf(label_name, sizeof(label_name) - 1, "label%d", label_index);

    label = widget_lookup(win, label_name, TRUE);
    if (label != NULL) {
        const char *led_styles[] = {"led_red", "led_green", "led_yellow", "led_gray"};
        int32_t style_index = get_random_int(0, 4);
        const char *old_style = widget_get_prop_str(label, WIDGET_PROP_STYLE, "led_green");
        const char *new_style = led_styles[style_index];

        if (!tk_str_eq(old_style, new_style)) {
            widget_use_style(label, new_style);
        }
    }

    return RET_REPEAT;
}

/**
 * 绘制表盘的指针
 */
static ret_t on_needle_paint(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    value_t v;
    paint_event_t *evt = (paint_event_t *)e;
    canvas_t *c = evt->c;
    widget_t *widget = WIDGET(evt->e.target);
    vgcanvas_t *vg = lcd_get_vgcanvas(c->lcd);

    float_t rotation = 0.0;
    float_t w = widget->w;
    float_t h = widget->h;
    float_t cx = w * 0.5f;
    float_t cy = h * 0.5f;
    float_t anchor_x = w * 0.5;
    float_t anchor_y = h * 0.5;
    float_t center_point_radius = 0.0;
    color_t bg = color_init(0xff, 0, 0, 0xff);

    if (widget_get_prop(widget, WIDGET_PROP_ROTATION, &v) == RET_OK) {
        rotation = value_float(&v);
    }

    if (widget_get_prop(widget, CENTER_POINT_RADIUS, &v) == RET_OK) {
        center_point_radius = value_float(&v);
    }

    if (vg != NULL) {
        vgcanvas_save(vg);
        vgcanvas_translate(vg, c->ox, c->oy);
        vgcanvas_translate(vg, anchor_x, anchor_y);
        vgcanvas_rotate(vg, rotation);
        vgcanvas_translate(vg, -anchor_x, -anchor_y);

        vgcanvas_begin_path(vg);
        vgcanvas_move_to(vg, cx, 0);
        vgcanvas_line_to(vg, cx, h * 0.6f);
        vgcanvas_set_line_width(vg, 4);
        vgcanvas_set_stroke_color(vg, bg);
        vgcanvas_stroke(vg);

        vgcanvas_begin_path(vg);
        vgcanvas_set_fill_color(vg, bg);
        vgcanvas_arc(vg, cx, cy, center_point_radius, 0, M_PI * 2, FALSE);
        vgcanvas_fill(vg);

        vgcanvas_restore(vg);
    }

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
        if (tk_str_eq(name, "needle")) {
            widget_on(widget, EVT_PAINT, on_needle_paint, win);
        }

    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t home_page_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);
    widget_add_timer(win, on_update_led_status, 1000);

    return RET_OK;
}
