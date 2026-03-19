#include "awtk.h"
#include "usb_camera_decoder.h"

extern usb_camera_decoder_t *jpeg_decoder_create(const uint8_t *buff, uint32_t buff_size);

typedef usb_camera_decoder_t *(*usb_camera_decoder_create_t)(const uint8_t *buff, uint32_t buff_size);
static const usb_camera_decoder_create_t s_decoder_create_list[] = {
    NULL,
#ifdef WITH_JPEP_DECODE
    jpeg_decoder_create,
#endif
};


usb_camera_decoder_t *usb_camera_decoder_create(const uint8_t *buff, uint32_t buff_size)
{
    uint32_t i = 0;
    usb_camera_decoder_t *decoder = NULL;
    return_value_if_fail(buff != NULL && buff_size > 0, NULL);
    for (i = 0; i < ARRAY_SIZE(s_decoder_create_list); i++) {
        usb_camera_decoder_create_t fun = s_decoder_create_list[i];
        if (fun != NULL) {
            decoder = fun(buff, buff_size);
            if (decoder != NULL) {
                return decoder;
            }
        }
    }
    return NULL;
}

ret_t usb_camera_decoder_destroy(usb_camera_decoder_t *decoder)
{
    return_value_if_fail(decoder != NULL, RET_BAD_PARAMS);
    return decoder->destroy(decoder);
}

uint32_t usb_camera_decoder_get_width(usb_camera_decoder_t *decoder)
{
    return_value_if_fail(decoder != NULL, 0);
    return decoder->get_width(decoder);
}

uint32_t usb_camera_decoder_get_height(usb_camera_decoder_t *decoder)
{
    return_value_if_fail(decoder != NULL, 0);
    return decoder->get_height(decoder);
}

uint32_t usb_camera_decoder_get_bpp(usb_camera_decoder_t *decoder)
{
    return_value_if_fail(decoder != NULL, 0);
    return decoder->get_bpp(decoder);
}

ret_t usb_camera_decoder_read_data(usb_camera_decoder_t *decoder, uint8_t *dst, uint32_t dst_size, uint32_t line_len, bitmap_format_t dst_format, bool_t is_mirror, bool_t is_reverse)
{
    return_value_if_fail(decoder != NULL && dst != NULL && dst_size > 0, RET_BAD_PARAMS);
    return decoder->read_data(decoder, dst, dst_size, line_len, dst_format, is_mirror, is_reverse);
}
