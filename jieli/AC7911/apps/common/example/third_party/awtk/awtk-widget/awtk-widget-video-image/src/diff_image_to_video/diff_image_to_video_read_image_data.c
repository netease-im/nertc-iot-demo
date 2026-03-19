#include <stdlib.h>
#include <string.h>

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "../lz4/lz4_assets.h"
#include "diff_image_to_video_types.h"
#include "diff_image_to_video_read_image_data.h"

#define MEMCPY_TINY_MAX_LENGTH   128
#define IS_ADDRESS_ALIGN_4(addr) !((((size_t)(addr)) & 0x3) | 0x0)

typedef void *(*memcpy_fun_t)(void *dst, void *src, uint32_t size);

static void *memcpy_tiny(void *dst, const void *src, uint32_t len)
{
    if (len < MEMCPY_TINY_MAX_LENGTH && IS_ADDRESS_ALIGN_4(dst) && IS_ADDRESS_ALIGN_4(src)) {
        register unsigned char *dd = (unsigned char *)dst + len;
        register const unsigned char *ss = (const unsigned char *)src + len;
        switch (len) {
        case 128:
            *((int *)(dd - 128)) = *((int *)(ss - 128));
        case 124:
            *((int *)(dd - 124)) = *((int *)(ss - 124));
        case 120:
            *((int *)(dd - 120)) = *((int *)(ss - 120));
        case 116:
            *((int *)(dd - 116)) = *((int *)(ss - 116));
        case 112:
            *((int *)(dd - 112)) = *((int *)(ss - 112));
        case 108:
            *((int *)(dd - 108)) = *((int *)(ss - 108));
        case 104:
            *((int *)(dd - 104)) = *((int *)(ss - 104));
        case 100:
            *((int *)(dd - 100)) = *((int *)(ss - 100));
        case 96:
            *((int *)(dd - 96)) = *((int *)(ss - 96));
        case 92:
            *((int *)(dd - 92)) = *((int *)(ss - 92));
        case 88:
            *((int *)(dd - 88)) = *((int *)(ss - 88));
        case 84:
            *((int *)(dd - 84)) = *((int *)(ss - 84));
        case 80:
            *((int *)(dd - 80)) = *((int *)(ss - 80));
        case 76:
            *((int *)(dd - 76)) = *((int *)(ss - 76));
        case 72:
            *((int *)(dd - 72)) = *((int *)(ss - 72));
        case 68:
            *((int *)(dd - 68)) = *((int *)(ss - 68));
        case 64:
            *((int *)(dd - 64)) = *((int *)(ss - 64));
        case 60:
            *((int *)(dd - 60)) = *((int *)(ss - 60));
        case 56:
            *((int *)(dd - 56)) = *((int *)(ss - 56));
        case 52:
            *((int *)(dd - 52)) = *((int *)(ss - 52));
        case 48:
            *((int *)(dd - 48)) = *((int *)(ss - 48));
        case 44:
            *((int *)(dd - 44)) = *((int *)(ss - 44));
        case 40:
            *((int *)(dd - 40)) = *((int *)(ss - 40));
        case 36:
            *((int *)(dd - 36)) = *((int *)(ss - 36));
        case 32:
            *((int *)(dd - 32)) = *((int *)(ss - 32));
        case 28:
            *((int *)(dd - 28)) = *((int *)(ss - 28));
        case 24:
            *((int *)(dd - 24)) = *((int *)(ss - 24));
        case 20:
            *((int *)(dd - 20)) = *((int *)(ss - 20));
        case 16:
            *((int *)(dd - 16)) = *((int *)(ss - 16));
        case 12:
            *((int *)(dd - 12)) = *((int *)(ss - 12));
        case  8:
            *((int *)(dd - 8)) = *((int *)(ss - 8));
        case  4:
            *((int *)(dd - 4)) = *((int *)(ss - 4));
            break;
        case 127:
            *((int *)(dd - 127)) = *((int *)(ss - 127));
        case 123:
            *((int *)(dd - 123)) = *((int *)(ss - 123));
        case 119:
            *((int *)(dd - 119)) = *((int *)(ss - 119));
        case 115:
            *((int *)(dd - 115)) = *((int *)(ss - 115));
        case 111:
            *((int *)(dd - 111)) = *((int *)(ss - 111));
        case 107:
            *((int *)(dd - 107)) = *((int *)(ss - 107));
        case 103:
            *((int *)(dd - 103)) = *((int *)(ss - 103));
        case 99:
            *((int *)(dd - 99)) = *((int *)(ss - 99));
        case 95:
            *((int *)(dd - 95)) = *((int *)(ss - 95));
        case 91:
            *((int *)(dd - 91)) = *((int *)(ss - 91));
        case 87:
            *((int *)(dd - 87)) = *((int *)(ss - 87));
        case 83:
            *((int *)(dd - 83)) = *((int *)(ss - 83));
        case 79:
            *((int *)(dd - 79)) = *((int *)(ss - 79));
        case 75:
            *((int *)(dd - 75)) = *((int *)(ss - 75));
        case 71:
            *((int *)(dd - 71)) = *((int *)(ss - 71));
        case 67:
            *((int *)(dd - 67)) = *((int *)(ss - 67));
        case 63:
            *((int *)(dd - 63)) = *((int *)(ss - 63));
        case 59:
            *((int *)(dd - 59)) = *((int *)(ss - 59));
        case 55:
            *((int *)(dd - 55)) = *((int *)(ss - 55));
        case 51:
            *((int *)(dd - 51)) = *((int *)(ss - 51));
        case 47:
            *((int *)(dd - 47)) = *((int *)(ss - 47));
        case 43:
            *((int *)(dd - 43)) = *((int *)(ss - 43));
        case 39:
            *((int *)(dd - 39)) = *((int *)(ss - 39));
        case 35:
            *((int *)(dd - 35)) = *((int *)(ss - 35));
        case 31:
            *((int *)(dd - 31)) = *((int *)(ss - 31));
        case 27:
            *((int *)(dd - 27)) = *((int *)(ss - 27));
        case 23:
            *((int *)(dd - 23)) = *((int *)(ss - 23));
        case 19:
            *((int *)(dd - 19)) = *((int *)(ss - 19));
        case 15:
            *((int *)(dd - 15)) = *((int *)(ss - 15));
        case 11:
            *((int *)(dd - 11)) = *((int *)(ss - 11));
        case  7:
            *((int *)(dd - 7)) = *((int *)(ss - 7));
        case  3:
            *((short *)(dd - 3)) = *((short *)(ss - 3));
            dd[-1] = ss[-1];
            break;
        case 126:
            *((int *)(dd - 126)) = *((int *)(ss - 126));
        case 122:
            *((int *)(dd - 122)) = *((int *)(ss - 122));
        case 118:
            *((int *)(dd - 118)) = *((int *)(ss - 118));
        case 114:
            *((int *)(dd - 114)) = *((int *)(ss - 114));
        case 110:
            *((int *)(dd - 110)) = *((int *)(ss - 110));
        case 106:
            *((int *)(dd - 106)) = *((int *)(ss - 106));
        case 102:
            *((int *)(dd - 102)) = *((int *)(ss - 102));
        case 98:
            *((int *)(dd - 98)) = *((int *)(ss - 98));
        case 94:
            *((int *)(dd - 94)) = *((int *)(ss - 94));
        case 90:
            *((int *)(dd - 90)) = *((int *)(ss - 90));
        case 86:
            *((int *)(dd - 86)) = *((int *)(ss - 86));
        case 82:
            *((int *)(dd - 82)) = *((int *)(ss - 82));
        case 78:
            *((int *)(dd - 78)) = *((int *)(ss - 78));
        case 74:
            *((int *)(dd - 74)) = *((int *)(ss - 74));
        case 70:
            *((int *)(dd - 70)) = *((int *)(ss - 70));
        case 66:
            *((int *)(dd - 66)) = *((int *)(ss - 66));
        case 62:
            *((int *)(dd - 62)) = *((int *)(ss - 62));
        case 58:
            *((int *)(dd - 58)) = *((int *)(ss - 58));
        case 54:
            *((int *)(dd - 54)) = *((int *)(ss - 54));
        case 50:
            *((int *)(dd - 50)) = *((int *)(ss - 50));
        case 46:
            *((int *)(dd - 46)) = *((int *)(ss - 46));
        case 42:
            *((int *)(dd - 42)) = *((int *)(ss - 42));
        case 38:
            *((int *)(dd - 38)) = *((int *)(ss - 38));
        case 34:
            *((int *)(dd - 34)) = *((int *)(ss - 34));
        case 30:
            *((int *)(dd - 30)) = *((int *)(ss - 30));
        case 26:
            *((int *)(dd - 26)) = *((int *)(ss - 26));
        case 22:
            *((int *)(dd - 22)) = *((int *)(ss - 22));
        case 18:
            *((int *)(dd - 18)) = *((int *)(ss - 18));
        case 14:
            *((int *)(dd - 14)) = *((int *)(ss - 14));
        case 10:
            *((int *)(dd - 10)) = *((int *)(ss - 10));
        case  6:
            *((int *)(dd - 6)) = *((int *)(ss - 6));
        case  2:
            *((short *)(dd - 2)) = *((short *)(ss - 2));
            break;
        case 125:
            *((int *)(dd - 125)) = *((int *)(ss - 125));
        case 121:
            *((int *)(dd - 121)) = *((int *)(ss - 121));
        case 117:
            *((int *)(dd - 117)) = *((int *)(ss - 117));
        case 113:
            *((int *)(dd - 113)) = *((int *)(ss - 113));
        case 109:
            *((int *)(dd - 109)) = *((int *)(ss - 109));
        case 105:
            *((int *)(dd - 105)) = *((int *)(ss - 105));
        case 101:
            *((int *)(dd - 101)) = *((int *)(ss - 101));
        case 97:
            *((int *)(dd - 97)) = *((int *)(ss - 97));
        case 93:
            *((int *)(dd - 93)) = *((int *)(ss - 93));
        case 89:
            *((int *)(dd - 89)) = *((int *)(ss - 89));
        case 85:
            *((int *)(dd - 85)) = *((int *)(ss - 85));
        case 81:
            *((int *)(dd - 81)) = *((int *)(ss - 81));
        case 77:
            *((int *)(dd - 77)) = *((int *)(ss - 77));
        case 73:
            *((int *)(dd - 73)) = *((int *)(ss - 73));
        case 69:
            *((int *)(dd - 69)) = *((int *)(ss - 69));
        case 65:
            *((int *)(dd - 65)) = *((int *)(ss - 65));
        case 61:
            *((int *)(dd - 61)) = *((int *)(ss - 61));
        case 57:
            *((int *)(dd - 57)) = *((int *)(ss - 57));
        case 53:
            *((int *)(dd - 53)) = *((int *)(ss - 53));
        case 49:
            *((int *)(dd - 49)) = *((int *)(ss - 49));
        case 45:
            *((int *)(dd - 45)) = *((int *)(ss - 45));
        case 41:
            *((int *)(dd - 41)) = *((int *)(ss - 41));
        case 37:
            *((int *)(dd - 37)) = *((int *)(ss - 37));
        case 33:
            *((int *)(dd - 33)) = *((int *)(ss - 33));
        case 29:
            *((int *)(dd - 29)) = *((int *)(ss - 29));
        case 25:
            *((int *)(dd - 25)) = *((int *)(ss - 25));
        case 21:
            *((int *)(dd - 21)) = *((int *)(ss - 21));
        case 17:
            *((int *)(dd - 17)) = *((int *)(ss - 17));
        case 13:
            *((int *)(dd - 13)) = *((int *)(ss - 13));
        case  9:
            *((int *)(dd - 9)) = *((int *)(ss - 9));
        case  5:
            *((int *)(dd - 5)) = *((int *)(ss - 5));
        case  1:
            dd[-1] = ss[-1];
            break;
        case 0:
        default:
            break;
        }
        return dd;
    }
    return NULL;
}

#define DIFF_IMAGE_TO_VIDEO_CREATE_IMAGE_DATA_FOR_ALIG(data_for_image_diff_data_list_lz4_dec, image_data_length, image_data, line_length, is_su, memcpy_fun, ptr) {                          \
  unsigned int y = 0;                                                                                                                                                                        \
  unsigned int data_length = 0;                                                                                                                                                              \
  unsigned int width_length = 0;                                                                                                                                                             \
  image_diff_rect_t* diff_rect = NULL;                                                                                                                                                       \
  unsigned int image_data_list_point = 0;                                                                                                                                                    \
  do {                                                                                                                                                                                       \
    unsigned int image_x_point = 0;                                                                                                                                                          \
    unsigned int image_y_point = 0;                                                                                                                                                          \
    unsigned int data_width_length = 0;                                                                                                                                                      \
    unsigned int tmp_image_data_length = 0;                                                                                                                                                  \
    diff_rect = (image_diff_rect_t*)(data_for_image_diff_data_list_lz4_dec + image_data_list_point);                                                                                         \
    image_data_list_point += sizeof(image_diff_rect_t);                                                                                                                                      \
                                                                                                                                                                                             \
    width_length = line_length;                                                                                                                                                              \
    image_x_point = diff_rect->x << (sizeof(ptr) / 2);                                                                                                                                       \
    image_y_point = diff_rect->y * width_length;                                                                                                                                             \
    data_width_length = diff_rect->width << (sizeof(ptr) / 2);                                                                                                                               \
    data_length = diff_rect->line_length * diff_rect->height;                                                                                                                                \
                                                                                                                                                                                             \
    for (y = 0; y < diff_rect->height; y++) {                                                                                                                                                \
      if (data_width_length < MEMCPY_TINY_MAX_LENGTH) {                                                                                                                                      \
        void* dst = memcpy_tiny(image_data + image_x_point + image_y_point, data_for_image_diff_data_list_lz4_dec + image_data_list_point + tmp_image_data_length, data_width_length);       \
        if (dst == NULL) {                                                                                                                                                                   \
          memcpy_fun((ptr *)(image_data + image_x_point + image_y_point), (ptr *)(data_for_image_diff_data_list_lz4_dec + image_data_list_point + tmp_image_data_length), diff_rect->width); \
        }                                                                                                                                                                                    \
      } else {                                                                                                                                                                               \
        memcpy_fun((ptr *)(image_data + image_x_point + image_y_point), (ptr *)(data_for_image_diff_data_list_lz4_dec + image_data_list_point + tmp_image_data_length), diff_rect->width);   \
      }                                                                                                                                                                                      \
      image_y_point += width_length;                                                                                                                                                         \
      tmp_image_data_length += diff_rect->line_length;                                                                                                                                       \
    }                                                                                                                                                                                        \
    image_data_list_point += data_length;                                                                                                                                                    \
    is_su = 0;                                                                                                                                                                               \
  } while (image_data_list_point < image_data_length);                                                                                                                                       \
}

static int diff_image_to_video_create_image_data_for_align32bit(
    unsigned char *data_for_image_diff_data_list_lz4_dec,
    unsigned int image_data_length, unsigned char *image_data, unsigned int line_length)
{
    int is_su = -1;
    DIFF_IMAGE_TO_VIDEO_CREATE_IMAGE_DATA_FOR_ALIG(data_for_image_diff_data_list_lz4_dec, image_data_length, image_data, line_length, is_su, tk_memcpy32, uint32_t);
    return is_su;
}

static int diff_image_to_video_create_image_data_for_align16bit(
    unsigned char *data_for_image_diff_data_list_lz4_dec,
    unsigned int image_data_length, unsigned char *image_data, unsigned int line_length)
{
    int is_su = -1;
    DIFF_IMAGE_TO_VIDEO_CREATE_IMAGE_DATA_FOR_ALIG(data_for_image_diff_data_list_lz4_dec, image_data_length, image_data, line_length, is_su, tk_memcpy16, uint16_t);
    return is_su;
}

static int diff_image_to_video_create_image_data_for_noalign(
    unsigned char *data_for_image_diff_data_list_lz4_dec,
    unsigned int image_data_length, unsigned char *image_data,
    unsigned int image_width, unsigned int channels, unsigned int line_length)
{
    int is_su = -1;
    unsigned int y = 0;
    unsigned int data_length = 0;
    unsigned int width_length = 0;
    unsigned int data_width_length = 0;

    image_diff_rect_t *diff_rect = NULL;
    unsigned int image_data_list_point = 0;

    do {
        unsigned int image_x_point = 0;
        unsigned int image_y_point = 0;
        unsigned int tmp_image_data_length = 0;

        diff_rect = (image_diff_rect_t *)(data_for_image_diff_data_list_lz4_dec + image_data_list_point);
        image_data_list_point += sizeof(image_diff_rect_t);

        data_length = diff_rect->height * diff_rect->line_length;

        width_length = line_length;
        image_x_point = diff_rect->x * channels;
        image_y_point = diff_rect->y * width_length;
        data_width_length = channels * diff_rect->width;

        for (y = 0; y < diff_rect->height; y++) {
            memcpy(image_data + image_x_point + image_y_point, data_for_image_diff_data_list_lz4_dec + image_data_list_point + tmp_image_data_length, data_width_length);
            image_y_point += width_length;
            tmp_image_data_length += diff_rect->line_length;
        }

        image_data_list_point += data_length;

        is_su = 0;
    } while (image_data_list_point < image_data_length);
    return is_su;
}

static int diff_image_to_video_get_lz4_data_to_diff_image_data(
    unsigned char *lz4_data, unsigned int lz4_length,
    unsigned int image_data_length, unsigned char *image_data,
    unsigned int image_width, unsigned int channels, unsigned int line_length,
    unsigned char *max_data_decompress)
{

    int is_su = -1;

    if (max_data_decompress == NULL) {
        return is_su;
    }

    lz4_decompress_doubleBufferr_buffer(max_data_decompress,
                                        image_data_length, lz4_data, lz4_length);

    if (image_data_length > 0) {
        if (channels == 2) {
            is_su = diff_image_to_video_create_image_data_for_align16bit(
                        max_data_decompress, image_data_length, image_data,
                        line_length);
        } else if (channels == 4) {
            is_su = diff_image_to_video_create_image_data_for_align32bit(
                        max_data_decompress, image_data_length, image_data,
                        line_length);
        } else {
            is_su = diff_image_to_video_create_image_data_for_noalign(
                        max_data_decompress, image_data_length, image_data,
                        image_width, channels, line_length);
        }
    }

    return is_su;
}

static int diff_image_to_video_read_frame_image(
    const unsigned char *file_data, unsigned int file_data_length,
    video_info_t *video_info, frame_info_t *frame_info,
    unsigned char *last_image_data, unsigned int line_length,
    unsigned char *max_data_decompress)
{
    int is_su = -1;
    unsigned char *data_com = NULL;

    if (file_data_length >= frame_info->data_length + frame_info->start_point) {
        unsigned int width = video_info->width;
        unsigned int height = video_info->height;
        unsigned int channels = video_info->channels;

        data_com = (unsigned char *)(file_data + frame_info->start_point);

        switch (frame_info->compress_type) {
        case compress_type_lz4:
            memset(last_image_data, 0x0, height * line_length);
            lz4_decompress_doubleBufferr_buffer(last_image_data,
                                                height * line_length,
                                                data_com, frame_info->data_length);
            break;
        case compress_type_diff_lz4:
            is_su = diff_image_to_video_get_lz4_data_to_diff_image_data(
                        data_com, frame_info->data_length, frame_info->data_decompress_length,
                        last_image_data, width, channels, line_length, max_data_decompress);
            break;
        case compress_type_no_change:
        default:
            break;
        }

        is_su = 0;
    }

    return is_su;
}

static int diff_image_to_video_read_video_info_and_frame_info(
    const unsigned char *file_data, unsigned int file_data_length,
    video_info_t **video_info, frame_info_t **frame_info_list,
    unsigned int frame_number)
{
    unsigned int file_point = 0;

    if (file_data == NULL || file_data_length < sizeof(video_info_t)) {
        return -1;
    }

    *video_info = (video_info_t *)file_data;
    file_point += sizeof(video_info_t);

    if (file_data_length < (sizeof(video_info_t) + ((*video_info)->frame_number *
                            sizeof(frame_info_t)))) {
        return -1;
    }
    file_point += (sizeof(frame_info_t) * frame_number);
    *frame_info_list = (frame_info_t *)(file_data + file_point);

    return 0;
}

int diff_image_to_video_read_image_data(
    const unsigned char *file_data, unsigned int file_data_length,
    unsigned int frame_number, unsigned char *last_image_data, unsigned int line_length,
    unsigned int width, unsigned int height, unsigned int channels, unsigned char **max_data_decompress)
{
    int is_su = -1;
    video_info_t *video_info;
    frame_info_t *frame_info_list;

    if (file_data == NULL || file_data_length < sizeof(video_info_t)) {
        return is_su;
    }

    if (diff_image_to_video_read_video_info_and_frame_info(
            file_data, file_data_length, &video_info, &frame_info_list,
            frame_number) == 0) {
        if (width != video_info->width || height != video_info->height || channels != video_info->channels) {
            assert(!"bitmap's width != video's width or bitmap's height != video's height or bitmap's channels != video's channels");
        } else if (line_length != video_info->line_length) {
            assert(!"line_length != video_info->line_length");
        } else {
            is_su = diff_image_to_video_read_frame_image(
                        file_data, file_data_length, video_info, frame_info_list,
                        last_image_data, line_length, *max_data_decompress);
        }
    }
    return is_su;
}

int diff_image_to_video_read_image_init(const unsigned char *file_data,
                                        unsigned int file_data_length,
                                        unsigned char **max_data_decompress)
{
    int i = 0;
    video_info_t *video_info = NULL;
    unsigned int max_data_decompress_length = 0;
    unsigned int file_point = sizeof(video_info_t);

    if (file_data == NULL || file_data_length < sizeof(video_info_t)) {
        return -1;
    }
    video_info = (video_info_t *)file_data;

    if (*max_data_decompress != NULL) {
        TKMEM_FREE(*max_data_decompress);
    }

    if (file_data_length < (sizeof(video_info_t) +
                            (video_info->frame_number * sizeof(frame_info_t)))) {
        return -1;
    }
    file_point += sizeof(frame_info_t);

    for (i = 1; i < video_info->frame_number; i++) {
        frame_info_t *frame_info_list = (frame_info_t *)(file_data + file_point);
        if (frame_info_list->compress_type == compress_type_diff_lz4 &&
            frame_info_list->data_decompress_length > max_data_decompress_length) {
            max_data_decompress_length = frame_info_list->data_decompress_length;
        }
        file_point += sizeof(frame_info_t);
    }

    if (max_data_decompress_length > 0) {
        *max_data_decompress = TKMEM_CALLOC(sizeof(unsigned char), max_data_decompress_length);
    }

    if (max_data_decompress_length > 0 && *max_data_decompress == NULL) {
        return -2;
    }

    return 0;
}

void diff_image_to_video_read_image_dispose(unsigned char **max_data_decompress)
{
    if (*max_data_decompress != NULL) {
        TKMEM_FREE(*max_data_decompress);
    }
}
