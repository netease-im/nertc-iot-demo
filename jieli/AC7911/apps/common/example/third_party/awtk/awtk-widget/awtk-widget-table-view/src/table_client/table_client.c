/**
 * File:   table_client.c
 * Author: AWTK Develop Team
 * Brief:  表格视图数据区。
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
#include "table_client.h"
#include "../table_row/table_row.h"
#include "base/widget_vtable.h"
#include "widget_animators/widget_animator_scroll.h"

#define PAGES_TO_LOAD 3
#define MAX_ROWS 5000 * 10000

static ret_t table_client_on_scroll(widget_t *widget);

static ret_t table_client_reset_load_data_idle(widget_t *widget)
{
    table_client_t *table_client = TABLE_CLIENT(widget);

    if (table_client->load_data_idle_id != TK_INVALID_ID) {
        idle_remove(table_client->load_data_idle_id);
        table_client->load_data_idle_id = TK_INVALID_ID;
    }
    return RET_OK;
}

static int32_t table_client_rows_per_page(widget_t *widget)
{
    int32_t rows = 1;
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL && table_client->row_height > 0, 1);

    if (widget->h > 0) {
        rows = widget->h / table_client->row_height;
    }
    return tk_max(rows, 1);
}

/*get the index of the first visible row*/
static int32_t table_client_get_vstart_index(widget_t *widget)
{
    int32_t vstart_index = 0;
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL && table_client->row_height > 0, 1);
    vstart_index = table_client->yoffset / (int)(table_client->row_height);

    vstart_index = tk_max(0, vstart_index);

    return vstart_index;
}

ret_t table_client_set_row_height(widget_t *widget, uint32_t row_height)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);

    table_client->row_height = row_height;

    return RET_OK;
}

ret_t table_client_set_rows(widget_t *widget, uint32_t rows)
{
    int32_t yoffset = 0;
    int32_t rows_per_page = 0;
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);
    return_value_if_fail(rows <= MAX_ROWS, RET_BAD_PARAMS);

    table_client->rows = rows;

    if (widget_count_children(widget) < 2) {
        return RET_OK;
    }

    rows_per_page = table_client_rows_per_page(widget);
    yoffset = table_client_get_virtual_h(widget);
    if (table_client->yoffset + table_client->row_height * rows_per_page >= yoffset) {
        yoffset = yoffset - widget->h;
        table_client_set_yoffset(widget, yoffset);
    } else {
        table_client_on_scroll(widget);
        widget_dispatch_simple_event(widget, EVT_SCROLL);
        widget_invalidate_force(widget, NULL);
    }

    return RET_OK;
}

ret_t table_client_set_yoffset(widget_t *widget, int32_t yoffset)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);

    if (table_client->yoffset != yoffset) {
        table_client->yoffset = yoffset;
        table_client_on_scroll(widget);
        widget_dispatch_simple_event(widget, EVT_SCROLL);
        widget_invalidate_force(widget, NULL);
    }

    return RET_OK;
}

ret_t table_client_add_yoffset(widget_t *widget, int32_t delta)
{
    int yoffset = 0;
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);

    yoffset = table_client->yoffset + delta;

    return table_client_set_yoffset(widget, yoffset);
}

ret_t table_client_set_yslidable(widget_t *widget, bool_t yslidable)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);

    table_client->yslidable = yslidable;

    return RET_OK;
}

ret_t table_client_set_yspeed_scale(widget_t *widget, float_t yspeed_scale)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);

    table_client->yspeed_scale = yspeed_scale;

    return RET_OK;
}

static ret_t table_client_get_prop(widget_t *widget, const char *name, value_t *v)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(TABLE_CLIENT_PROP_ROW_HEIGHT, name)) {
        value_set_uint32(v, table_client->row_height);
        return RET_OK;
    } else if (tk_str_eq(TABLE_CLIENT_PROP_ROWS, name)) {
        value_set_uint32(v, table_client->rows);
        return RET_OK;
    } else if (tk_str_eq(TABLE_CLIENT_PROP_YOFFSET, name)) {
        value_set_int32(v, table_client->yoffset);
        return RET_OK;
    } else if (tk_str_eq(WIDGET_PROP_VIRTUAL_H, name)) {
        value_set_int64(v, table_client_get_virtual_h(widget));
        return RET_OK;
    } else if (tk_str_eq(TABLE_CLIENT_PROP_YSLIDABLE, name)) {
        value_set_bool(v, table_client->yslidable);
        return RET_OK;
    } else if (tk_str_eq(TABLE_CLIENT_PROP_YSPEED_SCALE, name)) {
        value_set_float(v, table_client->yspeed_scale);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t table_client_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(TABLE_CLIENT_PROP_ROW_HEIGHT, name)) {
        table_client_set_row_height(widget, value_uint32(v));
        return RET_OK;
    } else if (tk_str_eq(TABLE_CLIENT_PROP_ROWS, name)) {
        table_client_set_rows(widget, value_uint32(v));
        return RET_OK;
    } else if (tk_str_eq(TABLE_CLIENT_PROP_YOFFSET, name)) {
        table_client_set_yoffset(widget, value_int32(v));
        return RET_OK;
    } else if (tk_str_eq(TABLE_CLIENT_PROP_YSLIDABLE, name)) {
        table_client_set_yslidable(widget, value_bool(v));
        return RET_OK;
    } else if (tk_str_eq(TABLE_CLIENT_PROP_YSPEED_SCALE, name)) {
        table_client_set_yspeed_scale(widget, value_float(v));
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t table_client_on_destroy(widget_t *widget)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(widget != NULL && table_client != NULL, RET_BAD_PARAMS);
    table_client_reset_load_data_idle(widget);
    return RET_OK;
}

static ret_t table_client_on_paint_self(widget_t *widget, canvas_t *c)
{
    return RET_OK;
}

static ret_t table_client_prepare_data(widget_t *widget)
{
    table_client_t *table_client = TABLE_CLIENT(widget);

    int32_t i = 0;
    int32_t nr = 0;
    int32_t row_height = table_client->row_height;
    int32_t start_index = table_client->start_index;
    int32_t rows_per_page = table_client_rows_per_page(widget);
    int32_t max_rows_to_load = PAGES_TO_LOAD * rows_per_page;
    return_value_if_fail(widget->children != NULL, RET_BAD_PARAMS);

    if ((start_index + max_rows_to_load) >= table_client->rows) {
        nr = table_client->rows - start_index;
    } else {
        nr = max_rows_to_load;
    }

    nr = tk_min(nr, table_client->rows);
    nr = tk_min(nr, widget_count_children(widget));
    max_rows_to_load = tk_min(max_rows_to_load, widget_count_children(widget));

    for (i = 0; i < nr; i++) {
        uint32_t index = start_index + i;
        widget_t *iter = widget_get_child(widget, i);
        event_t e = event_init(EVT_RESET, iter);

        widget_move(iter, iter->x, index * row_height);
        table_row_set_index(iter, index);
        widget_set_enable(iter, TRUE);
        widget_set_sensitive(iter, TRUE);
        widget_set_visible(iter, TRUE, FALSE);
        widget_dispatch_recursive(iter, &e);

        if (table_client->on_load_data != NULL) {
            table_client->on_load_data(table_client->on_load_data_ctx, index, iter);
        }
    }

    for (; i < max_rows_to_load; i++) {
        widget_t *iter = widget_get_child(widget, i);
        widget_set_visible(iter, FALSE, FALSE);
        widget_set_enable(iter, FALSE);
        widget_set_sensitive(iter, FALSE);
    }

    return RET_OK;
}

static ret_t table_client_on_idle_prepare_data(const idle_info_t *idle)
{
    widget_t *widget = WIDGET(idle->ctx);
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);

    table_client_prepare_data(widget);
    table_client->load_data_idle_id = TK_INVALID_ID;
    return RET_OK;
}

static ret_t table_client_prepare_data_async(widget_t *widget)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);

    if (table_client->load_data_idle_id == TK_INVALID_ID) {
        table_client->load_data_idle_id = idle_add(table_client_on_idle_prepare_data, widget);
    }

    return RET_OK;
}

ret_t table_client_reload(widget_t *widget)
{
    return table_client_prepare_data(widget);
}

static ret_t table_client_on_scroll(widget_t *widget)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    int32_t vstart_index = table_client_get_vstart_index(widget);
    int32_t rows_per_page = table_client_rows_per_page(widget);
    int32_t start_index = table_client->start_index;

    start_index = vstart_index - rows_per_page;
    table_client->start_index = tk_max(0, start_index);
    table_client_prepare_data_async(widget);

    return RET_OK;
}

ret_t table_client_ensure_children(widget_t *widget)
{
    xy_t iw = 0;
    xy_t ih = 0;
    uint32_t i = 0;
    uint32_t nr = 0;
    table_client_t *table_client = TABLE_CLIENT(widget);
    int32_t rows_per_page = table_client_rows_per_page(widget);
    return_value_if_fail(table_client->row_height > 0, RET_BAD_PARAMS);

    iw = widget->w;
    ih = table_client->row_height;
    nr = PAGES_TO_LOAD * rows_per_page;

    if (table_client->on_prepare_row != NULL) {
        table_client->on_prepare_row(table_client->on_prepare_row_ctx, widget, nr);
    } else {
        widget_t *twidget = widget_get_child(widget, 0);
        return_value_if_fail(twidget != NULL, RET_BAD_PARAMS);

        if (nr <= widget_count_children(widget)) {
            return RET_OK;
        }

        for (i = 0; i < nr - 1; i++) {
            ENSURE(widget_clone(twidget, widget) != NULL);
        }
    }

    nr = widget_count_children(widget);
    for (i = 0; i < nr; i++) {
        widget_t *iter = widget_get_child(widget, i);
        widget_move_resize_ex(iter, 0, ih * i, iw, ih, FALSE);
        widget_layout(iter);

        if (table_client->on_create_row != NULL) {
            table_client->on_create_row(table_client->on_create_row_ctx, i, iter);
        }
    }

    table_client_prepare_data(widget);
    widget_dispatch_simple_event(widget, EVT_SCROLL);

    return RET_OK;
}

static ret_t table_client_on_pointer_down(table_client_t *table_client, pointer_event_t *e)
{
    velocity_t *v = &(table_client->velocity);

    velocity_reset(v);
    table_client->down.x = e->x;
    table_client->down.y = e->y;
    table_client->yoffset_save = table_client->yoffset;
    table_client->yoffset_end = table_client->yoffset;

    velocity_update(v, e->e.time, e->x, e->y);

    return RET_OK;
}

static ret_t table_client_on_scroll_done(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(ctx);
    table_client_t *table_client = TABLE_CLIENT(ctx);
    return_value_if_fail(widget != NULL && table_client != NULL, RET_BAD_PARAMS);

    table_client->wa = NULL;
    widget_invalidate_force(widget, NULL);
    widget_dispatch_simple_event(widget, EVT_SCROLL_END);

    return RET_REMOVE;
}

ret_t table_client_scroll_to(widget_t *widget, int32_t yoffset_end, int32_t duration)
{
    int32_t yoffset = 0;
    int32_t max_yoffset = 0;
    table_client_t *table_client = TABLE_CLIENT(widget);
    int32_t rows_per_page = table_client_rows_per_page(widget);
    return_value_if_fail(table_client != NULL, RET_FAIL);

    max_yoffset = table_client->row_height * (table_client->rows - rows_per_page);
    if (yoffset_end > max_yoffset) {
        yoffset_end = max_yoffset;
    }

    if (yoffset_end < 0) {
        yoffset_end = 0;
    }

    if (yoffset_end == table_client->yoffset) {
        return RET_OK;
    }

    yoffset = table_client->yoffset;
    if (table_client->wa != NULL) {
        return RET_OK;
    } else {
        table_client->wa =
            widget_animator_scroll_create(widget, TK_ANIMATING_TIME, 0, EASING_SIN_INOUT);
        return_value_if_fail(table_client->wa != NULL, RET_OOM);

        widget_animator_scroll_set_params(table_client->wa, 0, yoffset, 0, yoffset_end);
        widget_animator_on(table_client->wa, EVT_ANIM_END, table_client_on_scroll_done, table_client);
        widget_animator_start(table_client->wa);
        widget_dispatch_simple_event(widget, EVT_SCROLL_START);
    }

    return RET_OK;
}

ret_t table_client_scroll_to_row(widget_t *widget, int32_t row)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_FAIL);

    return table_client_scroll_to(widget, table_client->row_height * row, TK_ANIMATING_TIME);
}

ret_t table_client_scroll_to_yoffset(widget_t *widget, int32_t yoffset)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_FAIL);

    return table_client_scroll_to(widget, yoffset, TK_ANIMATING_TIME);
}

ret_t table_client_scroll_delta_to(widget_t *widget, int32_t yoffset_delta, int32_t duration)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_FAIL);

    table_client->yoffset_end = table_client->yoffset + yoffset_delta;

    return table_client_scroll_to(widget, table_client->yoffset_end, duration);
}

static ret_t table_client_on_pointer_move(table_client_t *table_client, pointer_event_t *e)
{
    widget_t *widget = WIDGET(table_client);
    velocity_t *v = &(table_client->velocity);
    int32_t dy = e->y - table_client->down.y;
    velocity_update(v, e->e.time, e->x, e->y);

    if (table_client->wa == NULL) {
        if (table_client->yslidable && dy) {
            int32_t yoffset = table_client->yoffset_save - dy;
            table_client_set_yoffset(widget, yoffset);
        }
    }

    table_client->first_move_after_down = FALSE;

    return RET_OK;
}

static ret_t table_client_on_pointer_up(table_client_t *table_client, pointer_event_t *e)
{
    widget_t *widget = WIDGET(table_client);
    velocity_t *v = &(table_client->velocity);
    int32_t move_dy = e->y - table_client->down.y;

    velocity_update(v, e->e.time, e->x, e->y);
    if (table_client->yslidable) {
        int yv = v->yv;

        if (table_client->wa != NULL) {
            widget_animator_scroll_t *wa = (widget_animator_scroll_t *)table_client->wa;
            yv = -(wa->y_to - table_client->yoffset);
        }

        if (table_client->yslidable) {
            if (tk_abs(move_dy) > TK_CLICK_TOLERANCE) {
                table_client->yoffset_end = table_client->yoffset - yv * table_client->yspeed_scale;
            } else {
                table_client->yoffset_end = table_client->yoffset - yv / table_client->yspeed_scale;
            }
        }

        table_client_scroll_to(widget, table_client->yoffset_end, TK_ANIMATING_TIME);
    }

    return RET_OK;
}

static ret_t table_client_on_pointer_down_abort(table_client_t *table_client, pointer_event_t *e)
{
    widget_t *widget = WIDGET(table_client);

    if (table_client->yslidable) {
        table_client_scroll_to(widget, table_client->yoffset_end, TK_ANIMATING_TIME);
    }

    return RET_OK;
}

static bool_t table_client_is_dragged(widget_t *widget, pointer_event_t *evt)
{
    int32_t delta = 0;
    table_client_t *table_client = TABLE_CLIENT(widget);
    if (table_client->yslidable) {
        delta = evt->y - table_client->down.y;
    }

    return (tk_abs(delta) >= TK_DRAG_THRESHOLD);
}

static ret_t table_client_on_event(widget_t *widget, event_t *e)
{
    ret_t ret = RET_OK;
    int32_t row_height = 0;
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(widget != NULL && table_client != NULL, RET_BAD_PARAMS);

    row_height = table_client->row_height;
    switch (e->type) {
    case EVT_WINDOW_OPEN: {
        table_client_ensure_children(widget);
        return RET_OK;
    }
    case EVT_WHEEL: {
        wheel_event_t *evt = (wheel_event_t *)e;
        int32_t delta = evt->dy > 0 ? -row_height : row_height;
        table_client_scroll_delta_to(widget, delta, TK_ANIMATING_TIME);
        ret = RET_STOP;
        break;
    }
    case EVT_KEY_DOWN: {
        key_event_t *evt = (key_event_t *)e;
        if (evt->key == TK_KEY_PAGEDOWN) {
            table_client_scroll_delta_to(widget, widget->h, TK_ANIMATING_TIME);
            ret = RET_STOP;
        } else if (evt->key == TK_KEY_PAGEUP) {
            table_client_scroll_delta_to(widget, -widget->h, TK_ANIMATING_TIME);
            ret = RET_STOP;
        } else if (evt->key == TK_KEY_UP) {
            table_client_scroll_delta_to(widget, -row_height, TK_ANIMATING_TIME);
            ret = RET_STOP;
        } else if (evt->key == TK_KEY_DOWN) {
            table_client_scroll_delta_to(widget, row_height, TK_ANIMATING_TIME);
            ret = RET_STOP;
        }
        break;
    }
    case EVT_POINTER_DOWN: {
        table_client->pressed = TRUE;
        table_client->dragged = FALSE;
        widget_grab(widget->parent, widget);
        table_client->first_move_after_down = TRUE;
        table_client_on_pointer_down(table_client, (pointer_event_t *)e);
        break;
    }
    case EVT_POINTER_DOWN_ABORT: {
        pointer_event_t evt = *(pointer_event_t *)e;
        widget_t *target = widget_find_target(widget, evt.x, evt.y);
        if (target == NULL || target->parent != widget) {
            table_client_on_pointer_down_abort(table_client, (pointer_event_t *)e);
        }
        if (table_client->pressed) {
            widget_ungrab(widget->parent, widget);
        }
        table_client->pressed = FALSE;
        table_client->dragged = FALSE;
        break;
    }
    case EVT_POINTER_UP: {
        pointer_event_t *evt = (pointer_event_t *)e;
        if (table_client->pressed && table_client_is_dragged(widget, evt)) {
            table_client_on_pointer_up(table_client, (pointer_event_t *)e);
        }
        table_client->pressed = FALSE;
        table_client->dragged = FALSE;
        widget_ungrab(widget->parent, widget);
        break;
    }
    case EVT_POINTER_MOVE: {
        pointer_event_t *evt = (pointer_event_t *)e;
        if (!table_client->pressed || !table_client->yslidable) {
            break;
        }

        if (table_client->dragged) {
            table_client_on_pointer_move(table_client, evt);
            widget_invalidate_force(widget, NULL);
        } else {
            if (table_client_is_dragged(widget, evt)) {
                pointer_event_t abort = *evt;

                abort.e.type = EVT_POINTER_DOWN_ABORT;
                widget_dispatch_event_to_target_recursive(widget, (event_t *)(&abort));

                table_client->dragged = TRUE;
            }
        }

        ret = table_client->dragged ? RET_STOP : RET_OK;
        break;
    }
    case EVT_RESIZE:
    case EVT_MOVE_RESIZE: {
        if (widget_is_window_opened(widget)) {
            table_client_ensure_children(widget);
        }
        break;
    }
    default:
        break;
    }

    return ret;
}

const char *s_table_client_properties[] = {
    TABLE_CLIENT_PROP_ROW_HEIGHT, TABLE_CLIENT_PROP_ROWS,         TABLE_CLIENT_PROP_YOFFSET,
    TABLE_CLIENT_PROP_YSLIDABLE,  TABLE_CLIENT_PROP_YSPEED_SCALE, NULL
};

static ret_t table_client_paint_children(widget_t *widget, canvas_t *c)
{
    int32_t i = 0;
    int32_t nr = 0;
    table_client_t *table_client = TABLE_CLIENT(widget);
    int32_t start_index = table_client->start_index;
    int32_t vstart_index = table_client_get_vstart_index(widget);
    int32_t rows_per_page = table_client_rows_per_page(widget);
    return_value_if_fail(widget->children != NULL, RET_BAD_PARAMS);

    if ((vstart_index + rows_per_page) >= table_client->rows) {
        nr = table_client->rows - vstart_index;
    } else {
        nr = rows_per_page + 1;
    }

    for (i = 0; i < nr; i++) {
        uint32_t index = vstart_index - start_index + i;
        widget_t *iter = widget_get_child(widget, index);
        widget_paint(iter, c);
    }

    return RET_OK;
}

static ret_t table_client_on_paint_children(widget_t *widget, canvas_t *c)
{
    rect_t r_save;
    vgcanvas_t *vg = canvas_get_vgcanvas(c);
    table_client_t *table_client = TABLE_CLIENT(widget);
    rect_t r = rect_init(c->ox, c->oy, widget->w, widget->h);
    int32_t yoffset = -table_client->yoffset;

    canvas_translate(c, 0, yoffset);
    canvas_get_clip_rect(c, &r_save);

    r = rect_intersect(&r, &r_save);

    if (vg != NULL) {
        vgcanvas_save(vg);
        vgcanvas_clip_rect(vg, (float_t)r.x, (float_t)r.y, (float_t)r.w, (float_t)r.h);
    }

    canvas_set_clip_rect(c, &r);
    table_client_paint_children(widget, c);
    canvas_set_clip_rect(c, &r_save);
    canvas_untranslate(c, 0, yoffset);

    if (vg != NULL) {
        vgcanvas_clip_rect(vg, (float_t)r_save.x, (float_t)r_save.y, (float_t)r_save.w,
                           (float_t)r_save.h);
        vgcanvas_restore(vg);
    }
    return RET_OK;
}

static widget_t *table_client_find_target(widget_t *widget, xy_t x, xy_t y)
{
    return widget_find_target_default(widget, x, y);
}

static ret_t table_client_invalidate(widget_t *widget, const rect_t *r)
{
    return widget_invalidate_default(widget, r);
}

ret_t table_client_set_on_load_data(widget_t *widget, table_client_on_load_data_t on_load_data,
                                    void *ctx)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);

    table_client_reset_load_data_idle(widget);
    table_client->on_load_data_ctx = ctx;
    table_client->on_load_data = on_load_data;

    return RET_OK;
}

ret_t table_client_set_on_create_row(widget_t *widget, table_client_on_create_row_t on_create_row,
                                     void *ctx)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);

    table_client->on_create_row_ctx = ctx;
    table_client->on_create_row = on_create_row;

    return RET_OK;
}

ret_t table_client_set_on_prepare_row(widget_t *widget,
                                      table_client_on_prepare_row_t on_prepare_row, void *ctx)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, RET_BAD_PARAMS);

    table_client->on_prepare_row_ctx = ctx;
    table_client->on_prepare_row = on_prepare_row;

    return RET_OK;
}

static ret_t table_client_get_offset(widget_t *widget, xy_t *out_x, xy_t *out_y)
{
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL && out_x != NULL && out_y != NULL, RET_BAD_PARAMS);
    *out_x = 0;
    *out_y = table_client->yoffset;

    return RET_OK;
}

TK_DECL_VTABLE(table_client) = {.size = sizeof(table_client_t),
                                .type = WIDGET_TYPE_TABLE_CLIENT,
                                .scrollable = TRUE,
                                .clone_properties = s_table_client_properties,
                                .persistent_properties = s_table_client_properties,
                                .parent = TK_PARENT_VTABLE(widget),
                                .create = table_client_create,
                                .on_paint_self = table_client_on_paint_self,
                                .get_offset = table_client_get_offset,
                                .set_prop = table_client_set_prop,
                                .get_prop = table_client_get_prop,
                                .on_event = table_client_on_event,
                                .on_paint_children = table_client_on_paint_children,
                                .find_target = table_client_find_target,
                                .invalidate = table_client_invalidate,
                                .on_destroy = table_client_on_destroy
                               };

widget_t *table_client_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(table_client), x, y, w, h);
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, NULL);

    table_client->rows = 0;
    table_client->row_height = 40;
    table_client->yslidable = TRUE;
    table_client->yspeed_scale = 1;

    return widget;
}

widget_t *table_client_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, table_client), NULL);

    return widget;
}

int64_t table_client_get_virtual_h(widget_t *widget)
{
    int64_t virtual_h = 0;
    table_client_t *table_client = TABLE_CLIENT(widget);
    return_value_if_fail(table_client != NULL, 1);

    virtual_h = table_client->rows * table_client->row_height;

    return tk_max(virtual_h, widget->h);
}
