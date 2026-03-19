#include "awtk.h"
#include "jpeglib.h"
#include <setjmp.h>
#include "usb_camera_decoder.h"

typedef struct _JpegErrorMgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
} JpegErrorMgr;

typedef struct _JpegSource {
    struct jpeg_source_mgr pub;
    int skip;
} JpegSource;

typedef struct _JpegState {
    const uint8_t *buff;
    uint32_t buff_size;
    struct jpeg_decompress_struct cinfo; // IJG JPEG codec structure
    JpegErrorMgr jerr; // error processing manager state
    JpegSource source; // memory buffer source
} JpegState;

/////////////////////// Error processing /////////////////////
static void stub(j_decompress_ptr prt) { }

static bool_t fill_input_buffer(j_decompress_ptr prt)
{
    return FALSE;
}

// emulating memory input stream
static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    JpegSource *source = (JpegSource *) cinfo->src;
    if (num_bytes > (long)source->pub.bytes_in_buffer) {
        // We need to skip more data than we have in the buffer.
        // This will force the JPEG library to suspend decoding.
        source->skip = (int)(num_bytes - source->pub.bytes_in_buffer);
        source->pub.next_input_byte += source->pub.bytes_in_buffer;
        source->pub.bytes_in_buffer = 0;
    } else {
        // Skip portion of the buffer
        source->pub.bytes_in_buffer -= num_bytes;
        source->pub.next_input_byte += num_bytes;
        source->skip = 0;
    }
}

static void jpeg_buffer_src(j_decompress_ptr cinfo, JpegSource *source)
{
    cinfo->src = &source->pub;

    // Prepare for suspending reader
    source->pub.init_source = stub;
    source->pub.fill_input_buffer = fill_input_buffer;
    source->pub.skip_input_data = skip_input_data;
    source->pub.resync_to_restart = jpeg_resync_to_restart;
    source->pub.term_source = stub;
    source->pub.bytes_in_buffer = 0; // forces fill_input_buffer on first read

    source->skip = 0;
}

static void error_exit(j_common_ptr cinfo)
{
    JpegErrorMgr *err_mgr = (JpegErrorMgr *)(cinfo->err);

    /* Return control to the setjmp point */
    longjmp(err_mgr->setjmp_buffer, 1);
}


/***************************************************************************
 * following code is for supporting MJPEG image files
 * based on a message of Laurent Pinchart on the video4linux mailing list
 ***************************************************************************/

/* JPEG DHT Segment for YCrCb omitted from MJPEG data */
static unsigned char my_jpeg_odml_dht[0x1a4] = {
    0xff, 0xc4, 0x01, 0xa2,

    0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,

    0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,

    0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04,
    0x04, 0x00, 0x00, 0x01, 0x7d,
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06,
    0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1,
    0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a,
    0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65,
    0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85,
    0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3,
    0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
    0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
    0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4,
    0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa,

    0x11, 0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04,
    0x04, 0x00, 0x01, 0x02, 0x77,
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41,
    0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09,
    0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17,
    0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83,
    0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,
    0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
    0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4,
    0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

/*
 * Parse the DHT table.
 * This code comes from jpeg6b (jdmarker.c).
 */
static int my_jpeg_load_dht(struct jpeg_decompress_struct *info, unsigned char *dht, JHUFF_TBL *ac_tables[], JHUFF_TBL *dc_tables[])
{
    unsigned int length = (dht[2] << 8) + dht[3] - 2;
    unsigned int pos = 4;
    unsigned int count, i;
    int index;

    JHUFF_TBL **hufftbl;
    unsigned char bits[17];
    unsigned char huffval[256] = {0};

    while (length > 16) {
        bits[0] = 0;
        index = dht[pos++];
        count = 0;
        for (i = 1; i <= 16; ++i) {
            bits[i] = dht[pos++];
            count += bits[i];
        }
        length -= 17;

        if (count > 256 || count > length) {
            return -1;
        }

        for (i = 0; i < count; ++i) {
            huffval[i] = dht[pos++];
        }
        length -= count;

        if (index & 0x10) {
            index &= ~0x10;
            hufftbl = &ac_tables[index];
        } else {
            hufftbl = &dc_tables[index];
        }

        if (index < 0 || index >= NUM_HUFF_TBLS) {
            return -1;
        }

        if (*hufftbl == NULL) {
            *hufftbl = jpeg_alloc_huff_table((j_common_ptr)info);
        }
        if (*hufftbl == NULL) {
            return -1;
        }

        memcpy((*hufftbl)->bits, bits, sizeof(*hufftbl)->bits);
        memcpy((*hufftbl)->huffval, huffval, sizeof(*hufftbl)->huffval);
    }

    if (length != 0) {
        return -1;
    }
    return 0;
}

static ret_t jpeg_decode_destroy(JpegState *state)
{
    return_value_if_fail(state != NULL, RET_BAD_PARAMS);
    jpeg_destroy_decompress(&(state->cinfo));
    TKMEM_FREE(state);
    return RET_OK;
}

static void *jpeg_decode_create(const uint8_t *buff, uint32_t buff_size)
{
    ret_t ret = RET_FAIL;
    JpegState *state = NULL;
    return_value_if_fail(buff != NULL && buff_size > 0, NULL);

    state = TKMEM_ZALLOC(JpegState);
    return_value_if_fail(state != NULL, NULL);

    state->buff = buff;
    state->buff_size = buff_size;
    state->cinfo.err = jpeg_std_error(&state->jerr.pub);
    state->jerr.pub.error_exit = error_exit;

    if (setjmp(state->jerr.setjmp_buffer) == 0) {
        jpeg_create_decompress(&state->cinfo);

        jpeg_buffer_src(&state->cinfo, &state->source);
        state->source.pub.next_input_byte = buff;
        state->source.pub.bytes_in_buffer = buff_size;

        if (state->cinfo.src != 0) {
            jpeg_read_header(&state->cinfo, TRUE);

            state->cinfo.scale_num = 1;
            state->cinfo.scale_denom = 1;
            jpeg_calc_output_dimensions(&state->cinfo);
            ret = RET_OK;
        }
    }
    if (ret == RET_OK) {
        return state;
    } else {
        jpeg_decode_destroy(state);
        return NULL;
    }
}

static uint32_t jpeg_decode_get_width(usb_camera_decoder_t *decoder)
{
    return_value_if_fail(decoder != NULL && decoder->handle != NULL, 0);
    return ((JpegState *)(decoder->handle))->cinfo.output_width;
}

static uint32_t jpeg_decode_get_height(usb_camera_decoder_t *decoder)
{
    return_value_if_fail(decoder != NULL && decoder->handle != NULL, 0);
    return ((JpegState *)(decoder->handle))->cinfo.output_height;
}

static uint32_t jpeg_decode_get_bpp(usb_camera_decoder_t *decoder)
{
    return_value_if_fail(decoder != NULL && decoder->handle != NULL, 0);
    return ((JpegState *)(decoder->handle))->cinfo.num_components > 1 ? 3 : 1;
}

void icvCvt_BGR2BGR565(const uint8_t *bgr, int bgr_step,
                       uint8_t *bgr565, int bgr565_step, uint32_t width, uint32_t height)
{
    int i;
    for (; height--;) {
        for (i = 0; i < width; i++, bgr += 3, bgr565 += 2) {
            uint32_t t0 = bgr[0], t1 = bgr[1], t2 = bgr[2];
            *((uint16_t *)bgr565) = ((t0 & 0xF8) << 8) | ((t1 & 0xF8) << 3) | (t2 & 0xF8) >> 3;
        }
        bgr += bgr_step - width * 3;
        bgr565 += bgr565_step - width * 2;
    }
}

void icvCvt_BGR2RGB565(const uint8_t *bgr, int bgr_step,
                       uint8_t *rgb565, int rgb565_step, uint32_t width, uint32_t height)
{
    int i;
    for (; height--;) {
        for (i = 0; i < width; i++, bgr += 3, rgb565 += 2) {
            uint8_t t0 = bgr[0], t1 = bgr[1], t2 = bgr[2];
            *((uint16_t *)rgb565) = ((t2 & 0xF8) << 8) | ((t1 & 0xF8) << 3) | (t0 & 0xF8) >> 3;
        }
        bgr += bgr_step - width * 3;
        rgb565 += rgb565_step - width * 2;
    }
}

void icvCvt_BGR2RGB(const uint8_t *bgr, int bgr_step,
                    uint8_t *rgb, int rgb_step, uint32_t width, uint32_t height)
{
    int i;
    for (; height--;) {
        for (i = 0; i < width; i++, bgr += 3, rgb += 3) {
            uint8_t t0 = bgr[0], t1 = bgr[1], t2 = bgr[2];
            rgb[2] = t0;
            rgb[1] = t1;
            rgb[0] = t2;
        }
        bgr += bgr_step - width * 3;
        rgb += rgb_step - width * 3;
    }
}

void icvCvt_BGR2RGBA(const uint8_t *bgr, int bgr_step,
                     uint8_t *rgba, int rgba_step, uint32_t width, uint32_t height)
{
    int i;
    for (; height--;) {
        for (i = 0; i < width; i++, bgr += 3, rgba += 4) {
            uint8_t t0 = bgr[0], t1 = bgr[1], t2 = bgr[2];
            rgba[0] = t0;
            rgba[1] = t1;
            rgba[2] = t2;
            rgba[3] = 0xFF;
        }
        bgr += bgr_step - width * 3;
        rgba += rgba_step - width * 4;
    }
}

void icvCvt_BGR2BGRA(const uint8_t *bgr, int bgr_step,
                     uint8_t *bgra, int bgra_step, uint32_t width, uint32_t height)
{
    int i;
    for (; height--;) {
        for (i = 0; i < width; i++, bgr += 3, bgra += 4) {
            uint8_t t0 = bgr[0], t1 = bgr[1], t2 = bgr[2];
            bgra[2] = t0;
            bgra[1] = t1;
            bgra[0] = t2;
            bgra[3] = 0xFF;
        }
        bgr += bgr_step - width * 3;
        bgra += bgra_step - width * 4;
    }
}

static ret_t jpeg_decode_read_data(usb_camera_decoder_t *decoder, uint8_t *dst, uint32_t dst_size, uint32_t line_len, bitmap_format_t dst_format, bool_t is_mirror, bool_t is_reverse)
{
    ret_t ret = RET_FAIL;
    JpegState *state = NULL;
    uint32_t width, height, step;
    bool_t color = bitmap_get_bpp_of_format(dst_format) > 1;
    return_value_if_fail(decoder != NULL && dst != NULL && dst_size > 0, RET_BAD_PARAMS);

    state = (JpegState *)decoder->handle;
    width = jpeg_decode_get_width(decoder);
    height = jpeg_decode_get_height(decoder);
    step = line_len > 0 ? line_len :  bitmap_get_bpp_of_format(dst_format) * width;
    return_value_if_fail(width > 0 && height > 0, RET_BAD_PARAMS);

    struct jpeg_decompress_struct *cinfo = &((JpegState *)state)->cinfo;
    JpegErrorMgr *jerr = &((JpegState *)state)->jerr;
    JSAMPARRAY buffer = 0;

    if (setjmp(jerr->setjmp_buffer) == 0) {
        /* check if this is a mjpeg image format */
        if (cinfo->ac_huff_tbl_ptrs[0] == NULL &&
            cinfo->ac_huff_tbl_ptrs[1] == NULL &&
            cinfo->dc_huff_tbl_ptrs[0] == NULL &&
            cinfo->dc_huff_tbl_ptrs[1] == NULL) {
            /* yes, this is a mjpeg image format, so load the correct
            huffman table */
            my_jpeg_load_dht(cinfo, my_jpeg_odml_dht, cinfo->ac_huff_tbl_ptrs, cinfo->dc_huff_tbl_ptrs);
        }

        if (color) {
            if (cinfo->num_components != 4) {
                cinfo->out_color_space = JCS_RGB;
                cinfo->out_color_components = 3;
            } else {
                cinfo->out_color_space = JCS_CMYK;
                cinfo->out_color_components = 4;
                return RET_NOT_IMPL;
            }
        } else {
            if (cinfo->num_components != 4) {
                cinfo->out_color_space = JCS_GRAYSCALE;
                cinfo->out_color_components = 1;
            } else {
                cinfo->out_color_space = JCS_CMYK;
                cinfo->out_color_components = 4;
                return RET_NOT_IMPL;
            }
        }

        jpeg_start_decompress(cinfo);

        buffer = (*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo, JPOOL_IMAGE, width * 4, 1);

        uint8_t *data = is_reverse ? dst + (height - 1) * step : dst;
        for (; height--;) {
            jpeg_read_scanlines(cinfo, buffer, 1);
            if (color) {
                if (cinfo->out_color_components == 3) {
                    memcpy(data, buffer[0], width);
                    switch (dst_format) {
                    case BITMAP_FMT_RGB565:
                        icvCvt_BGR2RGB565(buffer[0], 0, data, 0, width, 1);
                        break;
                    case BITMAP_FMT_BGR565:
                        icvCvt_BGR2BGR565(buffer[0], 0, data, 0, width, 1);
                        break;
                    case BITMAP_FMT_RGB888:
                        memcpy(data, buffer[0], step);
                        break;
                    case BITMAP_FMT_BGR888:
                        icvCvt_BGR2RGB(buffer[0], 0, data, 0, width, 1);
                        break;
                    case BITMAP_FMT_BGRA8888:
                        icvCvt_BGR2BGRA(buffer[0], 0, data, 0, width, 1);
                        break;
                    case BITMAP_FMT_RGBA8888:
                        icvCvt_BGR2RGBA(buffer[0], 0, data, 0, width, 1);
                        break;
                    default:
                        break;
                    }
                }
            } else {
                if (cinfo->out_color_components == 1) {
                    memcpy(data, buffer[0], width);
                }
            }
            if (is_reverse) {
                data -= step;
            } else {
                data += step;
            }
        }
        ret = RET_OK;
        jpeg_finish_decompress(cinfo);
    }

    return ret;
}

static ret_t jpeg_decoder_destroy(usb_camera_decoder_t *decoder)
{
    JpegState *state = NULL;
    return_value_if_fail(decoder != NULL && decoder->handle != NULL, 0);
    state = (JpegState *)decoder->handle;
    TKMEM_FREE(decoder);
    return jpeg_decode_destroy(state);
}

usb_camera_decoder_t *jpeg_decoder_create(const uint8_t *buff, uint32_t buff_size)
{
    usb_camera_decoder_t *decoder = NULL;
    return_value_if_fail(buff != NULL && buff_size > 0, NULL);

    decoder = TKMEM_ZALLOC(usb_camera_decoder_t);
    return_value_if_fail(decoder != NULL, NULL);

    decoder->handle = jpeg_decode_create(buff, buff_size);
    decoder->read_data = jpeg_decode_read_data;
    decoder->get_bpp = jpeg_decode_get_bpp;
    decoder->get_height = jpeg_decode_get_height;
    decoder->get_width = jpeg_decode_get_width;
    decoder->destroy = jpeg_decoder_destroy;

    return decoder;
}
