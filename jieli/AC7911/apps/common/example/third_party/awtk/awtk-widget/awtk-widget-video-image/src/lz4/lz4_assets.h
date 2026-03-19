
#ifndef TK_LZ4_ASSETS_H
#define TK_LZ4_ASSETS_H

#ifdef __cplusplus
extern "C" {
#endif

void lz4_decompress_doubleBufferr_buffer(unsigned char *out_buff,
        long out_buff_length,
        unsigned char *in_buff,
        long in_buff_length);

long lz4_compress_doubleBuffer_buffer(unsigned char *out_buff,
                                      long out_buff_length,
                                      const unsigned char *in_buff,
                                      long in_buff_length);

#ifdef __cplusplus
}
#endif

#endif /*TK_LZ4_ASSETS_H*/
