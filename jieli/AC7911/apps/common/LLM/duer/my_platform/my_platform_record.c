#include "my_platform_record.h"
#include "system/includes.h"
#include "device/device.h"
#include "server/audio_server.h"
#include "app_config.h"


#define AUDIO_RECORD_VOICE_VOLUME 100
#if MY_AUDIO_SAVE_TEST
static FILE *rec_file = NULL;
#define __file rec_file
#else
#define __file NULL
#endif

static my_rec_t rec = {0};

static void my_rec_cbuf_init(void)
{
    if (!rec.buf) {
        rec.buf = malloc(MY_CBUF_SIZE);
        ASSERT(rec.buf);
    }
    cbuf_init(&rec.cbuf, rec.buf, MY_CBUF_SIZE);
}

static void my_rec_cbuf_exit(void)
{
    cbuf_clear(&(rec.cbuf));
    free(rec.buf);
    rec.buf = NULL;
}

static int my_fwrite(FILE *file, void *buf, u32 size)
{
    return fwrite(buf, size, 1, file);
}

// 音频数据写入回调
static u16 my_rec_write_data(u8 *voice_buf, u16 voice_len)
{
#if MY_AUDIO_SAVE_TEST
    if (__file) {
        size_t wlen = my_fwrite(__file, voice_buf, voice_len);
        if (wlen != voice_len) {
            printf("save file err: %d vs %d\n", (int)wlen, voice_len);
        }
    }
#endif
    int wlen = cbuf_write(&rec.cbuf, voice_buf, voice_len);
    if (wlen != voice_len) {
        MY_LOG_E("pcm out err: %d, %d\n", wlen, voice_len);
    }
    return 0;
}

static const struct audio_vfs_ops recorder_vfs_ops = {
    .fwrite = my_rec_write_data,
    .fopen = 0,
    .fread = 0,
    .fseek = 0,
    .ftell = 0,
    .flen = 0,
    .fclose = 0,
};

static void enc_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        break;
    case AUDIO_SERVER_EVENT_SPEAK_START:
        /* printf("speak start\n"); */
        /* recoder_state = 1; */
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:
        /* printf("speak stop\n"); */
        /* recoder_state = 0; */
        break;
    default:
        break;
    }
}

static int mic_rec_pram_init_(char *format, int CHANNEL, int SAMPLE_RATE)
{
    int op_ret;
    MY_LOG_D("into volc audio soft init");
    if (!rec.enc_server) {
        rec.enc_server = server_open("audio_server", "enc");
        if (!rec.enc_server) {
            op_ret = -1;
            audio_debug("server_open err:%d", op_ret);
            return op_ret;
        }
        MY_LOG_D("enc server_open succ!");
        server_register_event_handler_to_task(rec.enc_server, NULL, enc_server_event_handler, "app_core");
    }
    int err;
    union audio_req req = {0};
    req.enc.frame_size = SAMPLE_RATE / 100 * 4 * CHANNEL;        //收集够多少字节PCM数据就回调一次fwrite
    req.enc.output_buf_len = req.enc.frame_size * 3; //底层缓冲buf至少设成3倍frame_size
    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = CHANNEL;
    req.enc.volume = AUDIO_RECORD_VOICE_VOLUME;
    req.enc.sample_rate = SAMPLE_RATE;

    req.enc.format = format;

    req.enc.sample_source = "mic";
    req.enc.vfs_ops = &recorder_vfs_ops;
    req.enc.file = (FILE *)&rec.cbuf;
    req.enc.channel_bit_map = BIT(1);

    if (CHANNEL == 1 && !strcmp(req.enc.sample_source, "mic") && (SAMPLE_RATE == 8000 || SAMPLE_RATE == 16000)) {
        req.enc.use_vad = 1;                    //打开VAD断句功能
        req.enc.vad_auto_refresh = 1;   //VAD自动刷新
    }
    if (req.enc.use_vad == 1) {
        req.enc.vad_start_threshold = 300;    //ms
        req.enc.vad_stop_threshold  = 0;    //ms
    }

#ifdef CONFIG_AEC_ENC_ENABLE
    struct aec_s_attr aec_param = {0};
    req.enc.aec_attr = &aec_param;
    req.enc.aec_enable = 1;

    extern void get_cfg_file_aec_config(struct aec_s_attr * aec_param);
    get_cfg_file_aec_config(&aec_param);

    if (aec_param.EnableBit == 0) {
        req.enc.aec_enable = 0;
        req.enc.aec_attr = NULL;
    }

#if defined CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE && defined CONFIG_AISP_LINEIN_ADC_CHANNEL && defined CONFIG_AEC_LINEIN_CHANNEL_ENABLE
    if (req.enc.aec_enable) {
        aec_param.output_way = 1;               //1:使用硬件回采 0:使用软件回采
        if (aec_param.output_way) {
            req.enc.channel_bit_map |= BIT(CONFIG_AISP_LINEIN_ADC_CHANNEL);             //配置回采硬件通道
            if (CONFIG_AISP_LINEIN_ADC_CHANNEL < CONFIG_PHONE_CALL_ADC_CHANNEL) {
                req.enc.ch_data_exchange = 1;     //如果回采通道使用的硬件channel比MIC通道使用的硬件channel靠前的话处理数据时需要交换一下顺序
            }
        }
    }
#endif

    if (req.enc.sample_rate == 16000) {
        aec_param.wideband = 1;
        aec_param.hw_delay_offset = 50;
    } else {
        aec_param.wideband = 0;
        aec_param.hw_delay_offset = 75;
    }
#endif

    err = server_request(rec.enc_server, AUDIO_REQ_ENC, &req);
    printf("err:%d", err);

    if (err == 0) {
        rec.is_audio_record_open = 1;
    }
    return err;
}

static void mic_rec_close()
{
    int err;
    union audio_req req = {0};
    req.enc.cmd = AUDIO_ENC_STOP;
    err = server_request(rec.enc_server, AUDIO_REQ_ENC, &req);
    rec.is_audio_record_open = 0;

    server_close(rec.enc_server);
    rec.enc_server = NULL;
    MY_LOG_D("_audio_recoder_stop err=%d", err);
}

static int mic_is_busy()
{
    return rec.is_audio_record_open;
}

// 停止录音
int my_rec_stop(void)
{
    if (!mic_is_busy()) {
        printf("mic not recording\n");
        return false;
    }
    mic_rec_close();

#if MY_AUDIO_SAVE_TEST
    if (__file) {
        fclose(__file);
        __file = NULL;
    }
#endif
    my_rec_cbuf_exit();
    return true;
}

// 开始录音
int my_rec_start(void)
{
    my_rec_cbuf_init();
    if (mic_is_busy()) {
        printf("my_mic_is_busy \n\n");
        return false;
    }
#if MY_AUDIO_SAVE_TEST
    if (__file) {
        fclose(__file);
        __file = NULL;
    }
    __file = fopen("storage/sd0/C/ze.bin", "w+");
    if (!__file) {
        MY_LOG_E("fopen err \n\n");
    }
#endif
    int ret = mic_rec_pram_init_(MY_REC_TYPE, 1, 16000);
    if (ret) {
        goto err_exit;
    }
    /* mic_rec_start(); */
    return true;
err_exit:
    my_rec_stop();
    return false;
}
