#include "server/audio_server.h"
#include "server/server_core.h"
#include "generic/circular_buf.h"
#include "os/os_api.h"
#include "event.h"
#include "app_config.h"
#include "jlsp_far_keyword.h"
#include "event/key_event.h"
/* #include "jlsp_kws_aec.h" */

#if (defined CONFIG_ASR_ALGORITHM) && (CONFIG_ASR_ALGORITHM == JLKWS_ALGORITHM)
const int CONFIG_KWS_RAM_USE_ENABLE = 1;

extern void aisp_resume(void);

#define ONCE_SR_POINTS	160//256

#if (defined CONFIG_SXY_QYAI_ENABLE)
#define ONCE_SR_POINTS   160
#define AISP_BUF_SIZE	(ONCE_SR_POINTS * 2 * 2)	//跑不过来时适当加大倍数
#else
#define AISP_BUF_SIZE	(ONCE_SR_POINTS * 2 * 2)	//跑不过来时适当加大倍数
#endif
#define MIC_SR_LEN		(ONCE_SR_POINTS * 2)

#ifdef CONFIG_AEC_ENC_ENABLE

/*aec module enable bit define*/
#define AEC_EN              BIT(0)
#define NLP_EN              BIT(1)
#define ANS_EN              BIT(2)

#define AEC_MODE_ADVANCE	(AEC_EN | NLP_EN | ANS_EN)
#define AEC_MODE_REDUCE		(NLP_EN | ANS_EN)
#define AEC_MODE_SIMPLEX	(ANS_EN)

#endif

#define AEC_DATA_TO_SD 1 //将唤醒前的mic/dac/aec数据写卡进行查看,3channel(mic,dac,aec)

#if AEC_DATA_TO_SD
#define READSIZE 128
#define MIX_DATA_LEN (128*3*2)  //AEC READSIZE = 128, 3:channel, 2:s16
#define DAC_DATA_LEN (128*2*3)  //dac_sr/aec_sr*aec_points*channel  48000/16000 * 128 * 2
#endif


static struct {
    int pid;
    u16 sample_rate;
    u8 volatile exit_flag;
    int volatile run_flag;
    OS_SEM sem;
    s16 mic_buf[AISP_BUF_SIZE * 2];
    void *mic_enc;
    cbuffer_t mic_cbuf;
#if AEC_DATA_TO_SD
    FILE *fd;
    FILE *fd1;
    void *dac_orig_buf;
    void *cache_buf;
    OS_SEM w_sem;
    cbuffer_t save_cbuf;
    cbuffer_t dac_save_cbuf;
    s16 cache[READSIZE * 3];
    s16 dac_obuf[READSIZE * 3];
#endif
} aisp_server;

#define __this (&aisp_server)

static const float confidence[8] = {
    0.5, 0.5, 0.5, 0.5, //小杰小杰，小杰同学，播放音乐，暂停播放
    0.5, 0.5, 0.5, 0.5, //增大音量，减小音量，上一首, 下一首
};

enum {
    PLAY_MUSIC_EVENT = 4,
    STOP_MUSIC_EVENT,
    VOLUME_INC_EVENT = 7,
    VOLUME_DEC_EVENT,
    SONG_PREVIOUS_EVENT,
    SONG_NEXT_EVENT,
};

#if AEC_DATA_TO_SD
void aec_mix_data_to_sd()
{
    u8 data[MIX_DATA_LEN];
    u8 dac_data[DAC_DATA_LEN];
    while (1) {
        if (__this->fd && __this->fd1) {
            os_sem_pend(&__this->w_sem, 0);
            cbuf_read(&__this->save_cbuf, data, MIX_DATA_LEN);
            fwrite(data, MIX_DATA_LEN, 1, __this->fd);
            cbuf_read(&__this->dac_save_cbuf, dac_data, DAC_DATA_LEN);
            fwrite(dac_data, DAC_DATA_LEN, 1, __this->fd1);
        } else {
            break;
        }
    }
}

void aec_mix_data_set_cb(s16 *data, int step)
{
    if (__this->fd) {
        if (step == 1) { // !ch_data_exchange
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache[3 * i] = data[2 * i];
                __this->cache[3 * i + 1] = data[2 * i + 1];
            }
        } else if (step == 2) { //ch_data_exchange
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache[3 * i] = data[2 * i + 1];
                __this->cache[3 * i + 1] = data[2 * i];
            }
        }


        if (step == 3) { //aec_data
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache[3 * i + 2] = data[i];
            }

            if (0 == cbuf_write(&__this->save_cbuf, __this->cache, MIX_DATA_LEN)) {
                cbuf_clear(&__this->save_cbuf);
            }

            memset(__this->cache, 0, sizeof(__this->cache));
            os_sem_set(&__this->w_sem, 0);
            os_sem_post(&__this->w_sem);
        }
    }

}

void aec_soft_mix_data_set_cb(s16 *data, int step)
{
    if (__this->fd) {
        // mic data
        if (step == 1) {
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache[3 * i] = data[i];
            }
        }

        //dac data
        if (step == 2) {
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache[3 * i + 1] = data[i];
            }
        }

        //aec_data
        if (step == 3) {
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache[3 * i + 2] = data[i];
            }

            if (0 == cbuf_write(&__this->save_cbuf, __this->cache, MIX_DATA_LEN)) {
                printf("error jlkws aec_data cbuf write full!");
                cbuf_clear(&__this->save_cbuf);
            }

            memset(__this->cache, 0, sizeof(__this->cache));
            os_sem_set(&__this->w_sem, 0);
            os_sem_post(&__this->w_sem);

        }
    }

    if (__this->fd1) {
        if (step == 4) {
            for (u32 i = 0; i < DAC_DATA_LEN / 2; ++i) {
                __this->dac_obuf[i] = data[i];
            }

            if (0 == cbuf_write(&__this->dac_save_cbuf, __this->dac_obuf, DAC_DATA_LEN)) {
                printf("error jlkws dac_data cbuf write full!");
                cbuf_clear(&__this->dac_save_cbuf);
            }
        }
    }

}
#endif

static void aisp_task(void *priv)
{
    printf("--------------------------------------------------------------------1");
    u32 mic_len, linein_len;
    u32 time = 0, time_cnt = 0, cnt = 0;
    int ret;
    int model = 0;
    int model_size, private_heap_size, share_heap_size;
    void *kws = NULL;
    u8 *private_heap = NULL, *share_heap = NULL;
    int online = 0;

    jl_far_kws_model_get_heap_size(model, &model_size, &private_heap_size, &share_heap_size);
printf("--------------------------------------------------------------------2");
    private_heap = zalloc(private_heap_size);
    if (!private_heap) {
        goto __exit;
    }

    share_heap	 = zalloc(share_heap_size);
    if (!share_heap) {
        goto __exit;
    }
printf("--------------------------------------------------------------------3");
    kws = jl_far_kws_model_init(model, private_heap, private_heap_size, share_heap, share_heap_size, model_size, confidence, online);
    if (!kws) {
        goto __exit;
    }
printf("--------------------------------------------------------------------4");
#if AEC_DATA_TO_SD
    extern int storage_device_ready(void);
    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        os_time_dly(1);
    }
printf("--------------------------------------------------------------------5");
    __this->fd = fopen("storage/sd0/C/aec.pcm", "w+");
    __this->fd1 = fopen("storage/sd0/C/dac.pcm", "w+");
    os_sem_create(&__this->w_sem, 0);
    __this->cache_buf = malloc(1024 * 128);
    if (__this->cache_buf == NULL) {
        goto __exit;
    }
    __this->dac_orig_buf = malloc(1024 * 128);
    if (__this->dac_orig_buf == NULL) {
        goto __exit;
    }
    cbuf_init(&__this->save_cbuf, __this->cache_buf, 1024 * 128);
    cbuf_init(&__this->dac_save_cbuf, __this->dac_orig_buf, 1024 * 128);
    thread_fork("aec_mix_data_to_sd", 4, 256 * 1024, 0, 0, aec_mix_data_to_sd, NULL);
#endif
printf("--------------------------------------------------------------------5");
    aisp_resume();
printf("--------------------------------------------------------------------6");
    while (1) {
        if (__this->exit_flag) {
            break;
        }

        if (!__this->run_flag) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }

        if (__this->exit_flag) {
            break;
        }

        if ((cbuf_get_data_size(&__this->mic_cbuf) < ONCE_SR_POINTS * 2)) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }
        short near_data_buf[ONCE_SR_POINTS];
        mic_len = cbuf_read(&__this->mic_cbuf, near_data_buf, ONCE_SR_POINTS * 2);
        if (!mic_len) {
            continue;
        }

        time = timer_get_ms();
        ret = jl_far_kws_model_process(kws, model, (u8 *)near_data_buf, sizeof(near_data_buf));
        if (ret > 1) {
            printf("++++++++++++++++++ %d ++++++++++++++++++\n", ret);
#if AEC_DATA_TO_SD
            if (__this->mic_enc) {
                union audio_req req = {0};
                req.enc.cmd = AUDIO_ENC_STOP;
                server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);
                server_close(__this->mic_enc);
                __this->mic_enc = NULL;

                if (__this->fd) {
                    fclose(__this->fd);
                    __this->fd = NULL;
                }

                if (__this->fd1) {
                    fclose(__this->fd1);
                    __this->fd1 = NULL;
                }

                if (__this->cache_buf) {
                    free(__this->cache_buf);
                    __this->cache_buf = NULL;
                }

                if (__this->dac_orig_buf) {
                    free(__this->dac_orig_buf);
                    __this->dac_orig_buf = NULL;
                }

                os_sem_del(&__this->w_sem, OS_DEL_ALWAYS);
            }
#endif

            //add your button event according to ret
            struct key_event key = {0};

            if (ret == PLAY_MUSIC_EVENT) {
                key.action = KEY_EVENT_UP;
                key.value = KEY_LOCAL;
            } else if (ret == STOP_MUSIC_EVENT) {
                key.action = KEY_EVENT_HOLD;
                key.value = KEY_LOCAL;
            } else if (ret == VOLUME_INC_EVENT) {
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_DOWN;
            } else if (ret == VOLUME_DEC_EVENT) {
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_UP;
            } else if (ret == SONG_PREVIOUS_EVENT) {
                key.action = KEY_EVENT_LONG;
                key.value = KEY_UP;
            } else if (ret == SONG_NEXT_EVENT) {
                key.action = KEY_EVENT_LONG;
                key.value = KEY_DOWN;
            }

            key.type = KEY_EVENT_USER;
            /* key_event_notify(KEY_EVENT_FROM_USER, &key); */

#if (defined CONFIG_SXY_QYAI_ENABLE)
            if (ret == 2) {
                extern int qyai_pcm_strat(int sample_rate);
                qyai_pcm_strat(__this->sample_rate);
            }
#endif
            jl_far_kws_model_reset(kws);
        }

#if (defined CONFIG_SXY_QYAI_ENABLE)
        extern int qyai_pcm_write(char *buf, int len, int sample_rate);
        qyai_pcm_write(near_data_buf, sizeof(near_data_buf), __this->sample_rate);
#endif

        time_cnt += timer_get_ms() - time;
        if (++cnt == 100) {
            /* printf("aec time :%d \n", time_cnt); */
            time_cnt = cnt = 0;
        }
    }

__exit:

    if (kws) {
        jl_far_kws_model_free(kws);
    }
    if (private_heap) {
        free(private_heap);
    }
    if (share_heap) {
        free(share_heap);
    }

    __this->run_flag = 0;
#if (defined CONFIG_SXY_QYAI_ENABLE)
    extern void qyai_pcm_uninit(void);
    qyai_pcm_uninit();
#endif
}

static void enc_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        break;
#if (defined CONFIG_SXY_QYAI_ENABLE)
    case AUDIO_SERVER_EVENT_SPEAK_START:       /*!< VAD检测到开始说话 */
        qyai_speek_start();
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:        /*!< VAD检测到停止说话 */
        qyai_speek_stop();
        break;
#endif
    default:
        break;
    }
}

static int aisp_vfs_fwrite(void *file, void *data, u32 len)
{
    cbuffer_t *cbuf = (cbuffer_t *)file;

    u32 wlen = cbuf_write(cbuf, data, len);
    if (wlen != len) {
        cbuf_clear(&__this->mic_cbuf);
        puts("busy!\n");
    }
    os_sem_set(&__this->sem, 0);
    os_sem_post(&__this->sem);

    return len;
}

static int aisp_vfs_fclose(void *file)
{
    return 0;
}

static const struct audio_vfs_ops aisp_vfs_ops = {
    .fwrite = aisp_vfs_fwrite,
    .fclose = aisp_vfs_fclose,
};

int aisp_open(u16 sample_rate)
{
    printf("-------------------------------------------------->aisp_open.....");
    __this->exit_flag = 0;
    __this->mic_enc = server_open("audio_server", "enc");
    server_register_event_handler(__this->mic_enc, NULL, enc_server_event_handler);
    cbuf_init(&__this->mic_cbuf, __this->mic_buf, sizeof(__this->mic_buf));
    os_sem_create(&__this->sem, 0);
    __this->sample_rate =s ample_rate;

    return thread_fork("aisp", 3, 2048, 0, &__this->pid, aisp_task, __this);
}

void aisp_suspend(void)
{
    union audio_req req = {0};

    if (!__this->run_flag) {
        return;
    }

    __this->run_flag = 0;

    req.enc.cmd = AUDIO_ENC_STOP;
    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);
    cbuf_clear(&__this->mic_cbuf);
}

void aisp_resume(void)
{
    union audio_req req = {0};

    if (__this->run_flag) {
        return;
    }
    __this->run_flag = 1;
    os_sem_set(&__this->sem, 0);
    os_sem_post(&__this->sem);

    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = 1;
    req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
    req.enc.frame_size = ONCE_SR_POINTS * 2 * req.enc.channel;
    req.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    req.enc.sample_rate = __this->sample_rate;
    req.enc.format = "pcm";
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
    req.enc.sample_source = "plnk0";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK1
    req.enc.sample_source = "plnk1";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS0
    req.enc.sample_source = "iis0";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS1
    req.enc.sample_source = "iis1";
#else
    req.enc.sample_source = "mic";
#endif
    req.enc.vfs_ops = &aisp_vfs_ops;
    req.enc.output_buf_len = req.enc.frame_size * 5;
    req.enc.file = (FILE *)&__this->mic_cbuf;
#ifdef CONFIG_AEC_ENC_ENABLE
    struct aec_s_attr aec_param = {0};
    aec_param.EnableBit = AEC_MODE_ADVANCE;
    req.enc.aec_attr = &aec_param;
#ifdef CONFIG_SXY_QYAI_ENABLE
    req.enc.aec_enable = 0;
    aec_param.output_way = 0;	 //1:使用硬件回采 0:使用软件回采
#else
#ifndef CONFIG_AEC_USE_SMS_TDE
    req.enc.aec_enable = 1;
#else
    req.enc.aec_enable = 2;
#endif

    extern void get_cfg_file_aec_config(struct aec_s_attr * aec_param);
    get_cfg_file_aec_config(&aec_param);

    if (aec_param.EnableBit == 0) {
        req.enc.aec_enable = 0;
        req.enc.aec_attr = NULL;
    }
    if (aec_param.EnableBit != AEC_MODE_ADVANCE) {
        aec_param.output_way = 0;
    }

#if defined CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE && defined CONFIG_AISP_LINEIN_ADC_CHANNEL && defined CONFIG_AEC_LINEIN_CHANNEL_ENABLE
    if (req.enc.aec_enable) {
        aec_param.output_way = 1;	 //1:使用硬件回采 0:使用软件回采
        req.enc.channel_bit_map |= BIT(CONFIG_AISP_LINEIN_ADC_CHANNEL);	 //配置回采硬件通道
        if (CONFIG_AISP_LINEIN_ADC_CHANNEL < CONFIG_PHONE_CALL_ADC_CHANNEL) {
            req.enc.ch_data_exchange = 1;	 //如果回采通道使用的硬件channel比MIC通道使用的硬件channel靠前的话处理数据时需要交换一下顺序
        }
    }
#endif

    if (req.enc.sample_rate == 16000) {
        aec_param.wideband = 1;
        aec_param.hw_delay_offset = 30;
    } else {
        aec_param.wideband = 0;
        aec_param.hw_delay_offset = 75;
    }

#if CONFIG_AEC_USE_PLAY_MUSIC_ENABLE
    if (aec_param.output_way == 0) {
        aec_param.dac_ref_sr = 48000; //aec软件回采dac参考采样率
    }
#endif

#endif
#endif
#if (defined CONFIG_SXY_QYAI_ENABLE)
    if (req.enc.channel == 1 && !strcmp(req.enc.sample_source, "mic") && (req.enc.sample_rate == 8000 || req.enc.sample_rate == 16000)) {
        req.enc.use_vad = 1; //打开VAD断句功能
        req.enc.dns_enable = 1; //打开降噪功能
        req.enc.vad_auto_refresh = 1; //VAD自动刷新
        req.enc.vad_start_threshold = 300;
        req.enc.vad_stop_threshold = 0;
    }
#endif

    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);

#ifndef CONFIG_SXY_QYAI_ENABLE
#if defined CONFIG_AEC_ENC_ENABLE && !defined CONFIG_FPGA_ENABLE
    if (aec_param.output_way) {
#ifdef CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE
        extern void adc_multiplex_set_gain(const char *source, u8 channel_bit_map, u8 gain);
        adc_multiplex_set_gain("mic", BIT(CONFIG_AISP_LINEIN_ADC_CHANNEL), CONFIG_AISP_LINEIN_ADC_GAIN * 2);
#endif
    }
#endif
#endif

}

void aisp_close(void)
{
    if (__this->exit_flag) {
        return;
    }

    aisp_suspend();

    __this->exit_flag = 1;

    os_sem_post(&__this->sem);

    if (__this->mic_enc) {
        server_close(__this->mic_enc);
        __this->mic_enc = NULL;
    }
    thread_kill(&__this->pid, KILL_WAIT);
}

#endif

