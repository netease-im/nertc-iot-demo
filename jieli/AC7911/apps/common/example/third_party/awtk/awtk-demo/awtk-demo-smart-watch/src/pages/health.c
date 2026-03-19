#include "awtk.h"
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"
#include "chart_view/bar_series.h"

static float_t coordinate_to_radian(point_t point, point_t c)
{
    float_t radian = 0;

    if (point.x == c.x) {
        return point.y < c.y ? 0 : M_PI;
    } else if (point.y == c.y) {
        return point.x < c.x ? 3 * M_PI / 2 : M_PI / 2;
    }

    if (point.x > c.x) {
        if (point.y < c.y) {
            radian = atan(((float_t)point.x - c.x) / (c.y - point.y));
        } else {
            radian = atan(((float_t)point.y - c.y) / (point.x - c.x)) + M_PI / 2;
        }
    } else {
        if (point.y > c.y) {
            radian = atan(((float_t)c.x - point.x) / (point.y - c.y)) + M_PI;
        } else {
            radian = atan(((float_t)c.y - point.y) / (c.x - point.x)) + 3 * M_PI / 2;
        }
    }

    return radian;
}

static to_angle(point_t start_point, point_t end_point, point_t c)
{
    float_t rs = coordinate_to_radian(start_point, c);
    float_t re = coordinate_to_radian(end_point, c);

    float_t r = re - rs;
    return TK_R2D(r);
}

static float_t s_curr_activity = 0;
static float_t s_curr_train = 0;
static float_t s_curr_stand = 0;
static float_t s_sum_activity = 0;
static float_t s_sum_train = 0;
static float_t s_sum_stand = 0;
static float_t s_setted_activity = 1000;
static float_t s_setted_train = 1000;
static float_t s_setted_stand = 1000;

static ret_t on_set_begin(void *ctx, event_t *e)
{
    pointer_event_t *evt = (pointer_event_t *)e;
    widget_t *widget = WIDGET(e->target);
    point_t p1 = {evt->x, evt->y};
    point_t p2 = {widget->w / 2, 0};
    point_t c = {widget->w / 2, widget->h / 2};
    float_t r1 = tk_min(c.x, c.y);    // 外半径
    float_t r2 = r1 - widget_get_prop_int(widget, PROGRESS_CIRCLE_PROP_LINE_WIDTH, 0);    // 内半径
    float_t l;

    widget_to_local(widget, &p1);

    l = (p1.x - c.x) * (p1.x - c.x) + (p1.y - c.y) * (p1.y - c.y);
    if (l <= r1 * r1 && l >= r2 * r2) {
        widget_grab(widget_get_window(widget), widget);
        widget_set_prop_bool(widget, "dragging", TRUE);
        int32_t max = widget_get_prop_int(widget, "max", 0);
        widget_set_value(widget, to_angle(p2, p1, c)  * max / 360);

        return RET_STOP;
    }

    return RET_OK;
}

static ret_t on_setting(void *ctx, event_t *e)
{
    pointer_event_t *evt = (pointer_event_t *)e;
    widget_t *widget = WIDGET(e->target);
    point_t p1 = {evt->x, evt->y};
    point_t p2 = {widget->w / 2, 0};
    point_t c = {widget->w / 2, widget->h / 2};

    widget_to_local(widget, &p1);

    if (widget_get_prop_bool(widget, "dragging", FALSE)) {
        int32_t max = widget_get_prop_int(widget, "max", 0);
        widget_set_value(widget, to_angle(p2, p1, c)  * max / 360);
    }

    return RET_OK;
}

static ret_t on_set_end(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(e->target);

    widget_set_prop_bool(widget, "dragging", FALSE);
    widget_ungrab(widget_get_window(widget), widget);
    widget_invalidate(widget, NULL);

    return RET_OK;
}

static ret_t on_update_setted_activity(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *target = WIDGET(e->target);

    widget_t *widget = widget_lookup(win, "activity", TRUE);
    if (widget != NULL) {
        char text[10] = {0};
        s_setted_activity = widget_get_value(target);
        tk_itoa(text, 10, (uint32_t)s_setted_activity);
        widget_set_text_utf8(widget, text);
    }

    widget = widget_lookup(win, "sum_activity_prog", TRUE);
    if (widget != NULL) {
        widget_set_prop_int(widget, "max", s_setted_activity);
    }

    return RET_OK;
}

static ret_t on_update_setted_train(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *target = WIDGET(e->target);

    widget_t *widget = widget_lookup(win, "train", TRUE);
    if (widget != NULL) {
        char text[10] = {0};
        s_setted_train = widget_get_value(target);
        tk_itoa(text, 10, (uint32_t)s_setted_train);
        widget_set_text_utf8(widget, text);
    }

    widget = widget_lookup(win, "sum_train_prog", TRUE);
    if (widget != NULL) {
        widget_set_prop_int(widget, "max", s_setted_train);
    }

    return RET_OK;
}

static ret_t on_update_setted_stand(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *target = WIDGET(e->target);

    widget_t *widget = widget_lookup(win, "stand", TRUE);
    if (widget != NULL) {
        char text[10] = {0};
        s_setted_stand = widget_get_value(target);
        tk_itoa(text, 10, (uint32_t)s_setted_stand);
        widget_set_text_utf8(widget, text);
    }

    widget = widget_lookup(win, "sum_stand_prog", TRUE);
    if (widget != NULL) {
        widget_set_prop_int(widget, "max", s_setted_stand);
    }

    return RET_OK;
}

static ret_t on_back(void *ctx, event_t *e)
{
    return navigator_back();
}

static ret_t on_update_health_data(const timer_info_t *timer)
{
    char text[10] = {0};
    widget_t *win = WIDGET(timer->ctx);
    widget_t *widget;

    s_curr_activity = random() % 60;
    s_curr_train = random() % 60;
    s_curr_stand = random() % 60;
    s_sum_activity += s_curr_activity;
    s_sum_train += s_curr_train;
    s_sum_stand += s_curr_stand;

    widget = widget_lookup(win, "activity_series", TRUE);
    if (widget != NULL) {
        series_push(widget, &s_curr_activity, 1);
    }

    widget = widget_lookup(win, "train_series", TRUE);
    if (widget != NULL) {
        series_push(widget, &s_curr_train, 1);
    }

    widget = widget_lookup(win, "stand_series", TRUE);
    if (widget != NULL) {
        series_push(widget, &s_curr_stand, 1);
    }

    widget = widget_lookup(win, "sum_activity_prog", TRUE);
    if (widget != NULL) {
        widget_set_value(widget, s_sum_activity);
        // widget_set_prop_int(widget, "max", s_setted_activity);
    }

    widget = widget_lookup(win, "sum_train_prog", TRUE);
    if (widget != NULL) {
        widget_set_value(widget, s_sum_train);
        // widget_set_prop_int(widget, "max", s_setted_train);
    }

    widget = widget_lookup(win, "sum_stand_prog", TRUE);
    if (widget != NULL) {
        widget_set_value(widget, s_sum_stand);
        // widget_set_prop_int(widget, "max", s_setted_stand);
    }

    widget = widget_lookup(win, "sum_activity_val", TRUE);
    if (widget != NULL) {
        tk_itoa(text, 10, (uint32_t)s_sum_activity);
        widget_set_text_utf8(widget, text);
    }

    widget = widget_lookup(win, "sum_train_val", TRUE);
    if (widget != NULL) {
        tk_itoa(text, 10, (uint32_t)s_sum_train);
        widget_set_text_utf8(widget, text);
    }

    widget = widget_lookup(win, "sum_stand_val", TRUE);
    if (widget != NULL) {
        tk_itoa(text, 10, (uint32_t)s_sum_stand);
        widget_set_text_utf8(widget, text);
    }

    return RET_REPEAT;
}

/**
 * 初始化窗口的子控件
 */
static ret_t visit_init_child(void *ctx, const void *iter)
{
    char text[10] = {0};
    widget_t *win = WIDGET(ctx);
    widget_t *widget = WIDGET(iter);
    const char *name = widget->name;

    // 初始化指定名称的控件（设置属性或注册事件），请保证控件名称在窗口上唯一
    if (name != NULL && *name != '\0') {
        if (tk_str_eq(name, "gesture")) {
            widget_on(widget, EVT_SLIDE_RIGHT, on_back, win);
        } else if (tk_str_eq(name, "activity_series")) {
            series_set(widget, 0, &s_curr_activity, 1);
        } else if (tk_str_eq(name, "train_series")) {
            series_set(widget, 0, &s_curr_train, 1);
        } else if (tk_str_eq(name, "stand_series")) {
            series_set(widget, 0, &s_curr_stand, 1);
        } else if (tk_str_eq(name, "sum_activity_prog")) {
            widget_set_value(widget, s_sum_activity);
        } else if (tk_str_eq(name, "sum_train_prog")) {
            widget_set_value(widget, s_sum_train);
        } else if (tk_str_eq(name, "sum_stand_prog")) {
            widget_set_value(widget, s_sum_stand);
        } else if (tk_str_eq(name, "sum_activity_val")) {
            tk_snprintf(text, sizeof(text) - 1, "%d", (uint32_t)s_sum_activity);
            widget_set_text_utf8(widget, text);
        } else if (tk_str_eq(name, "sum_train_val")) {
            tk_snprintf(text, sizeof(text) - 1, "%d", (uint32_t)s_sum_train);
            widget_set_text_utf8(widget, text);
        } else if (tk_str_eq(name, "sum_stand_val")) {
            tk_snprintf(text, sizeof(text) - 1, "%d", (uint32_t)s_sum_stand);
            widget_set_text_utf8(widget, text);
        } else if (tk_str_eq(name, "activity_set")) {
            widget_set_value(widget, s_setted_activity);
            widget_on(widget, EVT_VALUE_CHANGED, on_update_setted_activity, win);
            widget_on(widget, EVT_POINTER_DOWN, on_set_begin, win);
            widget_on(widget, EVT_POINTER_MOVE, on_setting, win);
            widget_on(widget, EVT_POINTER_UP, on_set_end, win);
        } else if (tk_str_eq(name, "train_set")) {
            widget_set_value(widget, s_setted_train);
            widget_on(widget, EVT_VALUE_CHANGED, on_update_setted_train, win);
            widget_on(widget, EVT_POINTER_DOWN, on_set_begin, win);
            widget_on(widget, EVT_POINTER_MOVE, on_setting, win);
            widget_on(widget, EVT_POINTER_UP, on_set_end, win);
        } else if (tk_str_eq(name, "stand_set")) {
            widget_set_value(widget, s_setted_stand);
            widget_on(widget, EVT_VALUE_CHANGED, on_update_setted_stand, win);
            widget_on(widget, EVT_POINTER_DOWN, on_set_begin, win);
            widget_on(widget, EVT_POINTER_MOVE, on_setting, win);
            widget_on(widget, EVT_POINTER_UP, on_set_end, win);
        } else if (tk_str_eq(name, "activity")) {
            tk_itoa(text, 10, (uint32_t)s_setted_activity);
            widget_set_text_utf8(widget, text);
        } else if (tk_str_eq(name, "train")) {
            tk_itoa(text, 10, (uint32_t)s_setted_train);
            widget_set_text_utf8(widget, text);
        } else if (tk_str_eq(name, "stand")) {
            tk_itoa(text, 10, (uint32_t)s_setted_stand);
            widget_set_text_utf8(widget, text);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t health_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    s_sum_activity = s_curr_activity = random() % 100;
    s_sum_train = s_curr_train = random() % 100;
    s_sum_stand = s_curr_stand = random() % 100;

    widget_foreach(win, visit_init_child, win);
    widget_add_timer(win, on_update_health_data, 2000);

    return RET_OK;
}
