/**
 * File:   rlottie.c
 * Author: AWTK Develop Team
 * Brief:  Lottie animations player
 *
 * Copyright (c) 2021 - 2021 Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2021-06-04 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "rlottie.h"
#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/data_reader_factory.h"

/*ARGB32 --> RGBA32*/
static ret_t rlottie_fix_bitmap(uint32_t *buff, int32_t w, int32_t h)
{
    int32_t i = 0;
    int32_t n = w * h;

    for (i = 0; i < n; i++) {
        uint8_t *p = (uint8_t *)(buff + i);
        uint8_t r = p[0];
        *p = p[2];
        p[2] = r;
    }

    return RET_OK;
}

static ret_t rlottie_on_timer(const timer_info_t *info)
{
    rlottie_t *rlottie = RLOTTIE(info->ctx);

    if (rlottie->animation != NULL && rlottie->total_frames > 0) {
        uint32_t *buff = NULL;
        if (rlottie->running && rlottie->bitmap != NULL) {
            bitmap_t *b = rlottie->bitmap;
            rlottie->index = (rlottie->index + 1) % rlottie->total_frames;

            b->flags |= BITMAP_FLAG_CHANGED;
            buff = (uint32_t *)bitmap_lock_buffer_for_write(b);
            lottie_animation_render(rlottie->animation, rlottie->index, buff, b->w, b->h, b->w * 4);
            rlottie_fix_bitmap(buff, b->w, b->h);
            bitmap_unlock_buffer(b);

            widget_invalidate(WIDGET(rlottie), NULL);
        }
        return RET_REPEAT;
    }

    return RET_REMOVE;
}

static ret_t rlottie_load(widget_t *widget)
{
    uint32_t size = 0;
    char surl[MAX_PATH + 1];
    const char *data = NULL;
    rlottie_t *rlottie = RLOTTIE(widget);
    const char *url = rlottie->url;
    return_value_if_fail(url != NULL, RET_BAD_PARAMS);

    if (strstr(url, "://") == NULL) {
        tk_snprintf(surl, sizeof(surl) - 1, "asset://data/%s", url);
        url = surl;
    }

    data = (const char *)data_reader_read_all(url, &size);
    return_value_if_fail(data != NULL && size > 0, RET_FAIL);

    rlottie->animation = lottie_animation_from_data(data, url, url);
    TKMEM_FREE(data);
    return_value_if_fail(rlottie->animation != NULL, RET_FAIL);

    if (rlottie->timer_id != TK_INVALID_ID) {
        timer_remove(rlottie->timer_id);
        rlottie->timer_id = TK_INVALID_ID;
    }

    if (rlottie->animation != NULL) {
        size_t w = 0;
        size_t h = 0;
        double ratio = system_info()->device_pixel_ratio;
        lottie_animation_get_size(rlottie->animation, &w, &h);

        rlottie->width = w;
        rlottie->height = h;
        rlottie->fps = lottie_animation_get_framerate(rlottie->animation);
        rlottie->duration = lottie_animation_get_duration(rlottie->animation);
        rlottie->total_frames = lottie_animation_get_totalframe(rlottie->animation);
        rlottie->timer_id = widget_add_timer(widget, rlottie_on_timer, 1000 / rlottie->fps);

        log_debug("fps=%lf duration=%lf w=%u h=%u n=%u\n", rlottie->fps, rlottie->duration,
                  rlottie->width, rlottie->height, rlottie->total_frames);

        w = rlottie->width * ratio;
        h = rlottie->height * ratio;
        rlottie->bitmap = bitmap_create_ex(w, h, 0, BITMAP_FMT_RGBA8888);
        return_value_if_fail(rlottie->bitmap != NULL, RET_BAD_PARAMS);

        rlottie->bitmap->flags |= BITMAP_FLAG_OPAQUE;
    }

    return RET_OK;
}

ret_t rlottie_set_url(widget_t *widget, const char *url)
{
    rlottie_t *rlottie = RLOTTIE(widget);
    return_value_if_fail(rlottie != NULL, RET_BAD_PARAMS);

    rlottie->index = 0;
    if (rlottie->animation != NULL) {
        lottie_animation_destroy(rlottie->animation);
        rlottie->animation = NULL;
    }

    if (rlottie->bitmap != NULL) {
        bitmap_destroy(rlottie->bitmap);
        rlottie->bitmap = NULL;
    }

    if (url != NULL) {
        rlottie->url = tk_str_copy(rlottie->url, url);
        rlottie_load(widget);
    } else {
        TKMEM_FREE(rlottie->url);
    }

    return RET_OK;
}

static ret_t rlottie_get_prop(widget_t *widget, const char *name, value_t *v)
{
    rlottie_t *rlottie = RLOTTIE(widget);
    return_value_if_fail(rlottie != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(RLOTTIE_PROP_URL, name)) {
        value_set_str(v, rlottie->url);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t rlottie_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(RLOTTIE_PROP_URL, name)) {
        rlottie_set_url(widget, value_str(v));
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t rlottie_on_destroy(widget_t *widget)
{
    rlottie_t *rlottie = RLOTTIE(widget);
    return_value_if_fail(widget != NULL && rlottie != NULL, RET_BAD_PARAMS);

    rlottie_set_url(widget, NULL);

    return RET_OK;
}

static ret_t rlottie_on_paint_self(widget_t *widget, canvas_t *c)
{
    rlottie_t *rlottie = RLOTTIE(widget);
    return_value_if_fail(rlottie != NULL, RET_BAD_PARAMS);

    if (rlottie->bitmap != NULL) {
        bitmap_t *b = rlottie->bitmap;
        double ratio = system_info()->device_pixel_ratio;

        if (b->w > widget->w * ratio || b->h > widget->h * ratio) {
            rect_t dst = rect_init(0, 0, widget->w, widget->h);
            canvas_draw_image_ex(c, b, IMAGE_DRAW_SCALE_DOWN, &dst);
        } else {
            int32_t cx = widget->w >> 1;
            int32_t cy = widget->h >> 1;
            canvas_draw_icon(c, rlottie->bitmap, cx, cy);
        }
    }

    return RET_OK;
}

static ret_t rlottie_on_event(widget_t *widget, event_t *e)
{
    rlottie_t *rlottie = RLOTTIE(widget);
    return_value_if_fail(widget != NULL && rlottie != NULL, RET_BAD_PARAMS);

    (void)rlottie;

    return RET_OK;
}

const char *s_rlottie_properties[] = {RLOTTIE_PROP_URL, NULL};

TK_DECL_VTABLE(rlottie) = {.size = sizeof(rlottie_t),
                           .type = WIDGET_TYPE_RLOTTIE,
                           .clone_properties = s_rlottie_properties,
                           .persistent_properties = s_rlottie_properties,
                           .parent = TK_PARENT_VTABLE(widget),
                           .create = rlottie_create,
                           .on_paint_self = rlottie_on_paint_self,
                           .set_prop = rlottie_set_prop,
                           .get_prop = rlottie_get_prop,
                           .on_event = rlottie_on_event,
                           .on_destroy = rlottie_on_destroy
                          };

widget_t *rlottie_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(rlottie), x, y, w, h);
    rlottie_t *rlottie = RLOTTIE(widget);
    return_value_if_fail(rlottie != NULL, NULL);
    rlottie_play(widget);

    return widget;
}

ret_t rlottie_play(widget_t *widget)
{
    rlottie_t *rlottie = RLOTTIE(widget);
    return_value_if_fail(rlottie != NULL, RET_BAD_PARAMS);

    rlottie->running = TRUE;

    return RET_OK;
}

ret_t rlottie_pause(widget_t *widget)
{
    rlottie_t *rlottie = RLOTTIE(widget);
    return_value_if_fail(rlottie != NULL, RET_BAD_PARAMS);

    rlottie->running = FALSE;

    return RET_OK;
}

ret_t rlottie_stop(widget_t *widget)
{
    rlottie_t *rlottie = RLOTTIE(widget);
    return_value_if_fail(rlottie != NULL, RET_BAD_PARAMS);

    rlottie->index = 0;
    rlottie->running = FALSE;

    return RET_OK;
}

widget_t *rlottie_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, rlottie), NULL);

    return widget;
}
