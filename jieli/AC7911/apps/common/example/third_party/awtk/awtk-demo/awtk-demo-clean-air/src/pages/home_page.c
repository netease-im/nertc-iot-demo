#include "awtk.h"
#include "../common/navigator.h"

/**
 * 是否使能背景淡出淡入
 */
#define WITH_BKGND_FADE

/**
 * 是否使能风扇动画
 */
#define WITH_WIND_ANIM

/**
 * 是否使能报警图标动画
 */
//#define WITH_ALARM_ANIM

/**
 * 背景(主题)数量
 */
#define BKGND_IMG_CNT 2

/**
 * Label文本的数值 + offset
 */
static ret_t label_add(widget_t *label, int32_t offset)
{
    if (label) {
        int32_t val = 0;
        if (wstr_to_int(&(label->text), &val) == RET_OK) {
            char text[32];
            val += offset;
            val = tk_max(-200, tk_min(val, 200));
            tk_snprintf(text, sizeof(text), "%d", val);
            widget_set_text_utf8(label, text);

            return RET_OK;
        }
    }
    return RET_FAIL;
}

/**
 * Label文本显示随机int
 */
static void label_set_random_int(widget_t *label, int32_t min_num, int32_t max_num)
{
    if (label) {
        char text[32];
        int32_t val = fmod(rand(), max_num - min_num) + min_num;
        widget_set_text_utf8(label, tk_itoa(text, sizeof(text), val));
    }
}

/**
 * Label文本显示随机double, 并返回是否超出报警范围
 */
static bool_t label_set_random_double(widget_t *label, double min_num, double max_num,
                                      double alarm)
{
    if (label) {
        char text[64];
        double val = fmod(rand(), max_num - min_num) + min_num;
        tk_snprintf(text, sizeof(text), "%.1f", val);
        widget_set_text_utf8(label, text);

        return (val >= alarm);
    }
    return FALSE;
}

/**
 * 主题背景切换
 */
static void bkgnd_change(widget_t *widget)
{
    if (widget) {
        char style[5];
        value_t val;
        value_set_uint32(&val, 0);
        if (widget_get_prop(widget, "style_id", &val) != RET_OK || value_uint32(&val) == 0) {
            value_set_uint32(&val, 1);
        } else {
            value_set_uint32(&val, (value_uint32(&val) + 1) % BKGND_IMG_CNT);
        }
        widget_set_prop(widget, "style_id", &val);

        tk_snprintf(style, sizeof(style), "bg%u", value_uint32(&val));
        widget_use_style(widget, style);
    }
}

/**
 * 主题背景延迟淡入
 */
static ret_t on_bkgnd_delay_in(const timer_info_t *timer)
{
    widget_t *bkgnd = WIDGET(timer->ctx);

    if (bkgnd != NULL) {
        widget_start_animator(bkgnd, "fade_in");
        bkgnd_change(bkgnd);
    }

    return RET_REMOVE;
}

/**
 * 更新主题背景(淡出淡入)
 */
static ret_t on_bkgnd_update(const timer_info_t *timer)
{
    widget_t *win = WIDGET(timer->ctx);
    widget_t *bkgnd = widget_lookup(win, "bkgnd", TRUE);

    if (bkgnd) {
#ifdef WITH_BKGND_FADE
        widget_start_animator(bkgnd, "fade_out");
        widget_add_timer(bkgnd, on_bkgnd_delay_in, 500);
#else
        bkgnd_change(bkgnd);
#endif
    }

    return RET_REPEAT;
}

/**
 * 启动风扇(启动风扇、风向箭头的动画)
 */
static void on_wind_on(widget_t *win)
{
    widget_t *wind_in = widget_lookup(win, "wind_in", TRUE);
    widget_t *wind_out = widget_lookup(win, "wind_out", TRUE);
    widget_t *fan_1 = widget_lookup(win, "fan_1", TRUE);
    widget_t *fan_2 = widget_lookup(win, "fan_2", TRUE);
    widget_animator_t *animator;

#ifdef WITH_WIND_ANIM
    widget_start_animator(wind_in, "opacity");
    widget_start_animator(wind_out, "opacity");
    widget_start_animator(fan_2, "rotation");
#else
    (void)wind_in;
    (void)wind_out;
    (void)fan_2;
#endif
    image_animation_play(fan_1);
}

/**
 * 停止风扇(启动风扇、风向箭头的动画)
 */
static void on_wind_off(widget_t *win)
{
    widget_t *wind_in = widget_lookup(win, "wind_in", TRUE);
    widget_t *wind_out = widget_lookup(win, "wind_out", TRUE);
    widget_t *fan_1 = widget_lookup(win, "fan_1", TRUE);
    widget_t *fan_2 = widget_lookup(win, "fan_2", TRUE);
#ifdef WITH_WIND_ANIM
    widget_stop_animator(wind_in, "opacity");
    widget_stop_animator(wind_out, "opacity");
    widget_stop_animator(fan_2, "rotation");
#else
    (void)wind_in;
    (void)wind_out;
    (void)fan_2;
#endif
    image_animation_pause(fan_1);
}

/**
 * 显示报警指示器(报警图标闪烁)
 */
static void on_alarm_on(widget_t *win)
{
    widget_t *alarm = widget_lookup(win, "alarm_status", TRUE);
#ifdef WITH_ALARM_ANIM
    widget_start_animator(alarm, "opacity");
#endif
    widget_set_visible(alarm, TRUE, FALSE);
}

/**
 * 关闭报警指示器(报警图标隐藏)
 */
static void on_alarm_off(widget_t *win)
{
    widget_t *alarm = widget_lookup(win, "alarm_status", TRUE);
#ifdef WITH_ALARM_ANIM
    widget_stop_animator(alarm, "opacity");
#endif
    widget_set_visible(alarm, FALSE, FALSE);
}

/**
 * 模拟温度、湿度等读数变化(随机数值), 并判断温度是否超过报警值，超过则报警
 */
static ret_t on_reading_update(const timer_info_t *timer)
{
    bool_t alarm = FALSE;
    widget_t *win = WIDGET(timer->ctx);
    widget_t *fan1 = widget_lookup(win, "fan1", TRUE);
    widget_t *fan2 = widget_lookup(win, "fan2", TRUE);
    widget_t *co_2 = widget_lookup(win, "CO_2", TRUE);
    widget_t *flow = widget_lookup(win, "flow", TRUE);
    widget_t *pm2_5 = widget_lookup(win, "PM2_5", TRUE);
    widget_t *temperature_out1 = widget_lookup(win, "temperature_out1", TRUE);
    widget_t *temperature_out2 = widget_lookup(win, "temperature_out2", TRUE);

    label_set_random_int(fan1, 80, 85);
    label_set_random_int(fan2, 80, 82);
    label_set_random_int(pm2_5, 18, 20);
    label_set_random_int(co_2, 350, 352);
    label_set_random_int(flow, 500, 505);

    if (label_set_random_double(temperature_out1, 20.5, 22.8, 22.0)) {
        widget_use_style(temperature_out1, "reading_temp_hum_alarm");
        alarm = TRUE;
    } else {
        widget_use_style(temperature_out1, "reading_temp_hum_black");
    }

    if (label_set_random_double(temperature_out2, 20.5, 22.8, 22.0)) {
        widget_use_style(temperature_out2, "reading_temp_hum_alarm");
        alarm = TRUE;
    } else {
        widget_use_style(temperature_out2, "reading_temp_hum_black");
    }

    if (alarm) {
        on_alarm_on(win);
    } else {
        on_alarm_off(win);
    }

    return RET_REPEAT;
}

/**
 * 启动/关闭设备(开始/停止模拟温度、湿度等读数变化, 风扇动画)
 */
static ret_t on_switch(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *btn = widget_lookup(win, "switch", TRUE);
    value_t val;
    value_set_uint32(&val, 0);
    if (widget_get_prop(btn, "timer_id", &val) != RET_OK || value_uint32(&val) == 0) {
        value_set_uint32(&val, timer_add(on_reading_update, win, 1000));
        on_wind_on(win);
    } else {
        timer_remove(value_uint32(&val));
        on_wind_off(win);
        value_set_uint32(&val, 0);
    }
    widget_set_prop(btn, "timer_id", &val);

    return RET_OK;
}

/**
 * 开始/停止背景切换(10s一次)
 */
static ret_t on_auto(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    uint32_t id = widget_get_prop_int(win, "timer_id", TK_INVALID_ID);

    if (id == TK_INVALID_ID) {
        id = widget_add_timer(win, on_bkgnd_update, 10000);
    } else {
        widget_remove_timer(win, id);
    }

    widget_set_prop_int(win, "timer_id", id);

    return RET_OK;
}

/**
 * 弹出定时窗口
 */
static ret_t on_timing(void *ctx, event_t *e)
{
    (void)e;
    time_t t;
    widget_t *win = WIDGET(ctx);
    widget_t *timing = widget_lookup(win, "timing_status", TRUE);
    if (navigator_to("timing") == RET_OK) {
        widget_set_visible(timing, TRUE, FALSE);
    } else {
        widget_set_visible(timing, FALSE, FALSE);
    }
    return RET_OK;
}

/**
 * 弹出记录窗口
 */
static ret_t on_record_click(void *ctx, event_t *e)
{
    return navigator_to("record");
}

/**
 * 弹出设置窗口
 */
static ret_t on_setting_click(void *ctx, event_t *e)
{
    return navigator_to("setting");
}

/**
 * 频率+1
 */
static ret_t on_frequency_inc(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *label = widget_lookup(win, "frequency", TRUE);
    (void)e;

    return label_add(label, 1);
}

/**
 * 频率-1
 */
static ret_t on_frequency_dec(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *label = widget_lookup(win, "frequency", TRUE);
    (void)e;

    return label_add(label, -1);
}

/**
 * 温度+1
 */
static ret_t on_temperature_inc(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *label = widget_lookup(win, "temperature", TRUE);
    (void)e;

    return label_add(label, 1);
}

/**
 * 温度-1
 */
static ret_t on_temperature_dec(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *label = widget_lookup(win, "temperature", TRUE);
    (void)e;

    return label_add(label, -1);
}

/**
 * 湿度+1
 */
static ret_t on_humidity_inc(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *label = widget_lookup(win, "humidity", TRUE);
    (void)e;

    return label_add(label, 1);
}

/**
 * 湿度-1
 */
static ret_t on_humidity_dec(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *label = widget_lookup(win, "humidity", TRUE);
    (void)e;

    return label_add(label, -1);
}

/**
 * 点击中英文互译按钮
 */
static ret_t on_language_btn_click(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    const char *language = locale_info()->language;
    if (tk_str_eq(language, "en")) {
        locale_info_change(locale_info(), "zh", "CN");
    } else {
        locale_info_change(locale_info(), "en", "US");
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
        if (tk_str_eq(name, "language_btn")) {
            widget_on(widget, EVT_CLICK, on_language_btn_click, win);
        } else if (tk_str_eq(name, "switch")) {
            widget_on(widget, EVT_CLICK, on_switch, win);
        } else if (tk_str_eq(name, "auto")) {
            widget_on(widget, EVT_CLICK, on_auto, win);
        } else if (tk_str_eq(name, "timing")) {
            widget_on(widget, EVT_CLICK, on_timing, win);
        } else if (tk_str_eq(name, "frequency_inc")) {
            widget_on(widget, EVT_CLICK, on_frequency_inc, win);
        } else if (tk_str_eq(name, "frequency_dec")) {
            widget_on(widget, EVT_CLICK, on_frequency_dec, win);
        } else if (tk_str_eq(name, "temperature_inc")) {
            widget_on(widget, EVT_CLICK, on_temperature_inc, win);
        } else if (tk_str_eq(name, "temperature_dec")) {
            widget_on(widget, EVT_CLICK, on_temperature_dec, win);
        } else if (tk_str_eq(name, "humidity_inc")) {
            widget_on(widget, EVT_CLICK, on_humidity_inc, win);
        } else if (tk_str_eq(name, "humidity_dec")) {
            widget_on(widget, EVT_CLICK, on_humidity_dec, win);
        } else if (tk_str_eq(name, "record")) {
            widget_on(widget, EVT_CLICK, on_record_click, win);
        } else if (tk_str_eq(name, "setting")) {
            widget_on(widget, EVT_CLICK, on_setting_click, win);
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

    locale_info_change(locale_info(), "zh", "CN");
    widget_foreach(win, visit_init_child, win);
    on_switch(win, NULL);
    on_auto(win, NULL);

    return RET_OK;
}
