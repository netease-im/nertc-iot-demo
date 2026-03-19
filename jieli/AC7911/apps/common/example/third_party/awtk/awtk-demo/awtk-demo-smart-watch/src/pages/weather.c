#include "awtk.h"
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"

static void set_random_int(widget_t *label, int32_t min_num, int32_t max_num, char *unit)
{
    if (label) {
        char text[32] = {0};
        int32_t val = fmod(rand(), max_num - min_num) + min_num;
        if (unit) {
            tk_snprintf(text, sizeof(text), "%d%s", val, unit);
        } else {
            tk_snprintf(text, sizeof(text), "%d", val);
        }

        widget_set_text_utf8(label, text);
    }
}

static void set_random_int_wstr(widget_t *label, int32_t min_num, int32_t max_num, wchar_t *unit)
{
    if (label) {
        char text[32] = {0};
        (void)text;
        int32_t val = fmod(rand(), max_num - min_num) + min_num;

        wstr_t result;
        wstr_init(&result, 0);

        char val_text[4];
        memset(val_text, '\0', sizeof(val_text));
        tk_snprintf(val_text, sizeof(val_text), "%d", val);
        wstr_set_utf8(&result, val_text);

        wstr_append(&result, unit);

        widget_set_text(label, result.str);
        wstr_reset(&result);
    }
}

/**
 * 定时刷新天气数据
 */
static ret_t update_weather_data(widget_t *win)
{
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_t *temp_now = widget_lookup(win, "weather:temp", TRUE);
    widget_t *temp_min = widget_lookup(win, "weather:temp_min", TRUE);
    widget_t *temp_max = widget_lookup(win, "weather:temp_max", TRUE);
    widget_t *aqi = widget_lookup(win, "weather:aqi", TRUE);
    widget_t *temp_n_min = widget_lookup(win, "weather:temp_n_min", TRUE);
    widget_t *temp_n_max = widget_lookup(win, "weather:temp_n_max", TRUE);
    widget_t *temp_nn_min = widget_lookup(win, "weather:temp_nn_min", TRUE);
    widget_t *temp_nn_max = widget_lookup(win, "weather:temp_nn_max", TRUE);
    widget_t *update_time = widget_lookup(win, "weather:update", TRUE);
    widget_t *wind = widget_lookup(win, "weather:wind", TRUE);
    widget_t *uvi = widget_lookup(win, "weather:uvi", TRUE);
    widget_t *humi = widget_lookup(win, "weather:humi", TRUE);
    widget_t *temp_feel = widget_lookup(win, "weather:temp_feel", TRUE);
    widget_t *air_p = widget_lookup(win, "weather:air_p", TRUE);

    set_random_int_wstr(temp_now, 20, 30, L"°");
    set_random_int(aqi, 10, 400, NULL);
    set_random_int_wstr(temp_min, 10, 20, L"°");
    set_random_int_wstr(temp_max, 30, 35, L"°");
    set_random_int_wstr(temp_n_min, 10, 20, L"°");
    set_random_int_wstr(temp_n_max, 30, 35, L"°");
    set_random_int_wstr(temp_nn_min, 10, 20, L"°");
    set_random_int_wstr(temp_nn_max, 30, 35, L"°");
    set_random_int(wind, 1, 15, NULL);
    set_random_int(uvi, 1, 12, NULL);
    set_random_int(humi, 64, 100, "%");
    set_random_int_wstr(temp_feel, 15, 35, L"°");
    set_random_int(air_p, 900, 1200, "hPa");

    date_time_t date;
    date_time_init(&date);
    char text[32] = {0};
    tk_snprintf(text, sizeof(text), "%02d:%02d", date.hour, date.minute);
    widget_set_text_utf8(update_time, text);

    return RET_OK;
}

static ret_t on_update_weather_timer(const timer_info_t *timer)
{
    widget_t *win = WIDGET(timer->ctx);

    update_weather_data(win);

    return RET_REPEAT;
}

static ret_t on_gesture_slide_right(void *ctx, event_t *e)
{
    return navigator_back();
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
            widget_on(widget, EVT_SLIDE_RIGHT, on_gesture_slide_right, win);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t weather_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);

    update_weather_data(win);
    widget_add_timer(win, on_update_weather_timer, 2000);

    return RET_OK;
}
