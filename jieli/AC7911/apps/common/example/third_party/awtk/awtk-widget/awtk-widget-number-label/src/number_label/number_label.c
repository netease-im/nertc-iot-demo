/**
 * File:   number_label.h
 * Author: AWTK Develop Team
 * Brief:  number_label
 *
 * Copyright (c) 2020 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-05-19 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "base/widget_vtable.h"
#include "number_label.h"

ret_t number_label_set_format(widget_t *widget, const char *format)
{
    number_label_t *number_label = NUMBER_LABEL(widget);
    return_value_if_fail(widget != NULL, RET_BAD_PARAMS);
    return_value_if_fail(format != NULL, RET_BAD_PARAMS);

    number_label->format = tk_str_copy(number_label->format, format);

    return widget_invalidate(widget, NULL);
}

ret_t number_label_set_value(widget_t *widget, double value)
{
    number_label_t *number_label = NUMBER_LABEL(widget);
    return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

    number_label->value = value;

    return widget_invalidate(widget, NULL);
}

ret_t number_label_set_decimal_font_size_scale(widget_t *widget, double decimal_font_size_scale)
{
    number_label_t *number_label = NUMBER_LABEL(widget);
    return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

    number_label->decimal_font_size_scale = decimal_font_size_scale;

    return widget_invalidate(widget, NULL);
}

static ret_t number_label_get_prop(widget_t *widget, const char *name, value_t *v)
{
    number_label_t *number_label = NUMBER_LABEL(widget);
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(name, WIDGET_PROP_MIN)) {
        value_set_double(v, number_label->min);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_MAX)) {
        value_set_double(v, number_label->max);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_STEP)) {
        value_set_double(v, number_label->step);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_READONLY)) {
        value_set_bool(v, number_label->readonly);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_LOOP)) {
        value_set_bool(v, number_label->loop);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_VALUE)) {
        value_set_double(v, number_label->value);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_FORMAT)) {
        value_set_str(v, number_label->format);
        return RET_OK;
    } else if (tk_str_eq(name, NUMBER_LABEL_PROP_DECIMAL_FONT_SIZE_SCALE)) {
        value_set_double(v, number_label->decimal_font_size_scale);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t number_label_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    number_label_t *number_label = NUMBER_LABEL(widget);
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(name, WIDGET_PROP_MIN)) {
        number_label->min = value_double(v);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_MAX)) {
        number_label->max = value_double(v);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_STEP)) {
        number_label->step = value_double(v);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_READONLY)) {
        number_label->readonly = value_bool(v);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_LOOP)) {
        number_label->loop = value_bool(v);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_VALUE)) {
        number_label_set_value(widget, value_double(v));
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_FORMAT)) {
        number_label_set_format(widget, value_str(v));
        return RET_OK;
    } else if (tk_str_eq(name, NUMBER_LABEL_PROP_DECIMAL_FONT_SIZE_SCALE)) {
        number_label_set_decimal_font_size_scale(widget, value_double(v));
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t number_label_on_destroy(widget_t *widget)
{
    number_label_t *number_label = NUMBER_LABEL(widget);
    return_value_if_fail(widget != NULL && number_label != NULL, RET_BAD_PARAMS);

    TKMEM_FREE(number_label->format);

    return RET_OK;
}

static ret_t number_label_paint_text(widget_t *widget, canvas_t *c, wstr_t *text)
{
    int32_t x = 0;
    int32_t y = 0;
    int32_t w = widget->w;
    int32_t h = widget->h;
    float_t font_height = 0;
    float_t text_width = 0;
    float_t int_part_width = 0;
    float_t decimal_part_width = 0;
    int32_t int_part_len = 0;
    int32_t decimal_part_len = 0;
    style_t *style = widget->astyle;
    color_t trans = color_init(0, 0, 0, 0);
    number_label_t *number_label = NUMBER_LABEL(widget);
    color_t tc = style_get_color(style, STYLE_ID_TEXT_COLOR, trans);
    const char *font_name = style_get_str(style, STYLE_ID_FONT_NAME, NULL);
    int32_t margin = style_get_int(style, STYLE_ID_MARGIN, 0);
    int32_t margin_top = style_get_int(style, STYLE_ID_MARGIN_TOP, margin);
    int32_t margin_left = style_get_int(style, STYLE_ID_MARGIN_LEFT, margin);
    int32_t margin_bottom = style_get_int(style, STYLE_ID_MARGIN_BOTTOM, margin);
    uint16_t font_size = style_get_int(style, STYLE_ID_FONT_SIZE, TK_DEFAULT_FONT_SIZE);
    uint16_t decimal_font_size = font_size * number_label->decimal_font_size_scale;
    align_h_t align_h = (align_h_t)style_get_int(style, STYLE_ID_TEXT_ALIGN_H, ALIGN_H_CENTER);
    align_v_t align_v = (align_v_t)style_get_int(style, STYLE_ID_TEXT_ALIGN_V, ALIGN_V_MIDDLE);

    while (text->str[int_part_len] != 0 && text->str[int_part_len] != '.') {
        int_part_len++;
    }
    int_part_len++;
    decimal_part_len = text->size - int_part_len;
    decimal_part_len = tk_max(decimal_part_len, 0);

    canvas_set_text_color(c, tc);
    canvas_set_text_align(c, align_h, align_v);

    canvas_set_font(c, font_name, font_size);
    int_part_width = canvas_measure_text(c, text->str, int_part_len);

    canvas_set_font(c, font_name, decimal_font_size);
    decimal_part_width = canvas_measure_text(c, text->str + int_part_len, decimal_part_len);

    text_width = int_part_width + decimal_part_width;
    switch (align_h) {
    case ALIGN_H_LEFT: {
        x = margin_left;
        break;
    }
    case ALIGN_H_RIGHT: {
        x = w - margin_left - text_width;
        break;
    }
    default: {
        x = (w - text_width) / 2;
        break;
    }
    }

    canvas_set_font(c, font_name, font_size);
    font_height = canvas_get_font_height(c);
    switch (align_v) {
    case ALIGN_V_TOP: {
        y = margin_top;
        canvas_draw_text(c, text->str, int_part_len, x, y);

        break;
    }
    case ALIGN_V_BOTTOM: {
        y = h - margin_bottom - font_height;
        canvas_draw_text(c, text->str, int_part_len, x, y);

        break;
    }
    default: {
        y = (h - font_height) / 2;
        canvas_draw_text(c, text->str, int_part_len, x, y);
        break;
    }
    }

    canvas_set_font(c, font_name, decimal_font_size);
    y += font_height * (1 - number_label->decimal_font_size_scale) - 1;
    x += int_part_width;
    canvas_draw_text(c, text->str + int_part_len, decimal_part_len, x, y);

    return RET_OK;
}

static ret_t number_label_on_paint_self(widget_t *widget, canvas_t *c)
{
    char buff[64];
    wstr_t *text = &(widget->text);
    number_label_t *number_label = NUMBER_LABEL(widget);
    const char *format =
        number_label->format != NULL ? number_label->format : NUMBER_LABEL_DEFAULT_FORMAT;
    return_value_if_fail(widget->astyle != NULL, RET_BAD_PARAMS);

    memset(buff, 0x00, sizeof(buff));

    if (strchr(format, 'd') != NULL) {
        tk_snprintf(buff, sizeof(buff) - 1, format, (int)(number_label->value));
    } else {
        tk_snprintf(buff, sizeof(buff) - 1, format, number_label->value);
    }

    wstr_set_utf8(text, buff);

    return number_label_paint_text(widget, c, text);
}

ret_t number_label_set_limit(widget_t *widget, double min, double max, double step)
{
    number_label_t *number_label = NUMBER_LABEL(widget);
    return_value_if_fail(number_label != NULL, RET_BAD_PARAMS);

    number_label->min = min;
    number_label->max = max;
    number_label->step = step;

    return RET_OK;
}

ret_t number_label_set_readonly(widget_t *widget, bool_t readonly)
{
    number_label_t *number_label = NUMBER_LABEL(widget);
    return_value_if_fail(number_label != NULL, RET_BAD_PARAMS);

    number_label->readonly = readonly;

    return RET_OK;
}

ret_t number_label_set_loop(widget_t *widget, bool_t loop)
{
    number_label_t *number_label = NUMBER_LABEL(widget);
    return_value_if_fail(number_label != NULL, RET_BAD_PARAMS);

    number_label->loop = loop;

    return RET_OK;
}

static ret_t number_label_add_delta(widget_t *widget, double delta)
{
    number_label_t *number_label = NUMBER_LABEL(widget);
    double value = number_label->value + delta;

    if (number_label->min < number_label->max) {
        if (value < number_label->min) {
            value = number_label->loop ? number_label->max : number_label->min;
        }

        if (value > number_label->max) {
            value = number_label->loop ? number_label->min : number_label->max;
        }
    }

    if (!tk_fequal(number_label->value, value)) {
        value_change_event_t evt;
        value_change_event_init(&evt, EVT_VALUE_WILL_CHANGE, widget);
        value_set_double(&(evt.old_value), number_label->value);
        value_set_double(&(evt.new_value), value);

        if (widget_dispatch(widget, (event_t *)&evt) != RET_STOP) {
            number_label->value = value;
            evt.e.type = EVT_VALUE_CHANGED;
            widget_dispatch(widget, (event_t *)&evt);
            widget_invalidate(widget, NULL);
        }
    }

    return RET_OK;
}

ret_t number_label_on_event(widget_t *widget, event_t *e)
{
    ret_t ret = RET_OK;
    number_label_t *number_label = NUMBER_LABEL(widget);

    switch (e->type) {
    case EVT_KEY_DOWN: {
        key_event_t *evt = (key_event_t *)e;
        if (!(number_label->readonly)) {
            return_value_if_fail(number_label->step != 0, RET_FAIL);

            if (evt->key == TK_KEY_UP || evt->key == TK_KEY_LEFT) {
                number_label_add_delta(widget, number_label->step);
                ret = RET_STOP;
            } else if (evt->key == TK_KEY_DOWN || evt->key == TK_KEY_RIGHT) {
                number_label_add_delta(widget, -number_label->step);
                ret = RET_STOP;
            }
        }
        break;
    }
    }

    return ret;
}

static const char *s_number_label_properties[] = {NUMBER_LABEL_PROP_DECIMAL_FONT_SIZE_SCALE,
                                                  WIDGET_PROP_MIN,
                                                  WIDGET_PROP_MAX,
                                                  WIDGET_PROP_STEP,
                                                  WIDGET_PROP_LOOP,
                                                  WIDGET_PROP_READONLY,
                                                  WIDGET_PROP_FORMAT,
                                                  WIDGET_PROP_VALUE,
                                                  NULL
                                                 };

TK_DECL_VTABLE(number_label) = {.size = sizeof(number_label_t),
                                .type = WIDGET_TYPE_NUMBER_LABEL,
                                .parent = NULL,
                                .clone_properties = s_number_label_properties,
                                .persistent_properties = s_number_label_properties,
                                .create = number_label_create,
                                .on_paint_self = number_label_on_paint_self,
                                .set_prop = number_label_set_prop,
                                .get_prop = number_label_get_prop,
                                .on_event = number_label_on_event,
                                .on_destroy = number_label_on_destroy
                               };

widget_t *number_label_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    number_label_t *number_label =
        NUMBER_LABEL(widget_create(parent, TK_REF_VTABLE(number_label), x, y, w, h));
    number_label->format = tk_strdup(NUMBER_LABEL_DEFAULT_FORMAT);
    number_label->min = 0;
    number_label->max = 0;
    number_label->step = 1;
    number_label->readonly = FALSE;
    number_label->decimal_font_size_scale = 0.6;

    return (widget_t *)number_label;
}

widget_t *number_label_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, number_label), NULL);

    return widget;
}
