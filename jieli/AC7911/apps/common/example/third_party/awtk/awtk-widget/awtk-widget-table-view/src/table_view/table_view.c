/**
 * File:   table_view.c
 * Author: AWTK Develop Team
 * Brief:  表格视图。
 *
 * Copyright (c) 2020 - 2020 Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-07-15 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "table_view.h"
#include "scroll_view/scroll_bar.h"
#include "../table_client/table_client.h"

static ret_t table_view_on_client_scrolled(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(ctx);
    table_view_t *table_view = TABLE_VIEW(widget);
    table_client_t *client = TABLE_CLIENT(table_view->client);
    widget_t *vbar = table_view->vbar;

    if (vbar != NULL && client != NULL) {
        int64_t value = client->yoffset;
        int64_t row_height = client->row_height;
        int64_t virtual_h = table_client_get_virtual_h(WIDGET(client));

        scroll_bar_set_params(vbar, tk_max(vbar->h, virtual_h), row_height);

        if (virtual_h <= vbar->h) {
            value = 0;
        } else {
            value = value * virtual_h / (virtual_h - vbar->h);
        }

        scroll_bar_set_value_only(vbar, (int32_t)value);
    }

    return RET_OK;
}

static ret_t table_view_on_scroll_bar_changed(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(ctx);
    widget_t *target = WIDGET(e->target);
    table_view_t *table_view = TABLE_VIEW(widget);
    widget_t *client = table_view->client;
    scroll_bar_t *bar = SCROLL_BAR(target);
    int64_t value = bar->value;
    int64_t virtual_h = bar->virtual_size;
    double scale = (virtual_h - client->h) / (double)virtual_h;

    emitter_disable(client->emitter);
    table_client_set_yoffset(client, value * scale);
    emitter_enable(client->emitter);

    return RET_OK;
}

ret_t table_view_on_add_child(widget_t *widget, widget_t *child)
{
    table_view_t *table_view = TABLE_VIEW(widget);
    const char *type = widget_get_type(child);

    if (tk_str_eq(type, WIDGET_TYPE_SCROLL_BAR_MOBILE) ||
        tk_str_eq(type, WIDGET_TYPE_SCROLL_BAR_DESKTOP)) {
        widget_on(child, EVT_VALUE_CHANGING, table_view_on_scroll_bar_changed, widget);
        widget_on(child, EVT_VALUE_CHANGED, table_view_on_scroll_bar_changed, widget);
        table_view->vbar = child;
    } else if (tk_str_eq(type, WIDGET_TYPE_TABLE_CLIENT)) {
        table_view->client = child;
        widget_on(child, EVT_SCROLL, table_view_on_client_scrolled, widget);
    }

    return RET_FAIL;
}

const char *s_table_view_properties[] = {NULL};

TK_DECL_VTABLE(table_view) = {.size = sizeof(table_view_t),
                              .type = WIDGET_TYPE_TABLE_VIEW,
                              .clone_properties = s_table_view_properties,
                              .persistent_properties = s_table_view_properties,
                              .parent = TK_PARENT_VTABLE(widget),
                              .create = table_view_create,
                              .on_add_child = table_view_on_add_child
                             };

widget_t *table_view_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(table_view), x, y, w, h);
    table_view_t *table_view = TABLE_VIEW(widget);
    return_value_if_fail(table_view != NULL, NULL);

    return widget;
}

widget_t *table_view_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, table_view), NULL);

    return widget;
}
