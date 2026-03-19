#ifndef _MY_PLATFORM_RECORD_H_
#define _MY_PLATFORM_RECORD_H_

#include "my_platform_common.h"
#include "fs/fs.h"
#include "os/os_api.h"

#define AUDIO_CODING_OPUS   "opus"
#define MY_REC_LIMIT_MIN_MS      (5000)   // 录音时间最低限制
#define MY_REC_TYPE               AUDIO_CODING_OPUS
// #define MY_REC_TYPE           AUDIO_CODING_SPEEX

// #define MY_CBUF_SIZE             (1024 * 4)
#define MY_CBUF_SIZE             (200)
#define MY_REC_AUDIO_LEN          168
#define MY_AUDIO_SAVE_TEST        1
#define MY_AUDIO_SAVE_PATH        "storage/sd0/C/ze.bin"

typedef struct {
    char *buf;
    u8 *read_buf;
    cbuffer_t cbuf;
    struct server *enc_server;
    bool is_audio_record_open;
} my_rec_t;


extern int my_rec_stop(void);
extern int my_rec_start(void);


#endif

