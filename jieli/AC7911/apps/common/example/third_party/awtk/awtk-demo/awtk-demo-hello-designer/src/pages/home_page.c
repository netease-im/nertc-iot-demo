#include "awtk.h"
#include "../common/navigator.h"

/**
 * 中英文互译
 */
static ret_t change_locale(const char *str)
{
    char country[3];
    char language[3];

    strncpy(language, str, 2);
    strncpy(country, str + 3, 2);
    locale_info_change(locale_info(), language, country);

    return RET_OK;
}

/**
 * 点击中英文互译按钮
 */
static ret_t on_language_btn_click(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *language_btn = WIDGET(e->target);
    (void)e;

    const char *language = locale_info()->language;
    if (tk_str_eq(language, "en")) {
        change_locale("zh_CN");
        widget_use_style(language_btn, "en");
    } else {
        change_locale("en_US");
        widget_use_style(language_btn, "zh");
    }

    return RET_OK;
}

static ret_t on_basic_btn_click(void *ctx, event_t *e)
{
    return navigator_to("basic");
}

static ret_t on_background_btn_click(void *ctx, event_t *e)
{
    return navigator_to("background");
}

static ret_t on_listview_btn_click(void *ctx, event_t *e)
{
    return navigator_to("listview");
}

static ret_t on_animator_btn_click(void *ctx, event_t *e)
{
    return navigator_to("animator");
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
            widget_on(widget, EVT_CLICK, on_language_btn_click, NULL);
        } else if (tk_str_eq(name, "basic_btn")) {
            widget_on(widget, EVT_CLICK, on_basic_btn_click, NULL);
        } else if (tk_str_eq(name, "background_btn")) {
            widget_on(widget, EVT_CLICK, on_background_btn_click, NULL);
        } else if (tk_str_eq(name, "listview_btn")) {
            widget_on(widget, EVT_CLICK, on_listview_btn_click, NULL);
        } else if (tk_str_eq(name, "animator_btn")) {
            widget_on(widget, EVT_CLICK, on_animator_btn_click, NULL);
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

    return RET_OK;
}
