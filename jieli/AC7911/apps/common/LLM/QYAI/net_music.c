#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "storage_device.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "event/net_event.h"
#include "syscfg/syscfg_id.h"
#include "system/wait.h"
#include "system/app_core.h"
#include "lwip.h"


#define MUSIC_PLAY_NUM  50  //没有打断情况下：自动连续播放音乐数量

#include "server/ai_server.h"
#include "network_download/net_download.h"

struct net_music_hdl {
    int volume;
    char play_status;
    char play_loop;
    u16 wait_download;
    int download_ready;
    int play_time;
    int total_time;
    int total_num;
    int suspend_time;
    void *net_file;
    const char *ai_name;
    struct server *dec_server;
    struct server *ai_server;
    void *spectrum_fft_hdl;
    char *url; //保存断点歌曲的链接
    struct audio_dec_breakpoint dec_bp;
    OS_MUTEX mutex;
};
static char music_url[2][1024];
static struct net_music_hdl net_music_handler ALIGNED(4);

#define __this 	(&net_music_handler)
int net_music_dec_file(const char *url);
int websocket_client_thread_create(void *priv);

void net_music_play_set_stop(void);
//网络解码需要的数据访问句柄
static const struct audio_vfs_ops net_audio_dec_vfs_ops = {
    .fread = net_download_read,
    .fseek = net_download_seek,
    .flen  = net_download_get_file_len,
};

//检查音频需要的格式检查数据头部是否下载缓冲完成
static int __net_download_ready(void *p)
{
    __this->download_ready = net_download_check_ready(__this->net_file);
    if (__this->download_ready) {
        printf("---> __net_download_ready \n");
        return 1;
    }
    return 0;
}
static void net_music_dec_play_pause_tolong(void *p)
{
    if (net_music_play_pause_status()) {
        net_music_dec_stop();
    }
    __this->suspend_time = 0;
}
static int net_music_dec_play_pause(u8 notify)
{
    union audio_req r = {0};
    union ai_req req  = {0};
    int err = 0;

    if (!__this->net_file || !__this->dec_server) {
        __this->play_status = 0;
        return -1;
    }
#ifdef CONFIG_DEC_ANALOG_VOLUME_ENABLE
    r.dec.attr = AUDIO_ATTR_FADE_INOUT;
#endif
    r.dec.cmd = AUDIO_DEC_PP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    __this->play_status = r.dec.status;

    if (r.dec.status == AUDIO_DEC_START) {//播放状态
        net_download_set_pp(__this->net_file, 0);//播放状态
        if (__this->suspend_time) {
            sys_timer_del(__this->suspend_time);
            __this->suspend_time = 0;
        }
        printf("======== AUDIO_DEC_STAR\n");
    } else if (r.dec.status == AUDIO_DEC_PAUSE) {//暂停状态
        net_download_set_pp(__this->net_file, 1);//暂停播放
        if (!__this->suspend_time) {
            __this->suspend_time = sys_timeout_add(NULL, net_music_dec_play_pause_tolong, 20 * 60 * 1000); //20分钟暂停则关闭
        } else {
            sys_timer_modify(__this->suspend_time, 20 * 60 * 1000); //20分钟暂停则关闭
        }
        err = 1;
        printf("======== AUDIO_DEC_PAUSE\n");
    } else {
        if (__this->suspend_time) {
            sys_timer_del(__this->suspend_time);
            __this->suspend_time = 0;
        }
        printf("======== AUDIO_DEC err = %d\n", r.dec.status);
    }
    if (notify && __this->ai_server && r.dec.status != AUDIO_DEC_STOP) {
        //notify : 是否需要通知云端暂停
        req.evt.event   = AI_EVENT_PLAY_PAUSE;
        req.evt.ai_name = __this->ai_name;
        if (r.dec.status == AUDIO_DEC_PAUSE) {
            req.evt.arg = 1;
        }
        ai_server_request(__this->ai_server, AI_REQ_EVENT, &req);
    }
    return err;
}

int net_music_play_stop_status(void)
{
    if (!__this->net_file || !__this->dec_server || __this->play_status == AUDIO_DEC_STOP || __this->play_status == AUDIO_DEC_PAUSE || __this->play_status == 0) {
        return true;
    }
    return false;
}

int net_music_play_pause_status(void)
{
    union audio_req req = {0};
    if (!__this->net_file || !__this->dec_server) {
        return 0;
    }
    req.dec.cmd = AUDIO_DEC_GET_STATUS;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    return req.dec.status == AUDIO_DEC_PAUSE;//AUDIO_DEC_OPEN  AUDIO_DEC_START  AUDIO_DEC_PAUSE
}
int net_music_play_start_status(void)
{
    union audio_req req = {0};
    if (!__this->net_file || !__this->dec_server) {
        return 0;
    }
    req.dec.cmd = AUDIO_DEC_GET_STATUS;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    return req.dec.status == AUDIO_DEC_START;//AUDIO_DEC_OPEN  AUDIO_DEC_START  AUDIO_DEC_PAUSE
}
//暂停/继续播放
int net_music_play_pause(u8 stop)
{
    union audio_req req = {0};
    int err = 0;

    if (!__this->net_file || !__this->dec_server || __this->play_status == AUDIO_DEC_STOP  || __this->play_status == 0) {
        __this->play_status = 0;
        return -1;
    }
    req.dec.cmd = AUDIO_DEC_GET_STATUS;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (req.dec.status == AUDIO_DEC_STOP && __this->wait_download) {
        printf("__this->play_status = AUDIO_DEC_PP\n");
        __this->play_status = AUDIO_DEC_PP;
        return stop;
    }
    if (__this->play_status == AUDIO_DEC_START && stop == 0) {
        return 0;
    }
    if (__this->play_status == AUDIO_DEC_PAUSE && stop == 1) {
        return 1;
    }
    if (!stop) {
        __this->play_status = AUDIO_DEC_START;
        sys_timeout_add_to_task("sys_timer", NULL, net_music_dec_play_pause, 1000);
    } else {
        __this->play_status = AUDIO_DEC_PAUSE;
        net_music_dec_play_pause(0);
        err = 1;
    }
    return err;
}

//停止播放
int net_music_dec_stop(void)
{
    union audio_req r = {0};
    union ai_req req = {0};

    if (!__this->net_file) {
        return 0;
    }
    os_mutex_pend(&__this->mutex, 6000);
    __this->spectrum_fft_hdl = NULL;
    __this->play_status = AUDIO_DEC_STOP;

    log_i("net_music_dec_stop\n");

    net_download_buf_inactive(__this->net_file);

    if (__this->wait_download) {
        /*
         * 歌曲还未开始播放，删除wait
         */
        /* sys_timer_del(__this->wait_download); */
        wait_completion_del(__this->wait_download);
        __this->wait_download = 0;
    } else {
        r.dec.cmd = AUDIO_DEC_STOP;
        server_request(__this->dec_server, AUDIO_REQ_DEC, &r);

        int argv[2];
        argv[0] = AUDIO_SERVER_EVENT_END;
        argv[1] = (int)__this->net_file;
        server_event_handler_del(__this->dec_server, 2, argv);
    }

    extern int lwip_canceladdrinfo(void);
    lwip_canceladdrinfo();
    //释放网络下载资源
    net_download_close(__this->net_file);
    if (__this->suspend_time) {
        sys_timer_del(__this->suspend_time);
        __this->suspend_time = 0;
    }
    __this->net_file = NULL;
    __this->play_status = 0;
    os_mutex_post(&__this->mutex);
    return 0;
}

int net_music_play_next(void)
{
    __this->play_loop = 0;
    net_music_dec_stop();
    net_music_play_set_stop();
    extern int qyai_net_client_thread_create(void *priv);
    sys_timeout_add_to_task("sys_timer", 2, qyai_net_client_thread_create, 1000);//1秒后再创建，否则会在发断开消息到下次音乐请求
    return 0;//websocket_client_thread_create(2);//继续播放下一首
}

int net_music_num_clear(void)
{
    int ret = __this->total_num;
    __this->total_num = 0;
    return ret;
}

//播放结束
static int net_music_dec_end(void)
{
    union ai_req req = {0};

    printf("->net_music play num = %d \n", __this->total_num);
    os_mutex_pend(&__this->mutex, 6000);
    __this->spectrum_fft_hdl = NULL;

    if (__this->total_time - __this->play_time <= 1 && __this->total_num <= MUSIC_PLAY_NUM) { //完整播放完成，自动播放30首关闭
        if (__this->play_loop && music_url[1][0] != 0) {
            //net_music_dec_file(&music_url[1][0]);
            sys_timeout_add_to_task("sys_timer", &music_url[1][0], net_music_dec_file, 100);
        } else {
            printf("->play next music: %d, %d \n", __this->play_loop, music_url[1][0]);
            net_music_play_next();
        }
        __this->total_num++;
    } else if (__this->total_time - __this->play_time >= 2 || __this->total_num > MUSIC_PLAY_NUM) {
        __this->total_num = 0;
        void qyai_music_buf_play_set_stop(void);
        qyai_music_buf_play_set_stop();
        net_music_dec_stop();
        net_music_play_set_stop();
    }
    os_mutex_post(&__this->mutex);
    if (!__this->ai_server) {
        return 0;
    }

    /* 歌曲播放完成，发送此命令后ai平台会发送新的URL */
    req.evt.event   = AI_EVENT_MEDIA_END;
    req.evt.ai_name     = __this->ai_name;
    return ai_server_request(__this->ai_server, AI_REQ_EVENT, &req);
}

//数据缓冲已完成，开始解码
static int __net_music_dec_file(void *file)
{
    int err;
    union ai_req r = {0};
    union audio_req req = {0};

    extern int qyai_sys_connect_net_success(void);
    if (!qyai_sys_connect_net_success()) {
        return -1;
    }
    __this->wait_download = 0;
    if (__this->download_ready < 0) {
        /* 网络下载失败 */
        goto __err;
    }

    //获取网络资源的格式
    req.dec.dec_type = net_download_get_media_type(file);
    if (req.dec.dec_type == NULL) {
        goto __err;
    }
    printf("url_file_type: %s\n", req.dec.dec_type);

    net_download_set_read_timeout(file, 5000);

    qyai_sys_volume_read(&__this->volume);

    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = __this->volume;
    req.dec.output_buf_len  = 6 * 1024;
    req.dec.file            = (FILE *)file;
    req.dec.channel         = 0;
    req.dec.sample_rate     = 0;
    req.dec.priority        = 1;
    req.dec.vfs_ops         = &net_audio_dec_vfs_ops;
    req.dec.sample_source   = "dac";
    req.dec.force_sr        = 0;//强制使用采样率
    /* req.dec.bp              = &__this->dec_bp; //恢复断点 */
#if 0	//变声变调功能
    req.dec.speedV = 80; // >80是变快，<80是变慢，建议范围：30到130
    req.dec.pitchV = 32768; // >32768是音调变高，<32768音调变低，建议范围20000到50000
    req.dec.attr = AUDIO_ATTR_PS_EN;
#endif

#if TCFG_EQ_ENABLE
#if defined EQ_CORE_V1
    req.dec.attr |= AUDIO_ATTR_EQ_EN;
#if TCFG_LIMITER_ENABLE
    req.dec.attr |= AUDIO_ATTR_EQ32BIT_EN;
#endif
#if TCFG_DRC_ENABLE
    req.dec.attr |= AUDIO_ATTR_DRC_EN;
#endif
#endif
#endif
#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    req.dec.effect |= AUDIO_EFFECT_SPECTRUM_FFT;
#endif

    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        goto __err;
    }

    __this->play_time = req.dec.play_time;
    __this->total_time = req.dec.total_time;

    net_download_set_read_timeout(file, 0);

    req.dec.cmd = AUDIO_DEC_START;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    memset(&req, 0, sizeof(req));
    req.dec.cmd = AUDIO_DEC_GET_EFFECT_HANDLE;
    req.dec.effect = AUDIO_EFFECT_SPECTRUM_FFT;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    __this->spectrum_fft_hdl = req.dec.get_hdl;
#endif

    if (__this->play_status == AUDIO_DEC_PP) {
        req.dec.cmd = AUDIO_DEC_PAUSE;
        server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
        __this->play_status = AUDIO_DEC_PAUSE;
        net_download_set_pp(file, 1);
        printf("-> play_status = AUDIO_DEC_PP\n");
    } else {
        net_download_set_pp(file, 0);
    }
    return 0;

__err:
    log_e("play_net_music_faild\n");

    net_download_buf_inactive(file);

    req.dec.cmd = AUDIO_DEC_STOP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    net_download_close(file);
    __this->net_file = NULL;

    if (__this->ai_server) {
        r.evt.event   = AI_EVENT_MEDIA_END;
        r.evt.ai_name   = __this->ai_name;
        ai_server_request(__this->ai_server, AI_REQ_EVENT, &r);
    }
    __this->play_status = AUDIO_DEC_STOP;
    return -1;
}
static int net_music_mode_init(void);
int net_music_dec_file(const char *url)
{
    int err;
    struct net_download_parm parm = {0};

    if (!url || !qyai_sys_connect_net_success()) {
        return -1;
    }
    net_music_mode_init();
    net_music_dec_stop();
    __this->play_status = AUDIO_DEC_START;

    printf("net_download_open url = %s\n", url);
    if (url != &music_url[0][0] && url != &music_url[1][0]) {
        memcpy(&music_url[1][0], &music_url[0][0], sizeof(music_url[0]));
        memset(&music_url[0][0], 0, sizeof(music_url[0]));
        strcpy(&music_url[0][0], url);
    }

    parm.url                = url;
    //网络缓冲buf大小
#ifdef CONFIG_NO_SDRAM_ENABLE
    parm.cbuf_size          = 150 * 1024;
#else
    parm.cbuf_size          = 500 * 1024;
#endif
    //设置网络下载超时
    parm.timeout_millsec    = 10000;
#ifdef CONFIG_DOWNLOAD_SAVE_FILE
    if (storage_device_ready()) {
        parm.save_file			= 1;
        parm.file_dir			= NULL;
    }
#endif
    parm.seek_threshold     = 1024 * 200;	//用户可适当调整
    /* parm.seek_low_range     = __this->dec_bp.fptr;    //恢复断点时设置网络的开始下载地址 */

    err = net_download_open(&__this->net_file, &parm);
    if (err) {
        printf("net_download_open: err = %d\n", err);
        net_music_play_set_stop();
        __this->play_status = AUDIO_DEC_STOP;
        return err;
    }
    /*异步等待网络下载ready，防止网络阻塞导致app_core卡住 */
    __this->wait_download = wait_completion(__net_download_ready, (int (*)(void *))__net_music_dec_file, __this->net_file, NULL);
//    __this->wait_download = wait_completion_timeout_add_to_task("sys_timer", __net_download_ready, (int (*)(void *))__net_music_dec_file, __this->net_file, NULL, 10000);

    return 0;
}
int net_music_play_last(void)
{
    sys_timeout_add_to_task("sys_timer", &music_url[1][0], net_music_dec_file, 1000);//1秒后再创建，否则会在发断开消息到下次音乐请求
    return 0;
}
int net_music_play_last_chack(void)
{
    __this->play_loop = 0;
    if (music_url[1][0] != 0) {
        return 1;
    }
    return 0;
}
int net_music_play_loop(void)
{
    __this->play_loop = 1;
    if (music_url[0][0] != 0) {
        memcpy(&music_url[1][0], &music_url[0][0], sizeof(music_url[0]));
    }
    return 0;
}
int net_music_play_loop_clear(void)
{
    __this->play_loop = 0;
    return 0;
}
//切换上一首或下一首
static int net_music_dec_switch_file(int fsel_mode)
{
    union ai_req req = {0};

    if (!__this->ai_server) {
        return 0;
    }

    log_i("net_music_dec_switch_file\n");

    if (!strcmp(__this->ai_name, "dlna")) {
        return 0;
    }

    net_music_dec_stop();

    if (fsel_mode == FSEL_NEXT_FILE) {
        req.evt.event = AI_EVENT_NEXT_SONG;
    } else if (fsel_mode == FSEL_PREV_FILE) {
        req.evt.event = AI_EVENT_PREVIOUS_SONG;
    } else {
        return 0;
    }

    req.evt.ai_name = __this->ai_name;
    return ai_server_request(__this->ai_server, AI_REQ_EVENT, &req);
}

//获取断点数据
static int net_music_get_dec_breakpoint(struct audio_dec_breakpoint *bp)
{
    int err;
    union audio_req r = {0};

    bp->len = 0;
    r.dec.bp = bp;
    r.dec.cmd = AUDIO_DEC_GET_BREAKPOINT;

    if (bp->data) {
        free(bp->data);
        bp->data = NULL;
    }

    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    if (err) {
        return err;
    }

    if (r.dec.status == AUDIO_DEC_STOP) {
        bp->len = 0;
        free(bp->data);
        bp->data = NULL;
        return -1;
    }
    /* put_buf(bp->data, bp->len); */

    return 0;
}

//设置音量大小
int net_music_set_dec_volume_step(int step)
{
    union audio_req req = {0};
    union ai_req ai = {0};

    if (!__this->net_file) {
        return -1;
    }

    qyai_sys_volume_read(&__this->volume);
    __this->volume = sys_volume_chack(__this->volume + step);

    printf("->set_dec_volume: %d\n", __this->volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    if (__this->ai_server) {
        ai.evt.event       = AI_EVENT_VOLUME_CHANGE;
        ai.evt.arg         = __this->volume;
        ai.evt.ai_name     = __this->ai_name;
        ai_server_request(__this->ai_server, AI_REQ_EVENT, &ai);
    }
    qyai_sys_volume_write(&__this->volume);
    return 0;
}
//设置音量大小
int net_music_set_dec_volume(int volume)
{
    union audio_req req = {0};
    union ai_req ai = {0};

    if (!__this->net_file) {
        return -1;
    }

    __this->volume = sys_volume_chack(volume);

    printf("->set_dec_volume: %d\n", __this->volume);

    qyai_sys_volume_write(&__this->volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    if (__this->ai_server) {
        ai.evt.event       = AI_EVENT_VOLUME_CHANGE;
        ai.evt.arg         = __this->volume;
        ai.evt.ai_name     = __this->ai_name;
        ai_server_request(__this->ai_server, AI_REQ_EVENT, &ai);
    }

    return 0;
}

//获取解码器状态
static int net_music_get_dec_status(void)
{
    union audio_req req = {0};

    req.dec.cmd     = AUDIO_DEC_GET_STATUS;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    return req.dec.status;
}
short *net_music_get_audio_fft(int *len)
{
#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    if (!os_mutex_valid(&__this->mutex) || !__this->spectrum_fft_hdl || __this->play_status == AUDIO_DEC_STOP ||
        __this->play_status == AUDIO_DEC_PAUSE) {
        return NULL;
    }
    os_mutex_pend(&__this->mutex, 6000);
    if (!__this->spectrum_fft_hdl || __this->play_status == AUDIO_DEC_STOP ||
        __this->play_status == AUDIO_DEC_PAUSE) {
        os_mutex_post(&__this->mutex);
        return NULL;
    }
    if (__this->spectrum_fft_hdl) {
        short *db_data = audio_spectrum_fft_get_val(__this->spectrum_fft_hdl);//获取存储频谱值得地址
        if (db_data) {
            int n = audio_spectrum_fft_get_num(__this->spectrum_fft_hdl);
            *len = n;
            os_mutex_post(&__this->mutex);
            return db_data;
//                put_buf(db_data, n * 2);
//                for (int i = 0; i < audio_spectrum_fft_get_num(__this->spectrum_fft_hdl); i++) {
//                    //输出db_num个 db值
////                    printf("db_data db[%d] %d\n", i, db_data[i]); */
//                }
        }
    }
    os_mutex_post(&__this->mutex);
#endif
    return NULL;
}
//快进快退,单位是秒,暂时只支持MP3格式
static int net_music_dec_seek(int seek_step)
{
    int err;
    union audio_req r = {0};

    if (0 == seek_step) {
        return 0;
    }

    if (__this->total_time != 0 && __this->total_time != -1) {
        if (__this->play_time + seek_step <= 0 || __this->play_time + seek_step >= __this->total_time) {
            log_e("local music seek out of range\n");
            return -1;
        }
    }

    if (seek_step > 0) {
        r.dec.cmd = AUDIO_DEC_FF;
        r.dec.ff_fr_step = seek_step;
    } else {
        r.dec.cmd = AUDIO_DEC_FR;
        r.dec.ff_fr_step = -seek_step;
    }

    log_i("net music seek step : %d\n", seek_step);

    return server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
}

static void dec_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
        log_i("net_music: AUDIO_SERVER_EVENT_ERR\n");
        net_music_play_set_stop();
    case AUDIO_SERVER_EVENT_END:
        log_i("net_music: AUDIO_SERVER_EVENT_END\n");
        net_music_dec_end();
        //net_music_play_set_stop();
        break;
    case AUDIO_SERVER_EVENT_CURR_TIME:
        ;
        char buf[32];
        sprintf(buf, "%02d:%02d ", (int)argv[1] / 60, (int)argv[1] % 60);
        puts(buf);
//        printf("play_time: %d\n", argv[1]);
        __this->play_time = argv[1];
        break;
    }
}

static int net_music_mode_init(void)
{
    log_i("net_music_play_main\n");
    if (__this->dec_server) {
        return 0;
    }
    memset(__this, 0, sizeof(struct net_music_hdl));

    os_mutex_create(&__this->mutex);

    qyai_sys_volume_read(&__this->volume);

    __this->ai_name = "unknown";

    __this->dec_server = server_open("audio_server", "dec");
    if (!__this->dec_server) {
        return -1;
    }
    server_register_event_handler_to_task(__this->dec_server, NULL, dec_server_event_handler, "app_core");

    return 0;
}

static void net_music_mode_exit(void)
{
    if (__this->dec_server) {
        net_music_dec_stop();
        server_close(__this->dec_server);
        __this->dec_server = NULL;
        if (__this->ai_server) {
            server_close(__this->ai_server);
            __this->ai_server = NULL;
        }
        free(__this->url);
        __this->url = NULL;
    }
    //os_mutex_del(&__this->mutex, 0);
    memset(__this, 0, sizeof(struct net_music_hdl));
}
