#include "awtk.h"
#include "../src/usb_camera/usb_camera.h"

#define CAMERA_RATIO_FORMAT "%d*%d*%d"

static widget_t *btn_refresh = NULL;
static widget_t *drive_list = NULL;
static widget_t *camera_ratios_list = NULL;
static widget_t *camera_devices_list = NULL;

static ret_t on_camera_set_camera_ratio_list(widget_t *camera);

static ret_t on_camera_open(void *ctx, event_t *e)
{
    ret_t ret = RET_OK;
    widget_t *camera = WIDGET(ctx);
    return_value_if_fail(camera != NULL, RET_BAD_PARAMS);
    return_value_if_fail(!usb_camera_is_open(camera), RET_BAD_PARAMS);

    on_camera_set_camera_ratio_list(camera);
    // 如果读取摄像头分辨率列表失败的话，这里可以手动设置摄像头的分辨率。
    // usb_camera_set_camera_width(camera, 1280);
    // usb_camera_set_camera_height(camera, 720);

    ret = usb_camera_open(camera);
    if (ret != RET_OK) {
        dialog_toast("open camera fail", 3000);
        return ret;
    }

    widget_set_enable(btn_refresh, FALSE);
    widget_set_enable(drive_list, FALSE);
    widget_set_enable(camera_ratios_list, FALSE);
    widget_set_enable(camera_devices_list, FALSE);

    return RET_OK;
}

static ret_t on_camera_play(void *ctx, event_t *e)
{
    widget_t *camera = WIDGET(ctx);
    return_value_if_fail(camera != NULL, RET_BAD_PARAMS);

    if (!usb_camera_is_open(camera)) {
        dialog_toast(" do not open the camera", 2000);
        return RET_BAD_PARAMS;
    }

    return usb_camera_play(camera);
}

static ret_t on_camera_stop(void *ctx, event_t *e)
{
    widget_t *camera = WIDGET(ctx);
    return_value_if_fail(camera != NULL, RET_BAD_PARAMS);

    if (!usb_camera_is_play(camera)) {
        dialog_toast(" do not play the camera", 2000);
        return RET_BAD_PARAMS;
    }

    return usb_camera_stop(camera);
}

static ret_t on_camera_close(void *ctx, event_t *e)
{
    ret_t ret = RET_OK;
    widget_t *camera = WIDGET(ctx);
    return_value_if_fail(camera != NULL, RET_BAD_PARAMS);

    if (!usb_camera_is_open(camera)) {
        dialog_toast(" do not open the camera", 2000);
        return RET_BAD_PARAMS;
    }

    ret = usb_camera_close(camera);

    widget_set_enable(btn_refresh, TRUE);
    widget_set_enable(drive_list, TRUE);
    widget_set_enable(camera_ratios_list, TRUE);
    widget_set_enable(camera_devices_list, TRUE);

    return ret;
}


static ret_t combo_box_set_camera_list(void *ctx, const void *data)
{
    usb_camera_device_info_t *device_info = (usb_camera_device_info_t *)data;

    combo_box_append_option(camera_devices_list, device_info->camera_id, device_info->camera_name);

    return RET_OK;
}

static ret_t on_camera_load_camera_list(void *ctx, event_t *e)
{
    widget_t *camera = WIDGET(ctx);
    slist_t *list = usb_camera_enum_all_devices(camera);
    return_value_if_fail(list != NULL && camera != NULL, RET_BAD_PARAMS);

    if (list != NULL) {
        combo_box_reset_options(camera_devices_list);
        slist_foreach(list, combo_box_set_camera_list, NULL);
        combo_box_set_selected_index(camera_devices_list, 0);
        combo_box_set_selected_index(camera_ratios_list, 0);
    }

    return RET_OK;
}

static ret_t combo_box_set_camera_ratios_list(void *ctx, const void *data)
{
    char ratio_text[MAX_PATH];
    uint32_t *id = (uint32_t *)ctx;
    usb_camera_ratio_t *ratio_info = (usb_camera_ratio_t *)data;

    if (ratio_info->width != 0 && ratio_info->height != 0 && ratio_info->bpp != 0) {
        memset(ratio_text, 0x0, MAX_PATH);
        tk_snprintf(ratio_text, MAX_PATH, CAMERA_RATIO_FORMAT, ratio_info->width, ratio_info->height, ratio_info->bpp);

        combo_box_append_option(camera_ratios_list, *id, ratio_text);
        *id = *id + 1;
    }

    return RET_OK;
}

static ret_t on_camera_load_camera_ratio_list(void *ctx, event_t *e)
{
    uint32_t id = 0;
    slist_t *list = NULL;
    widget_t *camera = WIDGET(ctx);
    return_value_if_fail(camera != NULL && camera_devices_list != NULL, RET_BAD_PARAMS);

    if (usb_camera_set_camera_id(camera, combo_box_get_value(camera_devices_list)) != RET_OK) {
        dialog_toast(" do not find the camera", 2000);
        return RET_NOT_FOUND;
    }

    list = usb_camera_enum_device_all_ratio(camera);
    return_value_if_fail(list != NULL, RET_BAD_PARAMS);

    if (list != NULL) {
        combo_box_reset_options(camera_ratios_list);
        slist_foreach(list, combo_box_set_camera_ratios_list, &id);
        combo_box_set_selected_index(camera_ratios_list, 0);
    }

    return RET_OK;
}

static ret_t on_camera_set_camera_ratio_list(widget_t *camera)
{
    uint32_t w = 0;
    uint32_t h = 0;
    uint32_t bpp = 0;
    const char *ratio_text = NULL;
    return_value_if_fail(camera_ratios_list != NULL && camera != NULL, RET_BAD_PARAMS);

    ratio_text = combo_box_get_text(camera_ratios_list);

    tk_sscanf(ratio_text, CAMERA_RATIO_FORMAT, &w, &h, &bpp);

    usb_camera_set_camera_width(camera, w);
    usb_camera_set_camera_height(camera, h);

    return RET_OK;
}

static ret_t on_set_drive_index(void *ctx, event_t *e)
{
    ret_t ret = RET_OK;
    widget_t *camera = WIDGET(ctx);
    return_value_if_fail(camera != NULL, RET_BAD_PARAMS);
    ret = usb_camera_set_global_drive_index(camera, widget_get_value_int(drive_list));
    if (ret == RET_OK) {
        on_camera_load_camera_list(camera, NULL);
    }
    return RET_OK;
}

static ret_t install_one(void *ctx, const void *iter)
{
    widget_t *camera = WIDGET(ctx);
    widget_t *widget = WIDGET(iter);
    widget_t *win = widget_get_window(widget);

    if (widget->name != NULL) {
        const char *name = widget->name;
        if (tk_str_eq(name, "open")) {
            widget_on(widget, EVT_CLICK, on_camera_open, camera);
        } else if (tk_str_eq(name, "play")) {
            widget_on(widget, EVT_CLICK, on_camera_play, camera);
        } else if (tk_str_eq(name, "stop")) {
            widget_on(widget, EVT_CLICK, on_camera_stop, camera);
        } else if (tk_str_eq(name, "close")) {
            widget_on(widget, EVT_CLICK, on_camera_close, camera);
        } else if (tk_str_eq(name, "refresh")) {
            btn_refresh = widget;
            widget_on(widget, EVT_CLICK, on_camera_load_camera_list, camera);
        } else if (tk_str_eq(name, "camera_list")) {
            camera_devices_list = widget;
            widget_on(widget, EVT_VALUE_CHANGED, on_camera_load_camera_ratio_list, camera);
        } else if (tk_str_eq(name, "ratio_list")) {
            camera_ratios_list = widget;
        } else if (tk_str_eq(name, "drive_list")) {
            drive_list = widget;
            widget_on(widget, EVT_VALUE_CHANGED, on_set_drive_index, camera);
        }
    }

    return RET_OK;
}

/**
 * 初始化
 */
ret_t application_init(void)
{
    uint32_t len = 0;
    widget_t *win = NULL;
    widget_t *camera = NULL;
    const usb_camera_drive_info_t *usb_camera_drive_infos = NULL;
    usb_camera_register();

    win = window_open("main");
    camera = widget_lookup_by_type(win, "usb_camera", TRUE);

    widget_foreach(win, install_one, camera);

    usb_camera_drive_infos = usb_camera_enum_all_drives(camera, &len);
    if (usb_camera_drive_infos != NULL) {
        combo_box_reset_options(drive_list);
        for (uint32_t i = 0; i < len; i++) {
            combo_box_append_option(drive_list, i, usb_camera_drive_infos[i].name);
            combo_box_set_selected_index(drive_list, 0);
        }
    }

    on_camera_load_camera_list(camera, NULL);


    return RET_OK;
}

/**
 * 退出
 */
ret_t application_exit(void)
{
    log_debug("application_exit\n");
    return RET_OK;
}
