/**
 * File:   qr.c
 * Author: AWTK Develop Team
 * Brief:  显示二维码的控件
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
 * 2020-06-01 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "base/widget_vtable.h"

#include "qr.h"

#define IMAGE_QR_MAX_RATIO 0.45f /* 贴图的面积和二维码的面积最大比例 */

static ret_t qr_ensure_qrcode(widget_t *widget);

ret_t qr_set_value(widget_t *widget, const char *value)
{
    qr_t *qr = QR(widget);
    return_value_if_fail(qr != NULL, RET_BAD_PARAMS);

    qr->value = tk_str_copy(qr->value, value);

    if (qr->qrcode != NULL) {
        QRcode_free(qr->qrcode);
        qr->qrcode = NULL;
    }

    return (qr_ensure_qrcode(widget) == RET_OK) ? widget_invalidate(widget, NULL) : RET_FAIL;
}

static ret_t qr_get_prop(widget_t *widget, const char *name, value_t *v)
{
    qr_t *qr = QR(widget);
    return_value_if_fail(qr != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(WIDGET_PROP_VALUE, name) || tk_str_eq(WIDGET_PROP_TEXT, name)) {
        value_set_str(v, qr->value);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t qr_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    qr_t *qr = QR(widget);
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(WIDGET_PROP_VALUE, name) || tk_str_eq(WIDGET_PROP_TEXT, name)) {
        qr_set_value(widget, value_str(v));

        if (qr->qrcode != NULL) {
            QRcode_free(qr->qrcode);
            qr->qrcode = NULL;
        }

        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t qr_on_destroy(widget_t *widget)
{
    qr_t *qr = QR(widget);
    return_value_if_fail(widget != NULL && qr != NULL, RET_BAD_PARAMS);

    TKMEM_FREE(qr->value);
    if (qr->qrcode != NULL) {
        QRcode_free(qr->qrcode);
        qr->qrcode = NULL;
    }

    return RET_OK;
}

#define MIN_SIZE 21

static ret_t qr_ensure_qrcode(widget_t *widget)
{
    qr_t *qr = QR(widget);
    int32_t size = tk_min(widget->w, widget->h) - 5;

    if (size < MIN_SIZE) {
        return RET_FAIL;
    }

    if (qr->qrcode != NULL) {
        return RET_OK;
    }

    qr->qrcode = QRcode_encodeString(qr->value, 3, QR_ECLEVEL_H, QR_MODE_8, 1);

    if (qr->qrcode != NULL) {
        return RET_OK;
    }

    return RET_FAIL;
}

static ret_t qr_paint_logo(widget_t *widget, canvas_t *c)
{
    vgcanvas_t *vg = canvas_get_vgcanvas(c);
    const char *logo_name = NULL;
    bitmap_t logo = {0};
    return_value_if_fail(widget != NULL && vg != NULL, RET_BAD_PARAMS);

    logo_name = style_get_str(widget->astyle, STYLE_ID_BG_IMAGE, NULL);

    if (logo_name != NULL && widget_load_image(widget, logo_name, &logo) == RET_OK) {
        vgcanvas_translate(vg, c->ox + (widget->w - logo.w) / 2.0f,
                           c->oy + (widget->h - logo.h) / 2.0f);
        vgcanvas_draw_image(vg, &logo, 0, 0, logo.w, logo.h, 0, 0, logo.w, logo.h);
    }
    return RET_OK;
}

static ret_t qr_on_paint_self(widget_t *widget, canvas_t *c)
{
    qr_t *qr = QR(widget);
    style_t *style = widget->astyle;

    if (style != NULL) {
        color_t trans = color_init(0, 0, 0, 0);
        uint32_t margin = style_get_int(style, STYLE_ID_MARGIN, 2);
        color_t bg = style_get_color(style, STYLE_ID_BG_COLOR, trans);
        color_t fg = style_get_color(style, STYLE_ID_FG_COLOR, trans);

        if (qr_ensure_qrcode(widget) == RET_OK) {
            uint32_t i = 0;
            uint32_t j = 0;
            uint8_t *p = qr->qrcode->data;
            uint32_t size = qr->qrcode->width;
            uint32_t pix_size = (tk_min(widget->w, widget->h) - 2 * margin) / size;
            uint32_t x = (widget->w - pix_size * size) / 2;
            uint32_t y = (widget->h - pix_size * size) / 2;
            rect_t r_clip = rect_init(widget->x + ((1 - IMAGE_QR_MAX_RATIO) / 2.0f) * widget->w,
                                      widget->y + ((1 - IMAGE_QR_MAX_RATIO) / 2.0f) * widget->h,
                                      IMAGE_QR_MAX_RATIO * widget->w, IMAGE_QR_MAX_RATIO * widget->h);

            return_value_if_fail(pix_size > 1, RET_BAD_PARAMS);

            for (j = 0; j < size; j++) {
                x = (widget->w - pix_size * size) / 2;
                for (i = 0; i < size; i++) {
                    if (*p & 0x01) {
                        canvas_set_fill_color(c, fg);
                    } else {
                        canvas_set_fill_color(c, bg);
                    }
                    canvas_fill_rect(c, x, y, pix_size, pix_size);
                    p++;
                    x += pix_size;
                }
                y += pix_size;
            }

            widget_paint_with_clip(widget, &r_clip, c, qr_paint_logo);
        }
    }

    return RET_OK;
}

static ret_t qr_on_event(widget_t *widget, event_t *e)
{
    qr_t *qr = QR(widget);
    return_value_if_fail(widget != NULL && qr != NULL, RET_BAD_PARAMS);

    (void)qr;

    return RET_OK;
}

const char *s_qr_properties[] = {WIDGET_PROP_VALUE, NULL};

TK_DECL_VTABLE(qr) = {.size = sizeof(qr_t),
                      .type = WIDGET_TYPE_QR,
                      .clone_properties = s_qr_properties,
                      .persistent_properties = s_qr_properties,
                      .parent = TK_PARENT_VTABLE(widget),
                      .create = qr_create,
                      .on_paint_self = qr_on_paint_self,
                      .set_prop = qr_set_prop,
                      .get_prop = qr_get_prop,
                      .on_event = qr_on_event,
                      .on_destroy = qr_on_destroy
                     };

widget_t *qr_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(qr), x, y, w, h);
    qr_t *qr = QR(widget);
    return_value_if_fail(qr != NULL, NULL);

    return widget;
}

widget_t *qr_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, qr), NULL);

    return widget;
}
