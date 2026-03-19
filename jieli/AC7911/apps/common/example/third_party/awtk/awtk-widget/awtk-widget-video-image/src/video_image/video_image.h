/**
 * File:   video_image.h
 * Author: AWTK Develop Team
 * Brief:  帧间差异视频播放控件
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
 * 2020-06-18 Luo ZhiMing <luozhiming@zlg.cn> created
 *
 */

#ifndef TK_VIDEO_IMAGE_H
#define TK_VIDEO_IMAGE_H

#include "base/widget.h"
#include "tkc/types_def.h"

BEGIN_C_DECLS

typedef ret_t (*video_image_dispose_image_t)(void *ctx, bitmap_t *bitmap);
typedef ret_t (*video_image_to_image_begin_t)(void *ctx, bitmap_t *bitmap);
typedef ret_t (*video_image_draw_image_t)(void *ctx, bitmap_t *bitmap, canvas_t *c);
typedef ret_t (*video_image_to_image_end_t)(void *ctx, bitmap_t *bitmap, bool_t is_get_frame);
typedef ret_t (*video_image_init_image_t)(void *ctx, bitmap_t *bitmap, uint32_t w, uint32_t h,
        uint32_t channels, uint32_t line_length,
        bitmap_format_t format, uint16_t flags);

/**
 * @class video_image_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 帧间差异视频播放控件
 *
 * 由于该控件使用的视频是特别资源，需要 diff_image_to_video_gen.exe 工具生成的
 * 所以详细用法请看 awtk-widget-calendar-month/docs/video_image控件用法.md。
 *
 * 在xml中使用"video\_image"标签创建video\_image控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <video_image x="0" y="0" w="200" h="200" />
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <video_image>
 *   <style name="default">
 *     <normal />
 *   </style>
 * </video_image>
 * ```
 */
typedef struct _video_image_t {
    widget_t widget;

    /**
     * @property {char*} video_name
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 视频名字。
     */
    char *video_name;

    /**
     * @property {bool_t} auto_play
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 是否自动播放。
     */
    bool_t auto_play;

    /**
     * @property {bool_t} loop
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 是否循环播放。
     */
    bool_t loop;

    /**
     * @property {bool_t} need_replay
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 是否优化重播效率（设置loop为true时该属性默认为true）。
     */
    bool_t need_replay;

    /**
     * @property {bool_t} draw_video_image
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 是否自动绘制视频贴图数据。（默认为TRUE，该成员变量是为了某一些特殊场景使用）
     */
    bool_t draw_video_image;

    /**
     * @property {uint32_t} delay_play
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 延迟播放。
     */
    uint32_t delay_play;

    /**
     * @property {void*} init_image_ctx
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 自定义初始化序列帧位图函数上下文
     */
    void *init_image_ctx;
    /**
     * @property {video_image_init_image_t} init_image
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 自定义初始化序列帧位图函数（如果该参数为NULL，会使用默认的位图创建方法）
     */
    video_image_init_image_t init_image;

    /**
     * @property {void*} dispose_image_ctx
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 自定义释放序列帧位图函数上下文
     */
    void *dispose_image_ctx;
    /**
     * @property {video_image_dispose_image_t} dispose_image
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 自定义释放序列帧位图函数（如果该参数为NULL，会使用默认的位图释放方法）
     */
    video_image_dispose_image_t dispose_image;

    /**
     * @property {void*} draw_image_ctx
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 自定义绘制序列帧位图函数上下文
     */
    void *draw_image_ctx;
    /**
     * @property {video_image_draw_image_t} draw_image
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 自定义绘制序列帧位图函数（如果该参数为NULL，会使用默认的位图绘制方法）
     */
    video_image_draw_image_t draw_image;

    /**
     * @property {void*} to_image_begin_ctx
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 解码序列帧前回调函数上下文
     */
    void *to_image_begin_ctx;
    /**
     * @property {video_image_to_image_begin_t} to_image_begin
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 解码序列帧前回调函数
     */
    video_image_to_image_begin_t to_image_begin;

    /**
     * @property {void*} to_image_end_ctx
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 解码序列帧后回调函数上下文
     */
    void *to_image_end_ctx;
    /**
     * @property {video_image_to_image_end_t} to_image_end
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 解码序列帧后回调函数
     */
    video_image_to_image_end_t to_image_end;
    /**
     * @property {float_t} playback_rate
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 播放速度（默认为1）
     */
    float_t playback_rate;

    /*private*/
    bool_t is_done;
    bool_t is_ready;
    bool_t is_playing;
    bool_t is_get_frame;

    bitmap_t bitmap;
    uint32_t timer_id;
    uint32_t frame_interval;
    uint32_t frame_number_max;
    uint32_t frame_number_curr;

    uint32_t old_bitmap_h;
    uint32_t old_bitmap_line_length;
    uint8_t *first_bitmap_data;

    asset_info_t *video_asset_info;
    unsigned char *max_data_decompress;
} video_image_t;

/**
 * @method video_image_create
 * @annotation ["constructor", "scriptable"]
 * 创建video_image对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} video_image对象。
 */
widget_t *video_image_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method video_image_cast
 * 转换为video_image对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget video_image对象。
 *
 * @return {widget_t*} video_image对象。
 */
widget_t *video_image_cast(widget_t *widget);

/**
 * @method video_image_set_video_name
 * 设置视频名字。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {const char*} video_name 视频名字。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_video_name(widget_t *widget, const char *video_name);

/**
 * @method video_image_set_auto_play
 * 设置是否自动播放。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} auto_play 是否自动播放。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_auto_play(widget_t *widget, bool_t auto_play);

/**
 * @method video_image_set_delay_play
 * 设置延迟播放。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {uint32_t} delay_play 延迟播放。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_delay_play(widget_t *widget, uint32_t delay_play);

/**
 * @method video_image_set_loop
 * 设置循环播放。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} loop 循环播放。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_loop(widget_t *widget, bool_t loop);

/**
 * @method video_image_play
 * 播放序列帧。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_play(widget_t *widget);

/**
 * @method video_image_stop
 * 终止播放序列帧。
 * 备注：下次调用 video_image_play ，则是重头开始播放
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_stop(widget_t *widget);

/**
 * @method video_image_pause
 * 停止播放序列帧。
 * 备注：下次调用 video_image_play ，则是在当前停止的帧开始播放。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_pause(widget_t *widget);

/**
 * @method video_image_replay
 * 马上重新播放序列帧。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_replay(widget_t *widget);

/**
 * @method video_image_set_need_replay
 * 设置是否需要重新播放序列帧模式。
 * 备注：（把第一帧保存下来，然后下次重播的时候，就不需要重新解析序列帧了，提高运行效率）
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} need_replay 是否需要循环播放模式。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_need_replay(widget_t *widget, bool_t need_replay);

/**
 * @method video_image_set_playback_rate
 * 设置当前播放速度。
 * 备注：序列帧没有跳帧功能，所以播放速度会受限解码和渲染速度，无法突破解码和渲染速度的限制。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {float_t} playback_rate 播放速度（1 为正常播放速度，小于 1 播放速度会变慢，大于 1 播放速度会变快）。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_playback_rate(widget_t *widget, float_t playback_rate);

/**
 * @method video_image_get_playback_rate
 * 获取当前播放速度
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {float_t} 返回播放速度。
 */
float_t video_image_get_playback_rate(widget_t *widget);

/**
 * @method video_image_set_draw_video_image
 * 设置是否自动绘制视频贴图数据。（默认为TRUE，该成员变量是为了某一些特殊场景使用）
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} draw_video_image 是否自动绘制视频贴图数据。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_draw_video_image(widget_t *widget, bool_t draw_video_image);

/**
 * @method video_image_set_init_bitmap_func
 * 设置自定义初始化序列帧位图函数（如果该参数为NULL，会使用默认的位图创建方法）
 * 如果返回失败的话，就会使用默认的方法创建位图。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {video_image_init_image_t} init_image 初始化序列帧位图函数。
 * @param {void*} init_image_ctx 上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_init_bitmap_func(widget_t *widget, video_image_init_image_t init_image,
                                       void *init_image_ctx);

/**
 * @method video_image_set_dispose_image_func
 * 设置自定义释放序列帧位图函数（如果该参数为NULL，会使用默认的位图释放方法）
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {video_image_dispose_image_t} dispose_image 释放序列帧位图函数。
 * @param {void*} dispose_image_ctx 上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_dispose_image_func(widget_t *widget,
        video_image_dispose_image_t dispose_image,
        void *dispose_image_ctx);

/**
 * @method video_image_set_draw_image_func
 * 设置自定义绘制序列帧位图函数（如果该参数为NULL，会使用默认的位图绘制方法）
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {video_image_draw_image_t} draw_image 绘制序列帧位图函数。
 * @param {void*} draw_image_ctx 上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_draw_image_func(widget_t *widget, video_image_draw_image_t draw_image,
                                      void *draw_image_ctx);

/**
 * @method video_image_set_to_image_begin_func
 * 设置解码序列帧前回调函数
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {video_image_to_image_begin_t} to_image_begin 解码序列帧前回调函数。
 * @param {void*} to_image_begin_ctx 解码序列帧前回调函数上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_to_image_begin_func(widget_t *widget,
        video_image_to_image_begin_t to_image_begin,
        void *to_image_begin_ctx);

/**
 * @method video_image_set_to_image_end_func
 * 设置解码序列帧后回调函数
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {video_image_to_image_end_t} to_image_end 解码序列帧后回调函数。
 * @param {void*} to_image_end_ctx 解码序列帧后回调函数上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_set_to_image_end_func(widget_t *widget, video_image_to_image_end_t to_image_end,
                                        void *to_image_end_ctx);

#define VIDEO_IMAGE_PROP_LOOP "loop"
#define VIDEO_IMAGE_PROP_VIDEO_NAME "video_name"
#define VIDEO_IMAGE_PROP_AUTO_PLAY "auto_play"
#define VIDEO_IMAGE_PROP_NEED_REPLAY "need_replay"
#define VIDEO_IMAGE_PROP_DELAY_PLAY "delay_play"
#define VIDEO_IMAGE_PROP_PLAYBACK_RETE "playback_rate"
#define VIDEO_IMAGE_PROP_DRAW_VIDEO_IMAGE "draw_video_image"

#define WIDGET_TYPE_VIDEO_IMAGE "video_image"

#define VIDEO_IMAGE(widget) ((video_image_t*)(video_image_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(video_image);

END_C_DECLS

#endif /*TK_VIDEO_IMAGE_H*/
