/**
 * File:   window_pie.c
 * Author: AWTK Develop Team
 * Brief:  window pie
 *
 * Copyright (c) 2018 - 2018  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-11-20 TangJunJie created
 *
 */

#include "awtk.h"
#include "tkc/date_time.h"
#include "../src/chart_view/chart_view.h"
#include "../src/chart_view/line_series.h"
#include "../src/chart_view/series.h"

#define SERIES_TIMER_UPDATE 1

extern ret_t on_series_rset_rand_ufloat_data(void *ctx, event_t *e);
extern ret_t on_series_push_rand_ufloat_data(const timer_info_t *timer);
extern ret_t on_series_line_show_changed(void *ctx, event_t *e);
extern ret_t on_series_area_show_changed(void *ctx, event_t *e);
extern ret_t on_series_symbol_show_changed(void *ctx, event_t *e);
extern ret_t on_series_smooth_changed(void *ctx, event_t *e);
extern ret_t on_series_typeie_visible_changed(void *ctx, event_t *e);
extern ret_t on_series_dayas_visible_changed(void *ctx, event_t *e);
extern ret_t on_series_typeie_bring_to_top(void *ctx, event_t *e);
extern ret_t on_series_dayas_bring_to_top(void *ctx, event_t *e);
extern ret_t on_series_bring_to_top(widget_t *win, uint32_t index, const char *icon_name,
                                    const char *unfocus, const char *focus);
extern ret_t axis_time_init(widget_t *widget);


static ret_t on_close(void *ctx, event_t *e)
{
    widget_t *win = (widget_t *)ctx;
    (void)e;

    return window_close(win);
}

static ret_t tooltip_format(void *ctx, const void *data, wstr_t *str)
{
    widget_t *series = WIDGET(ctx);

    const wchar_t *title = series_get_title(series);

    wstr_from_int(str, (int)(*((float *)data)));

    if (title != NULL && wcslen(title) > 0) {
        wstr_insert(str, 0, L": ", wcslen(L": "));
        wstr_insert(str, 0, title, wcslen(title));
    }
}

static ret_t init_widget(void *ctx, const void *iter)
{
    widget_t *widget = WIDGET(iter);
    widget_t *win = widget_get_window(widget);
    bool_t bring_to_top = (char *)ctx - (char *)NULL;
    return_value_if_fail(widget != NULL && win != NULL, RET_BAD_PARAMS);

    if (widget->name != NULL) {
        const char *name = widget->name;

        if (tk_str_eq(name, "line")) {
            widget_on(widget, EVT_CLICK, on_series_line_show_changed, win);
        } else if (tk_str_eq(name, "area")) {
            widget_on(widget, EVT_CLICK, on_series_area_show_changed, win);
        } else if (tk_str_eq(name, "symbol")) {
            widget_on(widget, EVT_CLICK, on_series_symbol_show_changed, win);
        } else if (tk_str_eq(name, "smooth")) {
            widget_on(widget, EVT_CLICK, on_series_smooth_changed, win);
        } else if (tk_str_eq(name, "close")) {
            widget_on(widget, EVT_CLICK, on_close, win);
        } else if (tk_str_eq(name, "typeie")) {
            if (bring_to_top) {
                widget_on(widget, EVT_CLICK, on_series_typeie_bring_to_top, win);
            } else {
                widget_on(widget, EVT_CLICK, on_series_typeie_visible_changed, win);
            }
        } else if (tk_str_eq(name, "dayas")) {
            if (bring_to_top) {
                widget_on(widget, EVT_CLICK, on_series_dayas_bring_to_top, win);
            } else {
                widget_on(widget, EVT_CLICK, on_series_dayas_visible_changed, win);
            }
        }
    }

    return RET_OK;
}

static void init_children_widget(widget_t *widget, void *ctx)
{
    widget_foreach(widget, init_widget, ctx);
}

static void init_normal_line_series_window(widget_t *widget)
{
    widget_t *new_graph = widget_lookup(widget, "new_graph", TRUE);
    bool_t bring_to_top = strstr(widget->name, "_more_axis") != NULL ? TRUE : FALSE;

    if (new_graph != NULL) {
        widget_on(new_graph, EVT_CLICK, on_series_rset_rand_ufloat_data, widget);
    }

    if (bring_to_top) {
        on_series_bring_to_top(widget, 0, "typeie_icon", "typeie", "typeie_focus");
    }

    init_children_widget(widget, (char *)NULL + bring_to_top);

#if SERIES_TIMER_UPDATE
    widget_add_timer(widget, on_series_push_rand_ufloat_data, 1000);
#endif
}


ret_t open_tooltip_format_window(void)
{
    widget_t *win = window_open("window_tooltip_format");
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);
    widget_t *s1 = widget_lookup(win, "s1", TRUE);

    init_normal_line_series_window(win);
    series_set_tooltip_format(s1, tooltip_format, s1);

    axis_time_init(win);

    return RET_OK;
}
