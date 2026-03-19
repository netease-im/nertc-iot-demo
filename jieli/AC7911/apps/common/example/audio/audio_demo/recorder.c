#include "server/audio_server.h"
#include "server/server_core.h"
#include "system/app_core.h"
#include "generic/circular_buf.h"
#include "os/os_api.h"
#include "app_config.h"
#include "syscfg/syscfg_id.h"
#include "event/key_event.h"
#include "storage_device.h"
#include "fs/fs.h"
#include <time.h>

#ifdef USE_AUDIO_DEMO

#ifdef CONFIG_RECORDER_MODE_ENABLE
#define CONFIG_STORE_VOLUME
#define VOLUME_STEP         5
#define GAIN_STEP           5
#define MIN_VOLUME_VALUE	5
#define MAX_VOLUME_VALUE	100
#define INIT_VOLUME_VALUE   50

/////////////////////////////---AEC_DATA_TO_SD---///////////////////////////////
#define AEC_DATA_TO_SD 1
#define READSIZE 128
#define MIX_DATA_LEN (128*3*2)  //AEC READSIZE = 128, 3:channel, 2:s16
#define DAC_DATA_LEN (128*2*3)  //dac_sr/aec_sr*aec_points*channel  48000/16000 * 128 * 2

struct recorder_hdl {
    FILE *aec_fd;
    FILE *dac_fd;
    FILE *play_fd;
    FILE *mic_fp;

    struct server *enc_server;
    struct server *dec_server;
    void *cache_buf;
    cbuffer_t save_cbuf;

    void *dac_orig_buf;
    cbuffer_t dac_save_cbuf;
    s16 cache[READSIZE * 3];
    s16 dac_obuf[READSIZE * 3];

    OS_SEM w_sem;
    OS_SEM r_sem;
    volatile u8 run_flag;
    u8 volume;
    u8 gain;
    u8 channel;
    u8 direct;
    const char *sample_source;
    int sample_rate;
};

static struct recorder_hdl recorder_handler;
#define __this (&recorder_handler)

//AUDIO ADC支持的采样率
static const u16 sample_rate_table[] = {
    8000,
    11025,
    12000,
    16000,
    22050,
    24000,
    32000,
    44100,
    48000,
};
////////////////////////////////////////////////

void aec_mix_data_to_sd(){
    u8 data[MIX_DATA_LEN];
    u8 dac_data[DAC_DATA_LEN];
    while (1) {
        if (__this->aec_fd && __this->dac_fd) {
            os_sem_pend(&__this->w_sem, 0);
            cbuf_read(&__this->save_cbuf, data, MIX_DATA_LEN);
            fwrite(data, MIX_DATA_LEN, 1, __this->aec_fd);
            cbuf_read(&__this->dac_save_cbuf, dac_data, DAC_DATA_LEN);
            fwrite(dac_data, DAC_DATA_LEN, 1, __this->dac_fd);
        } else {
            break;
        }
    }
}

void aec_mix_data_set_cb(s16 *data, int step)
{
    if (__this->aec_fd) {
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
    if (__this->aec_fd) {
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

    if (__this->dac_fd) {
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



static int recorder_close(void){
    union audio_req req = {0};
    if (!__this->run_flag)return 0;
    __this->run_flag = 0;

    log_d("----------recorder close----------\n");

    
    os_sem_post(&__this->w_sem);
    //os_sem_post(&__this->r_sem);

    if (__this->enc_server) {
        req.enc.cmd = AUDIO_ENC_CLOSE;
        server_request(__this->enc_server, AUDIO_REQ_ENC, &req);
    }

    if (__this->dec_server) {
        req.dec.cmd = AUDIO_DEC_STOP;
        server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    }

    if (__this->cache_buf) {
        free(__this->cache_buf);
        __this->cache_buf = NULL;
    }

    if (__this->dac_orig_buf) {
        free(__this->dac_orig_buf);
        __this->dac_orig_buf = NULL;
    }

    if (__this->aec_fd) {
        fclose(__this->aec_fd);
        __this->aec_fd = NULL;
    }

    if (__this->dac_fd) {
        fclose(__this->dac_fd);
        __this->dac_fd = NULL;
    }

    if (__this->play_fd) {
        fclose(__this->play_fd);
        __this->play_fd = NULL;
    }

    if (__this->mic_fp) {
        fclose(__this->mic_fp);
        __this->mic_fp = NULL;
    }

    return 0;
}

static void enc_server_event_handler(void *priv, int argc, int *argv){
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        recorder_close();
        break;
    case AUDIO_SERVER_EVENT_SPEAK_START:
        log_i("speak start ! \n");
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:
        log_i("speak stop ! \n");
        break;
    default:
        break;
    }
}


static int recorder_play_to_dac(int sample_rate, u8 channel){
    int err;
    union audio_req req = {0};

    log_d("----------recorder_play_to_dac----------\n");

    extern int storage_device_ready(void);
    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        os_time_dly(1);
    }

    __this->run_flag = 1;
    sample_rate=16000;
    channel=1;

    os_sem_create(&__this->w_sem, 0);
    //os_sem_create(&__this->r_sem, 0);

printf("--------------------------------------------------------------------5");
    __this->aec_fd = fopen("storage/sd0/C/aec.pcm", "w+");if(!__this->aec_fd) goto __err;
    __this->dac_fd = fopen("storage/sd0/C/dac.pcm", "w+");if(!__this->dac_fd) goto __err;
    __this->play_fd=fopen(CONFIG_ROOT_PATH"player.mp3","r");if(!__this->play_fd) goto __err;
    __this->mic_fp = fopen(CONFIG_ROOT_PATH"TEST.wav", "w+");if(!__this->mic_fp) goto __err;

    __this->cache_buf = malloc(1024 * 128);if (__this->cache_buf == NULL) goto __err;
    __this->dac_orig_buf = malloc(1024 * 128);if (__this->dac_orig_buf == NULL)goto __err;
    cbuf_init(&__this->save_cbuf, __this->cache_buf, 1024 * 128);
    cbuf_init(&__this->dac_save_cbuf, __this->dac_orig_buf, 1024 * 128);
    thread_fork("aec_mix_data_to_sd", 4, 256 * 1024, 0, 0, aec_mix_data_to_sd, NULL);

    
    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = __this->volume;
    req.dec.output_buf_len  = 6 * 1024;
    req.dec.file            = __this->play_fd;
    req.dec.channel         = 0;
    req.dec.sample_rate     = 0;
    req.dec.priority        = 1;
    req.dec.sample_source   = CONFIG_AUDIO_DEC_PLAY_SOURCE;
    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err)goto __err;
    
    req.dec.cmd = AUDIO_DEC_START;
    req.dec.attr = AUDIO_ATTR_NO_WAIT_READY;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    /****************打开编码器*******************/
    memset(&req, 0, sizeof(union audio_req));
    req.enc.channel_bit_map = BIT(CONFIG_AUDIO_ADC_CHANNEL_L);
    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = channel;
    req.enc.volume = __this->gain;
    req.enc.frame_size = 8192;
    req.enc.output_buf_len = req.enc.frame_size * 10;
    req.enc.sample_rate = sample_rate;
    req.enc.format = "wav";
    req.enc.sample_source = __this->sample_source;
    req.enc.msec = 15*1000;//CONFIG_AUDIO_RECORDER_DURATION;
    req.enc.file = __this->mic_fp;

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

printf("-------------------------------------------------------aec_param.output_way:%d",aec_param.output_way);
#if defined CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE && defined CONFIG_AISP_LINEIN_ADC_CHANNEL && defined CONFIG_AEC_LINEIN_CHANNEL_ENABLE
    if (req.enc.aec_enable) {
        aec_param.output_way = 1;               //1:使用硬件回采 0:使用软件回采

        if (aec_param.output_way) {
            req.enc.channel_bit_map |= BIT(CONFIG_AISP_LINEIN_ADC_CHANNEL);             //配置回采硬件通道

            if (CONFIG_AISP_LINEIN_ADC_CHANNEL < CONFIG_AUDIO_ADC_CHANNEL_L) {
                printf("--------------------------55555555555555555555555555-----------------------------------");
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

    err = server_request(__this->enc_server, AUDIO_REQ_ENC, &req);
    if (err)goto __err;
    return 0;


__err:
    recorder_close();
    return -1;
}

//MIC或者LINEIN模拟直通到DAC，不需要软件参与
static int audio_adc_analog_direct_to_dac(int sample_rate, u8 channel){
    union audio_req req = {0};

    log_d("----------audio_adc_analog_direct_to_dac----------\n");

    __this->run_flag = 1;

    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = channel;
    req.enc.volume = __this->gain;
    req.enc.format = "pcm";
    req.enc.sample_source = __this->sample_source;
    req.enc.sample_rate = sample_rate;
    req.enc.direct2dac = 1;
    req.enc.high_gain = 1;
    if (channel == 4) {
        req.enc.channel_bit_map = 0x0f;
    } else if (channel == 2) {
        req.enc.channel_bit_map = BIT(CONFIG_AUDIO_ADC_CHANNEL_L) | BIT(CONFIG_AUDIO_ADC_CHANNEL_R);
    } else {
        req.enc.channel_bit_map = BIT(CONFIG_AUDIO_ADC_CHANNEL_L);
    }

    return server_request(__this->enc_server, AUDIO_REQ_ENC, &req);
}

//录音文件到SD卡
static int recorder_to_file(int sample_rate, u8 channel){
    union audio_req req = {0};

    __this->run_flag = 1;
    __this->direct = 0;

    char time_str[64] = {0};
    char file_name[100] = {0};
    u8 dir_len = 0;
    struct tm timeinfo = {0};
    time_t timestamp = time(NULL) + 28800;
    localtime_r(&timestamp, &timeinfo);
    strcpy(time_str, CONFIG_ROOT_PATH"RECORDER/\\U");
    dir_len = strlen(time_str);
    strftime(time_str + dir_len, sizeof(time_str) - dir_len, "%Y-%m-%dT%H-%M-%S.wav", &timeinfo);
    log_i("recorder file name : %s\n", time_str);

    memcpy(file_name, time_str, dir_len);

    for (u8 i = 0; i < strlen(time_str) - dir_len; ++i) {
        file_name[dir_len + i * 2] = time_str[dir_len + i];
    }

    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = channel;
    req.enc.volume = __this->gain;
    req.enc.frame_size = 8192;
    req.enc.output_buf_len = req.enc.frame_size * 10;
    req.enc.sample_rate = sample_rate;
    req.enc.format = "wav";
    req.enc.sample_source = __this->sample_source;
    req.enc.msec = CONFIG_AUDIO_RECORDER_DURATION;
    req.enc.file = __this->mic_fp = fopen(file_name, "w+");
    /* req.enc.sample_depth = 24; //IIS支持采集24bit深度 */
    if (channel == 4) {
        req.enc.channel_bit_map = 0x0f;
    } else if (channel == 2) {
        req.enc.channel_bit_map = BIT(CONFIG_AUDIO_ADC_CHANNEL_L) | BIT(CONFIG_AUDIO_ADC_CHANNEL_R);
    } else {
        req.enc.channel_bit_map = BIT(CONFIG_AUDIO_ADC_CHANNEL_L);
    }

    return server_request(__this->enc_server, AUDIO_REQ_ENC, &req);
}

static void recorder_play_pause(void){
    union audio_req req = {0};

    req.dec.cmd = AUDIO_DEC_PP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    req.enc.cmd = AUDIO_ENC_PP;
    server_request(__this->enc_server, AUDIO_REQ_ENC, &req);

    if (__this->cache_buf) {
        cbuf_clear(&__this->save_cbuf);
    }
}

//调整ADC的模拟增益
static int recorder_enc_gain_change(int step){
    union audio_req req = {0};

    int gain = __this->gain + step;
    if (gain < 0) {
        gain = 0;
    } else if (gain > 100) {
        gain = 100;
    }
    if (gain == __this->gain) {
        return -1;
    }
    __this->gain = gain;

    if (!__this->enc_server) {
        return -1;
    }

    log_d("set_enc_gain: %d\n", gain);

    req.enc.cmd     = AUDIO_ENC_SET_VOLUME;
    req.enc.volume  = gain;
    return server_request(__this->enc_server, AUDIO_REQ_ENC, &req);
}

//调整DAC的数字音量和模拟音量
static int recorder_dec_volume_change(int step){
    union audio_req req = {0};

    int volume = __this->volume + step;
    if (volume < MIN_VOLUME_VALUE) {
        volume = MIN_VOLUME_VALUE;
    } else if (volume > MAX_VOLUME_VALUE) {
        volume = MAX_VOLUME_VALUE;
    }
    if (volume == __this->volume) {
        return -1;
    }
    __this->volume = volume;

    if (!__this->dec_server) {
        return -1;
    }

    log_d("set_dec_volume: %d\n", volume);

#ifdef CONFIG_STORE_VOLUME
    syscfg_write(CFG_MUSIC_VOL, &__this->volume, sizeof(__this->volume));
#endif

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = volume;
    return server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
}

static int recorder_mode_init(void){
    log_i("recorder_play_main\n");

    memset(__this, 0, sizeof(struct recorder_hdl));

#ifdef CONFIG_STORE_VOLUME
    if (syscfg_read(CFG_MUSIC_VOL, &__this->volume, sizeof(__this->volume)) < 0 ||
        __this->volume < MIN_VOLUME_VALUE || __this->volume > MAX_VOLUME_VALUE) {
        __this->volume = INIT_VOLUME_VALUE;
    }
#else
    __this->volume = INIT_VOLUME_VALUE;
#endif
    if (__this->volume < 0 || __this->volume > MAX_VOLUME_VALUE) {
        __this->volume = INIT_VOLUME_VALUE;
    }

#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
    __this->sample_source = "plnk0";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK1
    __this->sample_source = "plnk1";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS0
    __this->sample_source = "iis0";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS1
    __this->sample_source = "iis1";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_LINEIN
    __this->sample_source = "linein";
#else
    __this->sample_source = "mic";
#endif

    __this->channel = CONFIG_AUDIO_RECORDER_CHANNEL;
    __this->gain = CONFIG_AUDIO_ADC_GAIN;
    __this->sample_rate = CONFIG_AUDIO_RECORDER_SAMPLERATE;

    __this->enc_server = server_open("audio_server", "enc");
    server_register_event_handler_to_task(__this->enc_server, NULL, enc_server_event_handler, "app_core");

    __this->dec_server = server_open("audio_server", "dec");
    return recorder_play_to_dac(__this->sample_rate, __this->channel);
}

static void recorder_mode_exit(void){
    recorder_close();
    server_close(__this->dec_server);
    __this->dec_server = NULL;
    server_close(__this->enc_server);
    __this->enc_server = NULL;
}

static int recorder_key_click(struct key_event *key){
    int ret = true;

    switch (key->value) {
    case KEY_OK:
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_LINEIN || CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_MIC
        if (__this->direct) {
            if (__this->run_flag) {
                recorder_close();
            } else {
                audio_adc_analog_direct_to_dac(__this->sample_rate, __this->channel);
            }
        } else {
            recorder_play_pause();
        }
#else
        recorder_play_pause();
#endif
        break;
    case KEY_VOLUME_DEC:
        recorder_dec_volume_change(-VOLUME_STEP);
        break;
    case KEY_VOLUME_INC:
        recorder_dec_volume_change(VOLUME_STEP);
        break;
    default:
        ret = false;
        break;
    }

    return ret;
}

static int recorder_key_long(struct key_event *key)
{
    switch (key->value) {
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_LINEIN || CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_MIC
    case KEY_OK:
        recorder_close();
        if (__this->direct) {
            recorder_play_to_dac(__this->sample_rate, __this->channel);
        } else {
            audio_adc_analog_direct_to_dac(__this->sample_rate, __this->channel);
        }
        __this->direct = !__this->direct;
        break;
    case KEY_VOLUME_DEC:
        recorder_enc_gain_change(-GAIN_STEP);
        break;
    case KEY_VOLUME_INC:
        recorder_enc_gain_change(GAIN_STEP);
        break;
#endif
    case KEY_MODE:
        if (storage_device_ready()) {
            recorder_close();
            recorder_to_file(__this->sample_rate, __this->channel);
        }
        break;
    default:
        break;
    }

    return true;
}

static int recorder_key_event_handler(struct key_event *key)
{
    switch (key->action) {
    case KEY_EVENT_CLICK:
        return recorder_key_click(key);
    case KEY_EVENT_LONG:
        return recorder_key_long(key);
    default:
        break;
    }

    return true;
}

static int recorder_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return recorder_key_event_handler((struct key_event *)event->payload);
    default:
        return false;
    }
}

static int recorder_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        recorder_mode_init();
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        recorder_mode_exit();
        break;
    case APP_STA_DESTROY:
        break;
    }

    return 0;
}

static const struct application_operation recorder_ops = {
    .state_machine  = recorder_state_machine,
    .event_handler 	= recorder_event_handler,
};

REGISTER_APPLICATION(recorder) = {
    .name 	= "recorder",
    .ops 	= &recorder_ops,
    .state  = APP_STA_DESTROY,
};

#endif

#endif//USE_AUDIO_DEMO
