/**
 * File:   vlabel.c
 * Author: AWTK Develop Team
 * Brief:  垂直显示文本的控件。
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
 * 2020-10-01 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "vlabel.h"

ret_t vlabel_set_right_to_left(widget_t *widget, bool_t right_to_left)
{
    vlabel_t *vlabel = VLABEL(widget);
    return_value_if_fail(vlabel != NULL, RET_BAD_PARAMS);

    vlabel->right_to_left = right_to_left;

    return RET_OK;
}

static ret_t vlabel_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    vlabel_t *vlabel = VLABEL(widget);
    return_value_if_fail(vlabel != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(VLABEL_PROP_RIGHT_TO_LEFT, name)) {
        vlabel->right_to_left = value_bool(v);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t vlabel_get_prop(widget_t *widget, const char *name, value_t *v)
{
    vlabel_t *vlabel = VLABEL(widget);
    return_value_if_fail(vlabel != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(VLABEL_PROP_RIGHT_TO_LEFT, name)) {
        value_set_bool(v, vlabel->right_to_left);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t vlabel_on_destroy(widget_t *widget)
{
    vlabel_t *vlabel = VLABEL(widget);
    return_value_if_fail(widget != NULL && vlabel != NULL, RET_BAD_PARAMS);

    return RET_OK;
}

ret_t vlabel_line_parser_init(vlabel_line_parser_t *parser, const wchar_t *str, uint32_t size,
                              uint32_t font_size, uint32_t height)
{
    return_value_if_fail(parser != NULL && str != NULL && size > 0, RET_BAD_PARAMS);
    return_value_if_fail(font_size > 0 && height >= font_size, RET_BAD_PARAMS);

    memset(parser, 0x00, sizeof(vlabel_line_parser_t));

    parser->str = str;
    parser->size = size;
    parser->height = height;
    parser->font_size = font_size;

    parser->line = str;
    parser->line_size = 0;
    parser->is_force_break = FALSE;

    while (vlabel_line_parser_next(parser) == RET_OK) {
        if (parser->is_force_break) {
            parser->total_paragraphs++;
        }
        parser->total_lines++;
    }

    if (!parser->is_force_break) {
        parser->total_paragraphs++;
    }

    parser->line = str;
    parser->line_size = 0;

    return RET_OK;
}

ret_t vlabel_line_parser_next(vlabel_line_parser_t *parser)
{
    uint32_t h = 0;
    const wchar_t *p = NULL;
    return_value_if_fail(parser != NULL && parser->str != NULL, RET_BAD_PARAMS);

    parser->line = parser->line + parser->line_size;
    parser->line_size = 0;

    if ((parser->line - parser->str) >= parser->size) {
        return RET_EOS;
    }

    p = parser->line;
    parser->is_force_break = FALSE;
    while ((p - parser->str) < parser->size) {
        if (*p == '\r') {
            p++;
            if (*p == '\n') {
                p++;
            }
            parser->is_force_break = TRUE;
            break;
        } else if (*p == '\n') {
            p++;
            parser->is_force_break = TRUE;
            break;
        }

        if ((h + parser->font_size) > parser->height) {
            break;
        }

        p++;
        h += parser->font_size;
    }
    parser->line_size = p - parser->line;

    return RET_OK;
}

static ret_t vlabel_draw_text_line_impl(widget_t *widget, canvas_t *c, const wchar_t *line,
                                        uint32_t size, int32_t x, int32_t vmargin,
                                        align_v_t align_v)
{
    int32_t y = 0;
    uint32_t i = 0;
    uint32_t font_size = c->font_size;
    uint32_t text_h = font_size * size;
    rect_t r = rect_init(x, vmargin, font_size, font_size);

    switch (align_v) {
    case ALIGN_V_MIDDLE: {
        y = (widget->h - text_h) >> 1;
        break;
    }
    case ALIGN_V_BOTTOM: {
        y = widget->h - vmargin - text_h;
        break;
    }
    default: {
        y = vmargin;
        break;
    }
    }

    if (y < vmargin) {
        y = vmargin;
    }

    r.y = y;
    for (i = 0; i < size; i++) {
        canvas_draw_text_in_rect(c, line + i, 1, &r);
        r.y += font_size;
    }

    return RET_OK;
}

static ret_t vlabel_draw_text_line(widget_t *widget, canvas_t *c, const wchar_t *line,
                                   uint32_t size, int32_t x, int32_t vmargin, align_v_t align_v)
{
    uint32_t i = 0;
    return_value_if_fail(line != NULL, RET_BAD_PARAMS);

    for (i = 0; i < size; i++) {
        if (line[i] == '\r' || line[i] == '\n') {
            size = i;
            break;
        }
    }

    return vlabel_draw_text_line_impl(widget, c, line, size, x, vmargin, align_v);
}

static ret_t vlabel_draw_text(widget_t *widget, canvas_t *c)
{
    int32_t x = 0;
    int32_t y = 0;
    int32_t w = 0;
    int32_t h = 0;
    int32_t right = 0;
    int32_t w_text = 0;
    int32_t x_delta = 0;
    int32_t line_w = 0;
    vlabel_line_parser_t p;
    style_t *style = widget->astyle;
    vlabel_t *vlabel = VLABEL(widget);
    int32_t font_size = c->font_size;
    int32_t margin = style_get_int(style, STYLE_ID_MARGIN, 2);
    int32_t spacer = style_get_int(style, STYLE_ID_SPACER, 2);
    align_v_t align_v = (align_v_t)style_get_int(style, STYLE_ID_TEXT_ALIGN_V, ALIGN_V_TOP);
    align_h_t align_h = (align_v_t)style_get_int(style, STYLE_ID_TEXT_ALIGN_H, ALIGN_H_LEFT);

    line_w = font_size + spacer;
    w = widget->w - margin - margin;
    h = widget->h - margin - margin;

    return_value_if_fail(
        vlabel_line_parser_init(&p, widget->text.str, widget->text.size, font_size, h) == RET_OK,
        RET_BAD_PARAMS);

    x = margin;
    y = margin;
    w_text = p.total_lines * line_w;
    x_delta = vlabel->right_to_left ? -line_w : line_w;

    switch (align_h) {
    case ALIGN_H_CENTER: {
        x = (w - w_text) >> 1;
        if (x < margin) {
            x = margin;
        }
        if (vlabel->right_to_left) {
            x += w_text - line_w;
        }

        break;
    }
    case ALIGN_H_RIGHT: {
        if (vlabel->right_to_left) {
            x = widget->w - margin - line_w;
        } else {
            x = widget->w - margin - w_text;
        }
        break;
    }
    default: {
        if (vlabel->right_to_left) {
            x = margin + w_text - line_w;
        } else {
            x = margin;
        }
        break;
    }
    }

    right = widget->w - margin - line_w;
    if (x < margin) {
        x = margin;
    }
    if (x > right) {
        x = right;
    }

    canvas_set_text_align(c, ALIGN_H_CENTER, ALIGN_V_MIDDLE);
    while (vlabel_line_parser_next(&p) == RET_OK) {
        if (x < margin || x > right) {
            break;
        }

        vlabel_draw_text_line(widget, c, p.line, p.line_size, x, y, align_v);
        x += x_delta;
    }

    return RET_OK;
}

static ret_t vlabel_on_paint_self(widget_t *widget, canvas_t *c)
{
    return_value_if_fail(widget->astyle != NULL, RET_BAD_PARAMS);

    if (widget->text.size > 0) {
        widget_prepare_text_style(widget, c);
        vlabel_draw_text(widget, c);
    }

    return RET_OK;
}

const char *s_vlabel_properties[] = {VLABEL_PROP_RIGHT_TO_LEFT, NULL};

TK_DECL_VTABLE(vlabel) = {.size = sizeof(vlabel_t),
                          .type = WIDGET_TYPE_VLABEL,
                          .clone_properties = s_vlabel_properties,
                          .persistent_properties = s_vlabel_properties,
                          .parent = TK_PARENT_VTABLE(widget),
                          .create = vlabel_create,
                          .on_paint_self = vlabel_on_paint_self,
                          .set_prop = vlabel_set_prop,
                          .get_prop = vlabel_get_prop,
                          .on_destroy = vlabel_on_destroy
                         };

widget_t *vlabel_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(vlabel), x, y, w, h);
    vlabel_t *vlabel = VLABEL(widget);
    return_value_if_fail(vlabel != NULL, NULL);

    vlabel->right_to_left = FALSE;

    return widget;
}

widget_t *vlabel_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, vlabel), NULL);

    return widget;
}
