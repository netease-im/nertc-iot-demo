#include "server/audio_server.h"
#include "server/server_core.h"
#include "generic/circular_buf.h"
#include "os/os_api.h"
#include "app_config.h"

struct audio_pcm_play_t {
    void *dec_server;
    void *cache_buf;
    cbuffer_t cbuf;
    OS_SEM w_sem;
    OS_SEM r_sem;
    u8 block;
    u8 run_flag;
    u8 channel;
    u8 force_double_channel;
#if defined CONFIG_AUDIO_MIX_ENABLE
    u8 inc_step;
    u8 dec_step;
    s16 adjust_step;
    s16 max_step;
    u32 begin_size;
    u32 top_size;
    u32 bottom_size;
    int drop_points;
#endif
};

static void __single_data_copy(s16 *data, int len, u8 ch_num)
{
    s16 *start = (s16 *)((u8 *)data + len / ch_num);
    s16 *end = (s16 *)((u8 *)data + len);

    if (ch_num == 2) {
        for (u32 i = 0; i < len / (ch_num * 2); ++i) {
            *--end = *--start;
            *--end = *start;
        }
    } else if (ch_num == 3) {
        for (u32 i = 0; i < len / (ch_num * 2); ++i) {
            *--end = *--start;
            *--end = *start;
            *--end = *start;
        }
    } else if (ch_num == 4) {
        for (u32 i = 0; i < len / (ch_num * 2); ++i) {
            *--end = *--start;
            *--end = *start;
            *--end = *start;
            *--end = *start;
        }
    }
}

//解码器读取PCM数据
static int audio_pcm_play_vfs_fread(void *priv, void *data, u32 len)
{
    struct audio_pcm_play_t *hdl = (struct audio_pcm_play_t *)priv;
    u32 rlen;

    if (hdl->force_double_channel && hdl->channel == 1) {
        len >>= 1;
    }

    do {
        rlen = cbuf_get_data_size(&hdl->cbuf);
        rlen = rlen > len ? len : rlen;
        if (cbuf_read(&hdl->cbuf, data, rlen) > 0) {
            if (hdl->block) {
                os_sem_set(&hdl->w_sem, 0);
                os_sem_post(&hdl->w_sem);
            }
            break;
        }
        if (!hdl->block) {
            return -2;	//非堵塞
        }
        //此处等待信号量是为了防止解码器因为读不到数而一直空转
        os_sem_pend(&hdl->r_sem, 0);
    } while (hdl->run_flag);

#if defined CONFIG_AUDIO_MIX_ENABLE
    if (hdl->drop_points > 0) {
        hdl->drop_points -= rlen / 2;
        memset(data, 0, rlen);
    }
#endif

    if (hdl->force_double_channel && hdl->channel == 1) {
        rlen <<= 1;
        __single_data_copy((s16 *)data, rlen, 2);
    }

    //返回成功读取的字节数
    return rlen;
}

static const struct audio_vfs_ops pcm_vfs_ops = {
    .fread  = audio_pcm_play_vfs_fread,
};

#if defined CONFIG_AUDIO_MIX_ENABLE

static int pcm_sync(void *priv, u32 data_size, u16 *in_rate, u16 *out_rate)
{
    struct audio_pcm_play_t *hdl = (struct audio_pcm_play_t *)priv;

    data_size = cbuf_get_data_size(&hdl->cbuf);

    if (data_size < hdl->bottom_size) {
        /* putchar('<'); */
        hdl->adjust_step += hdl->inc_step;
        if (hdl->adjust_step < 0) {
            hdl->adjust_step += hdl->inc_step * 2;
        }
    } else if (data_size > hdl->top_size) {
        /* putchar('>'); */
        hdl->adjust_step -= hdl->dec_step;
        if (hdl->adjust_step > 0) {
            hdl->adjust_step -= hdl->dec_step * 2;
        }
    } else {
        /* putchar('='); */
        if (hdl->adjust_step > 0) {
            hdl->adjust_step -= (hdl->adjust_step * hdl->inc_step) / hdl->max_step;
            if (hdl->adjust_step > 0) {
                hdl->adjust_step--;
            }
        } else if (hdl->adjust_step < 0) {
            hdl->adjust_step += ((0 - hdl->adjust_step) * hdl->dec_step) / hdl->max_step;
            if (hdl->adjust_step < 0) {
                hdl->adjust_step++;
            }
        }
    }

    if (hdl->adjust_step < -hdl->max_step) {
        hdl->adjust_step = -hdl->max_step;
        putchar(',');
    } else if (hdl->adjust_step > hdl->max_step) {
        hdl->adjust_step = hdl->max_step;
        putchar('.');
    }

    *out_rate += hdl->adjust_step;

    return 0;
}
#endif

void *audio_pcm_play_open(int sample_rate, u32 frame_size, u32 drop_points, u8 channel, u8 volume, u8 block)
{
    union audio_req req = {0};
    int err;
    u32 cache_buf_size = frame_size ? frame_size * 4 * (sample_rate / 11025 > 0 ? sample_rate / 11025 : 1) : sample_rate * channel; //上层缓冲buf缓冲0.5秒的数据，缓冲太大听感上会有延迟

    struct audio_pcm_play_t *hdl = (struct audio_pcm_play_t *)zalloc(sizeof(struct audio_pcm_play_t));
    if (hdl == NULL) {
        return NULL;
    }

    hdl->cache_buf = malloc(cache_buf_size);
    if (hdl->cache_buf == NULL) {
        free(hdl);
        return NULL;
    }
    cbuf_init(&hdl->cbuf, hdl->cache_buf, cache_buf_size);

    os_sem_create(&hdl->w_sem, 0);
    os_sem_create(&hdl->r_sem, 0);

    hdl->block = block;
    hdl->run_flag = 1;

    hdl->dec_server = server_open("audio_server", "dec");
    if (hdl->dec_server == NULL) {
        goto __err;
    }

    /****************打开解码DAC器*******************/
    hdl->force_double_channel = 1;
    hdl->channel = channel;

    if (hdl->force_double_channel && channel == 1) {
        req.dec.channel     = 2;
        req.dec.output_buf_len = cache_buf_size;
    } else {
        req.dec.channel     = channel;
        req.dec.output_buf_len = cache_buf_size / 2;
    }
    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = volume;
    req.dec.sample_rate     = sample_rate;
    req.dec.vfs_ops         = &pcm_vfs_ops;
    req.dec.dec_type 		= "pcm";
#if defined CONFIG_AUDIO_DEC_PLAY_SOURCE
    req.dec.sample_source   = CONFIG_AUDIO_DEC_PLAY_SOURCE;
#else
    req.dec.sample_source   = "dac";
#endif
    req.dec.file            = (FILE *)hdl;
    /* req.dec.attr            = AUDIO_ATTR_LR_ADD; */          //左右声道数据合在一起
#if defined CONFIG_AUDIO_MIX_ENABLE
    req.dec.attr            = AUDIO_ATTR_REAL_TIME;
    req.dec.dec_sync        = pcm_sync;
    req.dec.sync_priv       = hdl;

    hdl->drop_points  = drop_points * channel;
    hdl->begin_size   = cache_buf_size * 6 / 10;
    hdl->top_size     = cache_buf_size / 2;
    hdl->bottom_size  = cache_buf_size * 3 / 10;
    hdl->max_step     = sample_rate / 5;
    if (sample_rate > 8000) {
        hdl->inc_step = 10 * sample_rate / 11025;
        hdl->dec_step = 10 * sample_rate / 11025;
    } else {
        hdl->inc_step = 10;
        hdl->dec_step = 10;
    }
#endif

    err = server_request(hdl->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        goto __err;
    }

    return hdl;

__err:
    if (hdl->dec_server) {
        server_close(hdl->dec_server);
        hdl->dec_server = NULL;
    }
    if (hdl->cache_buf) {
        free(hdl->cache_buf);
        hdl->cache_buf = NULL;
    }
    free(hdl);

    return NULL;
}

int audio_pcm_play_start(void *priv)
{
    struct audio_pcm_play_t *hdl = (struct audio_pcm_play_t *)priv;
    union audio_req req = {0};

    if (hdl == NULL || hdl->dec_server == NULL) {
        return -1;
    }

    req.dec.cmd = AUDIO_DEC_START;
    return server_request(hdl->dec_server, AUDIO_REQ_DEC, &req);
}

int audio_pcm_play_pause(void *priv, int clear_cache)
{
    struct audio_pcm_play_t *hdl = (struct audio_pcm_play_t *)priv;
    union audio_req req = {0};
    int err;

    if (hdl == NULL || hdl->dec_server == NULL) {
        return -1;
    }

    req.dec.cmd = AUDIO_DEC_PAUSE;
    err = server_request(hdl->dec_server, AUDIO_REQ_DEC, &req);

    if (hdl->cache_buf && clear_cache) {
        cbuf_clear(&hdl->cbuf);
    }

    return err;
}

int audio_pcm_play_set_volume(void *priv, u8 volume)
{
    struct audio_pcm_play_t *hdl = (struct audio_pcm_play_t *)priv;
    union audio_req req = {0};
    int err;

    if (hdl == NULL || hdl->dec_server == NULL) {
        return -1;
    }

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = volume;
    return server_request(hdl->dec_server, AUDIO_REQ_DEC, &req);
}

int audio_pcm_play_stop(void *priv)
{
    struct audio_pcm_play_t *hdl = (struct audio_pcm_play_t *)priv;
    union audio_req req = {0};

    if (hdl == NULL || hdl->dec_server == NULL) {
        return -1;
    }

    hdl->run_flag = 0;

    if (hdl->block) {
        os_sem_post(&hdl->w_sem);
        os_sem_post(&hdl->r_sem);
    }

    if (hdl->dec_server) {
        req.dec.cmd = AUDIO_DEC_STOP;
        server_request(hdl->dec_server, AUDIO_REQ_DEC, &req);
        server_close(hdl->dec_server);
        hdl->dec_server = NULL;
    }

    os_sem_del(&hdl->w_sem, OS_DEL_ALWAYS);
    os_sem_del(&hdl->r_sem, OS_DEL_ALWAYS);

    if (hdl->cache_buf) {
        free(hdl->cache_buf);
        hdl->cache_buf = NULL;
    }

    free(hdl);

    return 0;
}

int audio_pcm_play_data_write(void *priv, void *data, u32 size)
{
    struct audio_pcm_play_t *hdl = (struct audio_pcm_play_t *)priv;
    u32 wlen;

    if (hdl == NULL || hdl->cache_buf == NULL) {
        return -1;
    }

    do {
        wlen = cbuf_write(&hdl->cbuf, data, size);
        if (hdl->block) {
            os_sem_set(&hdl->r_sem, 0);
            os_sem_post(&hdl->r_sem);
        }
        if (wlen == size) {
            break;
        }
        if (!hdl->block) {
            //上层buf写不进去时清空一下，避免出现声音滞后的情况
            cbuf_clear(&hdl->cbuf);
            return 0;
        }
        os_sem_pend(&hdl->w_sem, 0);
    } while (hdl->run_flag);

    return size;
}

