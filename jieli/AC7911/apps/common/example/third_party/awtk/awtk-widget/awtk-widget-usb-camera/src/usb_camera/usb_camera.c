/**
 * File:   usb_camera.c
 * Author: AWTK Develop Team
 * Brief:  USB 摄像头控件
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
 * 2020-06-17 Luo ZhiMing <luozhiming@zlg.cn> created
 *
 */

// #undef WITH_WIDGET_TYPE_CHECK

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "usb_camera.h"
#include "base/widget_vtable.h"

ret_t usb_camera_set_camera_id(widget_t *widget, uint32_t camera_id)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);
    return_value_if_fail(!usb_camera->is_open, RET_FAIL);

    usb_camera->camera_id = camera_id;

    return RET_OK;
}

ret_t usb_camera_set_camera_width(widget_t *widget, uint32_t width)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);
    return_value_if_fail(!usb_camera->is_open, RET_FAIL);

    usb_camera->camera_width = width;

    return RET_OK;
}

ret_t usb_camera_set_camera_height(widget_t *widget, uint32_t height)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);
    return_value_if_fail(!usb_camera->is_open, RET_FAIL);

    usb_camera->camera_height = height;

    return RET_OK;
}

ret_t usb_camera_set_camera_mirror(widget_t *widget, bool_t mirror)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);
    return_value_if_fail(!usb_camera->is_open, RET_FAIL);

    usb_camera->mirror = mirror;

    return usb_camera_set_mirror(usb_camera->p_device, mirror);
    ;
}

static ret_t usb_camera_get_prop(widget_t *widget, const char *name,
                                 value_t *v)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL && name != NULL && v != NULL,
                         RET_BAD_PARAMS);

    if (tk_str_eq(USB_CAMERA_PROP_CAMERA_ID, name)) {
        value_set_uint32(v, usb_camera->camera_id);
        return RET_OK;
    } else if (tk_str_eq(USB_CAMERA_PROP_CAMERA_WIDTH, name)) {
        value_set_uint32(v, usb_camera->camera_width);
        return RET_OK;
    } else if (tk_str_eq(USB_CAMERA_PROP_CAMERA_HEIGHT, name)) {
        value_set_uint32(v, usb_camera->camera_height);
        return RET_OK;
    } else if (tk_str_eq(USB_CAMERA_PROP_MIRROR, name)) {
        value_set_bool(v, usb_camera->mirror);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t usb_camera_set_prop(widget_t *widget, const char *name,
                                 const value_t *v)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(widget != NULL && name != NULL && v != NULL,
                         RET_BAD_PARAMS);

    if (tk_str_eq(USB_CAMERA_PROP_CAMERA_ID, name)) {
        usb_camera_set_camera_id(widget, value_uint32(v));
        return RET_OK;
    } else if (tk_str_eq(USB_CAMERA_PROP_CAMERA_WIDTH, name)) {
        usb_camera_set_camera_width(widget, value_uint32(v));
        return RET_OK;
    } else if (tk_str_eq(USB_CAMERA_PROP_CAMERA_HEIGHT, name)) {
        usb_camera_set_camera_height(widget, value_uint32(v));
        return RET_OK;
    } else if (tk_str_eq(USB_CAMERA_PROP_MIRROR, name)) {
        usb_camera_set_camera_mirror(widget, value_bool(v));
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t usb_camera_on_destroy(widget_t *widget)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);
    usb_camera->is_play = FALSE;
    usb_camera->is_open = FALSE;

    slist_deinit(&usb_camera->devices_info_list);
    slist_deinit(&usb_camera->devices_ratio_list);

    if (usb_camera->p_device != NULL) {
        usb_camera_close_device(usb_camera->p_device);
        usb_camera->p_device = NULL;
    }

    return mutable_image_on_destroy(widget);
}

static ret_t usb_camera_on_paint_self(widget_t *widget, canvas_t *c)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);

    if (usb_camera->is_play && usb_camera->is_open) {
        return mutable_image_on_paint_self(widget, c);
    } else {
        return RET_OK;
    }
}

static ret_t usb_camera_on_event(widget_t *widget, event_t *e)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(widget != NULL && usb_camera != NULL, RET_BAD_PARAMS);

    (void)usb_camera;

    return RET_OK;
}

const char *s_usb_camera_properties[] = {
    USB_CAMERA_PROP_CAMERA_ID, USB_CAMERA_PROP_CAMERA_WIDTH,
    USB_CAMERA_PROP_CAMERA_HEIGHT, USB_CAMERA_PROP_MIRROR, NULL
};

TK_DECL_VTABLE(usb_camera) = {.size = sizeof(usb_camera_t),
                              .type = WIDGET_TYPE_USB_CAMERA,
                              .clone_properties = s_usb_camera_properties,
                              .persistent_properties = s_usb_camera_properties,
                              .get_parent_vt = TK_GET_PARENT_VTABLE(mutable_image),
                              .create = usb_camera_create,
                              .on_paint_self = usb_camera_on_paint_self,
                              .set_prop = usb_camera_set_prop,
                              .get_prop = usb_camera_get_prop,
                              .on_event = usb_camera_on_event,
                              .on_destroy = usb_camera_on_destroy
                             };

widget_t *usb_camera_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget =
        widget_create(parent, TK_REF_VTABLE(usb_camera), x, y, w, h);
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, NULL);

    usb_camera->mirror = FALSE;
    usb_camera->camera_width = 0;
    usb_camera->camera_height = 0;

    slist_init(&usb_camera->devices_info_list,
               usb_camera_devices_info_list_destroy,
               usb_camera_devices_info_list_compare);
    slist_init(&usb_camera->devices_ratio_list,
               usb_camera_device_ratio_list_destroy,
               usb_camera_device_ratio_list_compare);

    mutable_image_init(widget);
    return widget;
}

widget_t *usb_camera_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, usb_camera), NULL);

    return widget;
}

const usb_camera_drive_info_t *usb_camera_enum_all_drives(widget_t *widget, uint32_t *ret_len)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL && ret_len != NULL, NULL);
    return usb_camera_device_get_drive_list(ret_len);
}

ret_t usb_camera_set_global_drive_index(widget_t *widget, uint32_t index)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_OK);
    return usb_camera_device_set_global_drive_index(index);
}

slist_t *usb_camera_enum_all_devices(widget_t *widget)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, NULL);

    slist_deinit(&usb_camera->devices_info_list);
    usb_camera_get_all_devices_info(&usb_camera->devices_info_list);

    return &usb_camera->devices_info_list;
}

slist_t *usb_camera_enum_device_all_ratio(widget_t *widget)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, NULL);
    return_value_if_fail(usb_camera->p_device == NULL, NULL);

    slist_deinit(&usb_camera->devices_ratio_list);
    usb_camera_get_devices_ratio_list_for_device_id(
        usb_camera->camera_id, &usb_camera->devices_ratio_list);

    return &usb_camera->devices_ratio_list;
}

ret_t usb_camera_set_camera_width_and_height(widget_t *widget, uint32_t width,
        uint32_t height)
{
    void *p_device = NULL;
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);
    return_value_if_fail(usb_camera->is_open, RET_FAIL);

    p_device = usb_camera_set_ratio(usb_camera->p_device, width, height);
    return_value_if_fail(p_device != NULL, RET_FAIL);
    usb_camera->p_device = p_device;
    return_value_if_fail(
        usb_camera_get_ratio(usb_camera->p_device, &usb_camera->camera_width,
                             &usb_camera->camera_height) == RET_OK,
        RET_FAIL);

    widget_resize(widget, usb_camera->camera_width, usb_camera->camera_height);

    return_value_if_fail(
        mutable_image_set_prepare_image(widget, usb_camera_prepare_image_fun,
                                        usb_camera->p_device) == RET_OK,
        RET_FAIL);

    return_value_if_fail(
        mutable_image_set_create_image(widget, usb_camera_prepare_image_create_image_fun,
                                       usb_camera->p_device) == RET_OK,
        RET_FAIL);

    return RET_OK;
}

ret_t usb_camera_open(widget_t *widget)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);
    return_value_if_fail(usb_camera->p_device == NULL, RET_FAIL);
    return_value_if_fail(!usb_camera->is_open, RET_FAIL);

    usb_camera->p_device = usb_camera_open_device_with_width_and_height(
                               usb_camera->camera_id, usb_camera->mirror, usb_camera->camera_width,
                               usb_camera->camera_height);
    return_value_if_fail(usb_camera->p_device != NULL, RET_FAIL);

    return_value_if_fail(
        usb_camera_get_ratio(usb_camera->p_device, &usb_camera->camera_width,
                             &usb_camera->camera_height) == RET_OK,
        RET_FAIL);

    widget_resize(widget, usb_camera->camera_width, usb_camera->camera_height);

    return_value_if_fail(
        mutable_image_set_prepare_image(widget, usb_camera_prepare_image_fun,
                                        usb_camera->p_device) == RET_OK,
        RET_FAIL);

    return_value_if_fail(
        mutable_image_set_create_image(widget, usb_camera_prepare_image_create_image_fun,
                                       usb_camera->p_device) == RET_OK,
        RET_FAIL);
    usb_camera->is_open = TRUE;
    return RET_OK;
}

bool_t usb_camera_is_open(widget_t *widget)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, FALSE);

    return usb_camera->is_open;
}

bool_t usb_camera_is_play(widget_t *widget)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, FALSE);

    return usb_camera->is_play;
}

ret_t usb_camera_play(widget_t *widget)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);
    return_value_if_fail(usb_camera_is_open(widget), RET_BAD_PARAMS);

    usb_camera->is_play = TRUE;

    return RET_OK;
}

ret_t usb_camera_stop(widget_t *widget)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);
    usb_camera->is_play = FALSE;

    return RET_OK;
}

ret_t usb_camera_close(widget_t *widget)
{
    usb_camera_t *usb_camera = USB_CAMERA(widget);
    return_value_if_fail(usb_camera != NULL, RET_BAD_PARAMS);
    usb_camera->is_play = FALSE;
    usb_camera->is_open = FALSE;

    if (usb_camera->p_device != NULL) {
        usb_camera_close_device(usb_camera->p_device);
        usb_camera->p_device = NULL;
    }

    return mutable_image_on_destroy(widget);
}
