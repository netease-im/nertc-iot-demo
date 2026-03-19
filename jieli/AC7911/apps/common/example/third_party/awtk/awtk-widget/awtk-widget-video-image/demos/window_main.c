#include "awtk.h"
#include "../src/video_image_register.h"
#include "../src/video_image/video_image.h"

static ret_t on_close(void *ctx, event_t *e)
{
    tk_quit();

    return RET_OK;
}

ret_t start_animation_end(void *ctx, event_t *e)
{
    if (e->type == EVT_ANIM_END) {
        value_t v;
        value_set_bool(&v, TRUE);
        widget_set_prop(WIDGET(ctx), WIDGET_PROP_VISIBLE, &v);
    }

    return RET_OK;
}

ret_t start_animation_start(void *ctx, event_t *e)
{
    if (e->type == EVT_ANIM_START) {
        widget_set_prop_bool(WIDGET(ctx), WIDGET_PROP_VISIBLE, FALSE);
    }

    return RET_OK;
}

// #define VIDEO_IMAGE_FUNC 1

#ifdef VIDEO_IMAGE_FUNC

#include "lcd/lcd_mem.h"
#include "lcd/lcd_mem_special.h"

ret_t video_image_dispose_image(void *ctx, bitmap_t *bitmap)
{
    bitmap->w = 0;
    bitmap->h = 0;
    bitmap->format = 0;
    bitmap->buffer = NULL;
    return RET_OK;
}
/* 这个是 pc 的 AGGE 模式下才是这样设置 offline buffer ，其他模式慎用 */
ret_t video_image_init_image(void *ctx, bitmap_t *bitmap, uint32_t w, uint32_t h, uint32_t channels, uint32_t line_length, bitmap_format_t format)
{
    canvas_t *c = widget_get_canvas(window_manager());
    lcd_t *lcd = c->lcd;
    lcd_mem_special_t *special = (lcd_mem_special_t *)lcd;
    bitmap_format_t lcd_format = lcd_get_desired_bitmap_format(lcd);
    uint32_t lcd_channels = bitmap_get_bpp_of_format(lcd_format);
    if (lcd->w != w || lcd->h != h || lcd_channels != channels || lcd_format != format || line_length != special->lcd_mem->line_length) {
        assert(!"lcd->w != w || lcd->h != h || lcd_channels != channels || lcd_format != format || line_length != lcd_line_length");
        return RET_FAIL;
    }
    /* 把 offline buffer 设置到 bitmap 上面，让序列帧的位图数据保持到 offline buffer 上面。 */
    bitmap->w = lcd->w;
    bitmap->h = lcd->h;
    bitmap->format = special->lcd_mem->format;
    bitmap->buffer = special->lcd_mem->offline_gb;
    graphic_buffer_attach(special->lcd_mem->offline_gb, special->lcd_mem->offline_fb, w, h);
    bitmap_set_line_length(bitmap, tk_max(lcd_channels * w, special->lcd_mem->line_length));

    return RET_OK;
}
#endif

static ret_t on_key_replay(void *ctx, event_t *e)
{
    static float_t playback_rate = 1.0f;
    key_event_t *evt = (key_event_t *)e;
    if (evt->key == TK_KEY_r) {
        video_image_replay(WIDGET(ctx));
    } else if (evt->key == TK_KEY_s) {
        video_image_stop(WIDGET(ctx));
    } else if (evt->key == TK_KEY_p) {
        video_image_play(WIDGET(ctx));
    } else if (evt->key == TK_KEY_n) {
        video_image_pause(WIDGET(ctx));
    } else if (evt->key == TK_KEY_LEFT) {
        playback_rate *= 0.5f;
        video_image_set_playback_rate(WIDGET(ctx), playback_rate);
    } else if (evt->key == TK_KEY_RIGHT) {
        playback_rate *= 2.0f;
        video_image_set_playback_rate(WIDGET(ctx), playback_rate);
    }
    return RET_OK;
}

/**
 * 初始化
 */
ret_t application_init(void)
{
    window_manager_set_show_fps(window_manager(), TRUE);
    video_image_register();

    widget_t *win = window_open("main");

    widget_t *label = widget_lookup_by_type(win, "label", TRUE);
    widget_t *video_image = widget_lookup_by_type(win, WIDGET_TYPE_VIDEO_IMAGE, TRUE);

    widget_on(window_manager(), EVT_KEY_DOWN, on_key_replay, video_image);

#ifdef VIDEO_IMAGE_FUNC
    /* 由于会把数据直接绘制到 offline 上面了，所以 video_image 控件不需要调用绘制函数把 bitmap 绘制到 lcd 上面。 */
    video_image_set_draw_video_image(video_image, FALSE);
    /*
     * 让视频数据直接画在 lcd 的 offline 上面，减少一次整屏的贴图的拷贝数据，极限提高效率的做法
     * 但是在初始化视频位图的时候可能会出现每个平台不一样的情况，所以必须要了解清楚才可以使用。
     * 还有更高效的拷贝到 lcd 的 olineline 上面，这个会有更多的限制，请慎用。
     */
    video_image_set_init_bitmap_func(video_image, video_image_init_image, NULL);
    /*
     * 一定要在设置 video_name 之前设置 init_bitmap 函数，
     * 需要注意的是，如果 xml 有设置 video_name 的话，就需要在 video_image_set_video_name 函数后才可以设置 dispose_image 函数。
     * 因为在 video_image_set_video_name 会创建 bitmap，由于 xml 中有设置 video_name，在执行这段代码前就会创建了一个 bitmap了，
     * 所以需要通过原来默认的方法释放该 bitmap。
     * 如果想避免这个情况，可以不在 xml 中写入 video_name，通过代码来设置 video_name。
     */
    video_image_set_video_name(video_image, "video_12");
    video_image_set_dispose_image_func(video_image, video_image_dispose_image, NULL);
#endif

    widget_on(video_image, EVT_ANIM_START, start_animation_start, label);  //设置开场动画播放完后的回调
    widget_on(video_image, EVT_ANIM_END, start_animation_end, label);  //设置开场动画播放完后的回调

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
