#ifndef _DUER_RECORD_H_
#define _DUER_RECORD_H_

#include "duer_common.h"
#include "fs/fs.h"
#include "os/os_api.h"

#define DUER_REC_LIMIT_MIN_MS				(5000) //录音时间最低限制
#define AUDIO_CODING_OPUS   "opus"

#define DUER_REC_TYPE           AUDIO_CODING_OPUS
// #define MY_REC_TYPE           AUDIO_CODING_SPEEX

#define DUER_CBUF_SIZE	1024*4
#define DUER_REC_AUDIO_LEN		168

extern int ai_mic_is_busy(void);
extern int ai_mic_rec_close(void);
extern int mic_rec_pram_init(/* const char **name,  */u32 enc_type, u8 opus_type, u16(*speech_send)(u8 *buf, u16 len), u16 frame_num, u16 cbuf_size);
extern int ai_mic_rec_start(void);


typedef struct {
    char *buf;
    u8 *read_buf;
    struct server *enc_server;
    cbuffer_t cbuf;
    bool is_audio_record_open;
} duer_rec_t;



extern int duer_rec_start(void);
extern int duer_rec_stop(void);

#endif
