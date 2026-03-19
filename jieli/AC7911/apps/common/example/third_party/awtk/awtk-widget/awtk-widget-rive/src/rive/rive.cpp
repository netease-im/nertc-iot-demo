/**
 * File:   rive.c
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

#include "rive.h"
#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/time_now.h"
#include "tkc/data_reader_factory.h"
#include "vgcanvas_renderer.hpp"

#include "rive/file.hpp"
#include "rive/artboard.hpp"
#include "rive/animation/linear_animation_instance.hpp"

class RiveInfo
{
private:
    uint64_t last_time;

    rive::File *file;
    rive::Artboard *artboard;
    rive::LinearAnimationInstance *animation;

public:
    RiveInfo(void)
    {
        file = NULL;
        artboard = NULL;
        animation = NULL;
    }

    ~RiveInfo(void)
    {
        this->reset();
    }

    ret_t reset(void)
    {
        delete animation;
        animation = nullptr;

        delete file;
        file = NULL;

        return RET_OK;
    }

    ret_t load(const char *url)
    {
        uint32_t size = 0;
        uint8_t *bytes = (uint8_t *)data_reader_read_all(url, &size);
        return_value_if_fail(bytes != NULL, RET_BAD_PARAMS);

        auto reader = rive::BinaryReader(bytes, (size_t)size);
        auto result = rive::File::import(reader, &file);
        if (result != rive::ImportResult::success) {
            TKMEM_FREE(bytes);
            return RET_FAIL;
        }

        this->artboard = file->artboard();
        this->artboard->advance(0.0f);

        auto animation = this->artboard->firstAnimation();
        if (animation) {
            this->animation = new rive::LinearAnimationInstance(animation);
        }
        log_debug("laod %s %d\n", url, this->is_valid());
        return RET_OK;
    }

    ret_t draw(vgcanvas_t *vg, uint32_t w, uint32_t h)
    {
        if (this->is_valid()) {
            vg_renderer_t *vgrender = vg_renderer_create(vg);
            rive::VGCanvasRenderer renderer(vgrender);

            renderer.save();
            renderer.align(rive::Fit::contain, rive::Alignment::center, rive::AABB(0, 0, w, h),
                           artboard->bounds());
            artboard->draw(&renderer);
            renderer.restore();
            vg_renderer_destroy(vgrender);
        }

        return RET_OK;
    }

    ret_t rewind(void)
    {
        if (this->is_valid()) {
            float elapsed = animation->time();

            artboard->advance(-elapsed);
            animation->apply(artboard);
            animation->advance(-elapsed);
        }

        return RET_OK;
    }

    ret_t update_last_time(void)
    {
        last_time = time_now_ms();
        return RET_OK;
    }

    ret_t step(void)
    {
        if (this->is_valid()) {
            uint64_t now = time_now_ms();
            double elapsed = (double)(now - last_time) / 1000.0;

            last_time = now;
            animation->advance(elapsed);
            animation->apply(artboard);
            artboard->advance(elapsed);
        }

        return RET_OK;
    }

    bool_t is_valid(void)
    {
        return (artboard != NULL && animation != NULL);
    }
};

static ret_t rive_on_timer(const timer_info_t *info)
{
    rive_t *rive = RIVE(info->ctx);
    RiveInfo *ainfo = static_cast<RiveInfo *>(rive->info);

    if (ainfo->is_valid() && rive->running) {
        ainfo->step();
        widget_invalidate(WIDGET(rive), NULL);
    } else {
        ainfo->update_last_time();
    }

    return RET_REPEAT;
}

static ret_t rive_load(widget_t *widget)
{
    char surl[MAX_PATH + 1];
    rive_t *rive = RIVE(widget);
    const char *url = rive->url;
    RiveInfo *info = static_cast<RiveInfo *>(rive->info);
    return_value_if_fail(url != NULL, RET_BAD_PARAMS);

    info->reset();
    if (strstr(url, "://") == NULL) {
        tk_snprintf(surl, sizeof(surl) - 1, "asset://data/%s", url);
        url = surl;
    }

    info->load(url);

    if (rive->timer_id != TK_INVALID_ID) {
        timer_remove(rive->timer_id);
        rive->timer_id = TK_INVALID_ID;
    }
    rive->timer_id = widget_add_timer(widget, rive_on_timer, 16);

    return RET_OK;
}

ret_t rive_set_url(widget_t *widget, const char *url)
{
    rive_t *rive = RIVE(widget);
    return_value_if_fail(rive != NULL, RET_BAD_PARAMS);

    rive->index = 0;

    if (url != NULL) {
        rive->url = tk_str_copy(rive->url, url);
        rive_load(widget);
    } else {
        TKMEM_FREE(rive->url);
    }

    return RET_OK;
}

static ret_t rive_get_prop(widget_t *widget, const char *name, value_t *v)
{
    rive_t *rive = RIVE(widget);
    return_value_if_fail(rive != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(RIVE_PROP_URL, name)) {
        value_set_str(v, rive->url);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t rive_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(RIVE_PROP_URL, name)) {
        rive_set_url(widget, value_str(v));
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t rive_on_destroy(widget_t *widget)
{
    rive_t *rive = RIVE(widget);
    return_value_if_fail(widget != NULL && rive != NULL, RET_BAD_PARAMS);
    RiveInfo *info = static_cast<RiveInfo *>(rive->info);

    delete info;
    rive_set_url(widget, NULL);

    return RET_OK;
}

static ret_t rive_on_paint_self(widget_t *widget, canvas_t *c)
{
    rectf_t r;
    rive_t *rive = RIVE(widget);
    RiveInfo *info = static_cast<RiveInfo *>(rive->info);
    vgcanvas_t *vg = canvas_get_vgcanvas(c);
    return_value_if_fail(rive != NULL, RET_BAD_PARAMS);

    r = vg->clip_rect;
    vgcanvas_save(vg);
    vgcanvas_begin_path(vg);
    vgcanvas_translate(vg, c->ox, c->oy);
    info->draw(vg, widget->w, widget->h);
    vgcanvas_restore(vg);
    vgcanvas_clip_rect(vg, r.x, r.y, r.w, r.h);

    return RET_OK;
}

const char *s_rive_properties[] = {RIVE_PROP_URL, NULL};

TK_DECL_VTABLE(rive) = {.size = sizeof(rive_t),
                        .type = WIDGET_TYPE_RIVE,
                        .clone_properties = s_rive_properties,
                        .persistent_properties = s_rive_properties,
                        .parent = TK_PARENT_VTABLE(widget),
                        .create = rive_create,
                        .get_prop = rive_get_prop,
                        .set_prop = rive_set_prop,
                        .on_paint_self = rive_on_paint_self,
                        .on_destroy = rive_on_destroy
                       };

widget_t *rive_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(rive), x, y, w, h);
    rive_t *rive = RIVE(widget);
    return_value_if_fail(rive != NULL, NULL);

    rive->info = new RiveInfo();
    rive_play(widget);

    return widget;
}

ret_t rive_play(widget_t *widget)
{
    rive_t *rive = RIVE(widget);
    return_value_if_fail(rive != NULL, RET_BAD_PARAMS);

    rive->running = TRUE;

    return RET_OK;
}

ret_t rive_pause(widget_t *widget)
{
    rive_t *rive = RIVE(widget);
    return_value_if_fail(rive != NULL, RET_BAD_PARAMS);

    rive->running = FALSE;

    return RET_OK;
}

ret_t rive_stop(widget_t *widget)
{
    rive_t *rive = RIVE(widget);
    return_value_if_fail(rive != NULL, RET_BAD_PARAMS);
    RiveInfo *info = static_cast<RiveInfo *>(rive->info);

    info->rewind();
    info->rewind();
    info->rewind();
    rive->running = FALSE;
    widget_invalidate_force(widget, NULL);

    return RET_OK;
}

widget_t *rive_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, rive), NULL);

    return widget;
}
