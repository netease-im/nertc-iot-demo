#ifndef __USB_CAMERA_DECODE_H__
#define __USB_CAMERA_DECODE_H__

#include "awtk.h"

BEGIN_C_DECLS

typedef struct _usb_camera_decoder_t usb_camera_decoder_t;

typedef ret_t (*usb_camera_decoder_destroy_t)(usb_camera_decoder_t *decoder);
typedef uint32_t (*usb_camera_decoder_get_width_t)(usb_camera_decoder_t *decoder);
typedef uint32_t (*usb_camera_decoder_get_height_t)(usb_camera_decoder_t *decoder);
typedef uint32_t (*usb_camera_decoder_get_bpp_t)(usb_camera_decoder_t *decoder);
typedef ret_t (*usb_camera_decoder_read_data_t)(usb_camera_decoder_t *decoder, uint8_t *dst, uint32_t dst_size, uint32_t line_len, bitmap_format_t dst_format, bool_t is_mirror, bool_t is_reverse);

struct _usb_camera_decoder_t {
    void *handle;

    usb_camera_decoder_destroy_t destroy;
    usb_camera_decoder_get_bpp_t get_bpp;
    usb_camera_decoder_get_width_t get_width;
    usb_camera_decoder_get_height_t get_height;
    usb_camera_decoder_read_data_t read_data;
};

/* in */
usb_camera_decoder_t *usb_camera_decoder_create(const uint8_t *buff, uint32_t buff_size);
ret_t usb_camera_decoder_destroy(usb_camera_decoder_t *decoder);
uint32_t usb_camera_decoder_get_width(usb_camera_decoder_t *decoder);
uint32_t usb_camera_decoder_get_height(usb_camera_decoder_t *decoder);
uint32_t usb_camera_decoder_get_bpp(usb_camera_decoder_t *decoder);
ret_t usb_camera_decoder_read_data(usb_camera_decoder_t *decoder, uint8_t *dst, uint32_t dst_size, uint32_t line_len, bitmap_format_t dst_format, bool_t is_mirror, bool_t is_reverse);


END_C_DECLS


#endif