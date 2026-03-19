#include "lz4_image_loader.h"
#include "../../src/base/image_manager.h"

static uint32_t image_loader_bitmap_get_bpp_of_format(bitmap_format_t format)
{
    switch (format) {
    case BITMAP_FMT_RGBA8888:
    case BITMAP_FMT_ABGR8888:
    case BITMAP_FMT_BGRA8888:
    case BITMAP_FMT_ARGB8888:
        return 4;
    case BITMAP_FMT_RGB565:
    case BITMAP_FMT_BGR565:
        return 2;
    case BITMAP_FMT_RGB888:
    case BITMAP_FMT_BGR888:
        return 3;
    case BITMAP_FMT_GRAY:
        return 1;
    default:
        break;
    }

    return 0;
}

static ret_t image_loader_lz4_load(image_loader_t *l, const asset_info_t *asset,
                                   bitmap_t *image)
{
    uint32_t bpp = 0;
    ret_t ret = RET_OK;
    uint8_t *data = NULL;
    long out_buf_length = 0;
    bitmap_header_t *header = NULL;

    return_value_if_fail(l != NULL && image != NULL, RET_BAD_PARAMS);

    if (asset->subtype != ASSET_TYPE_IMAGE_LZ4) {
        return RET_NOT_IMPL;
    }
    header = (bitmap_header_t *)asset->data;
    bpp = image_loader_bitmap_get_bpp_of_format(header->format);
    header = (bitmap_header_t *)asset->data;
    out_buf_length = header->w * header->h * bpp;

    ret = bitmap_init(image, header->w, header->h, header->format, NULL);

#ifdef TK_GRAPHIC_BUFFER_H
    data = bitmap_lock_buffer_for_write(image);
#else
    data = (uint8_t *)(image->data);
#endif
    lz4_decompress_doubleBufferr_buffer(data, out_buf_length, header->data,
                                        asset->size);
#ifdef TK_GRAPHIC_BUFFER_H
    bitmap_unlock_buffer(image);
#endif
    return ret;
}

static const image_loader_t lz4_image_loader = {.load = image_loader_lz4_load};

image_loader_t *image_loader_lz4()
{
    return (image_loader_t *)&lz4_image_loader;
}
