#include "awtk.h"
#include "../common/navigator.h"
#include "chart_view/chart_view.h"
#include "chart_view/line_series.h"
#include "chart_view/bar_series.h"

#define SLIDER_COUNT 9
#define BUFF_LEN 32

static float_t val_slider[SLIDER_COUNT] = {50, 50, 50, 50, 50, 50, 50, 50, 50};
static int32_t g_evt_pointer_down_num;

static ret_t on_close(void *ctx, event_t *e)
{
    widget_t *dialog = (widget_t *)ctx;
    (void)e;

    return dialog_quit(dialog, RET_QUIT);
}

/**
 * 退出动画
 */
static ret_t on_quit_anim(void *ctx, event_t *e)
{
    widget_t *dialog = (widget_t *)ctx;
    widget_animator_t *animator = NULL;
    (void)e;
    widget_t *wm = window_manager();

    if (wm->w == 480) {
        widget_create_animator(dialog, "move(x_from=240, x_to=480, duration=500)");
    } else {
        widget_create_animator(dialog, "move(x_from=400, x_to=800, duration=500)");
    }
    animator = widget_find_animator(dialog, "move");
    widget_animator_on(animator, EVT_ANIM_END, on_close, dialog);
    widget_off(dialog, g_evt_pointer_down_num);

    return RET_OK;
}

static ret_t on_dialog_state(void *ctx, event_t *e)
{
    widget_t *dialog = (widget_t *)ctx;
    pointer_event_t evt = *(pointer_event_t *)e;
    widget_t *close = widget_lookup(dialog, "close", TRUE);
    if (evt.x > (dialog->x + dialog->w) || evt.x < dialog->x || evt.y < dialog->y ||
        evt.y > (dialog->y + dialog->h)) {
        evt.e = event_init(EVT_CLICK, close);
        evt.e.size = sizeof(evt);
        widget_dispatch(close, (event_t *)&evt);
        widget_invalidate(close, NULL);
    }

    return RET_OK;
}

static ret_t set_series_data(widget_t *widget, float_t *slider_line, uint32_t count)
{
    void *buffer = TKMEM_CALLOC(count, sizeof(float_t));
    float_t *b = (float_t *)buffer;

    for (int i = 0; i < count; i++) {
        b[i] = slider_line[i];
    }

    series_set(widget, 0, buffer, count);
    TKMEM_FREE(buffer);
    return RET_OK;
}

/**
 * char_view波形数值改变
 */
static ret_t on_slider_changing(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *child = WIDGET(e->target);
    char name_buf[BUFF_LEN] = {0};
    strcpy(name_buf, child->name);
    strtok(name_buf, "_");
    int32_t i = atoi(strtok(NULL, "_"));
    val_slider[i] = widget_get_value(child);

    widget_t *widget = widget_lookup(win, "s1", TRUE);
    if (widget != NULL) {
        set_series_data(widget, val_slider, SLIDER_COUNT);
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
        if (tk_str_eq(name, "close")) {
            widget_on(widget, EVT_CLICK, on_quit_anim, win);
        } else if (tk_str_eq(name, "frequency_0")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_slider_changing, win);
            slider_set_value(widget, val_slider[0]);
        } else if (tk_str_eq(name, "frequency_1")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_slider_changing, win);
            slider_set_value(widget, val_slider[1]);
        } else if (tk_str_eq(name, "frequency_2")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_slider_changing, win);
            slider_set_value(widget, val_slider[2]);
        } else if (tk_str_eq(name, "frequency_3")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_slider_changing, win);
            slider_set_value(widget, val_slider[3]);
        } else if (tk_str_eq(name, "frequency_4")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_slider_changing, win);
            slider_set_value(widget, val_slider[4]);
        } else if (tk_str_eq(name, "frequency_5")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_slider_changing, win);
            slider_set_value(widget, val_slider[5]);
        } else if (tk_str_eq(name, "frequency_6")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_slider_changing, win);
            slider_set_value(widget, val_slider[6]);
        } else if (tk_str_eq(name, "frequency_7")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_slider_changing, win);
            slider_set_value(widget, val_slider[7]);
        } else if (tk_str_eq(name, "frequency_8")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_slider_changing, win);
            slider_set_value(widget, val_slider[8]);
        } else if (tk_str_eq(name, "s1")) {
            set_series_data(widget, val_slider, SLIDER_COUNT);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t equalizer_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    g_evt_pointer_down_num = widget_on(win, EVT_POINTER_UP, on_dialog_state, win);
    widget_foreach(win, visit_init_child, win);

    return RET_OK;
}
