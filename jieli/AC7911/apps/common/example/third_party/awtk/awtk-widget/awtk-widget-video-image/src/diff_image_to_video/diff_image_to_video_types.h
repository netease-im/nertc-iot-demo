#ifndef __DIFF_IMAGE_TO_VIDEO_TYPES_H__
#define __DIFF_IMAGE_TO_VIDEO_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

typedef struct _Point_t {
    int x;
    int y;
} Point;

typedef struct _Rect_t {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
} Rect;

typedef enum _image_format_t {
    IMAGE_FMT_RGBA8888 = 0,
    IMAGE_FMT_BGRA8888,
    IMAGE_FMT_RGB565,
    IMAGE_FMT_BGR565,
    IMAGE_FMT_RGB888,
    IMAGE_FMT_BGR888
} image_format_t;

typedef enum _compress_type_t {
    compress_type_lz4 = 0,
    compress_type_diff_lz4,
    compress_type_no_change,
} compress_type_t;

typedef struct _image_diff_rect_t {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    unsigned int line_length;
} image_diff_rect_t;

typedef struct _image_diff_data_t {
    image_diff_rect_t rect;
    unsigned char *data;
} image_diff_data_t;

typedef struct _frame_info_t {
    unsigned int start_point;
    unsigned int data_length;
    unsigned int data_decompress_length;
    compress_type_t compress_type;
} frame_info_t;

#define VIDEO_INFO_HEAD_STRING                 "ZLG_IMAGE_VIDEO"
const static unsigned char s_video_info_head[] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,           //兼容区
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17    //标记开头区
};

#define VIDEO_INFO_HEAD_SIZE                   sizeof(s_video_info_head) + sizeof(VIDEO_INFO_HEAD_STRING)
#define VIDEO_INFO_VERSION                     1

typedef struct _video_info_t {
    unsigned char video_info_head[VIDEO_INFO_HEAD_SIZE];
    unsigned int version;
    unsigned int width;
    unsigned int height;
    unsigned int flags;
    unsigned int delays;
    unsigned int channels;
    unsigned int line_length;
    unsigned int time_length;
    unsigned int frame_number;
    image_format_t frame_image_type;
} video_info_t;

typedef struct _video_head_t {
    video_info_t video_info;

    frame_info_t *frame_info_list;
} video_head_t;

#ifdef __cplusplus
}
#endif

#endif // !__DIFF_IMAGE_TO_VIDEO_TYPES_H__
