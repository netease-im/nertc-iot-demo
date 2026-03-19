#include "server/audio_server.h"
#include "server/server_core.h"
#include "generic/circular_buf.h"
#include "os/os_api.h"
#include "event.h"
#include "app_config.h"
#include "app_core.h"
#include "action.h"
#include "event/key_event.h"

#if (defined CONFIG_SXY_QYAI_ENABLE)

int app_res_music_stop_get(void *p);
static struct {
    char *first_500ms_buf;
    int first_500ms_buf_size;
    int first_500ms_len;
    int sample_rate;
    int wait_res_music_stop;
    char wakeup;
    char pcm_send;
    char enter_qyai_dialogue;
    char start_speak;
} qyai_info;

enum {
    WAKUP_INIT = 0,
    WAKUP_PCM_VAD,
    WAKUP_KEY_VAD,
};

#define __this (&qyai_info)

static int qyai_pcm_strat_init(void *p)
{
    __this->wait_res_music_stop = 0;
    __this->wakeup = WAKUP_PCM_VAD;
    __this->pcm_send = 0;
    __this->enter_qyai_dialogue = 0;
    printf("->qyai_pcm_strat_init\n");
}
int qyai_pcm_strat(int sample_rate)
{
    if (!__this->first_500ms_buf) {
        __this->sample_rate = sample_rate;
        __this->first_500ms_buf_size = __this->sample_rate * 2 / 2;//500ms
        __this->first_500ms_buf = malloc(__this->first_500ms_buf_size);//500ms
        __this->first_500ms_len = 0;
    }

    //清空标记
    __this->wakeup = 0;
    __this->pcm_send = 0;
    __this->enter_qyai_dialogue = 0;
    __this->first_500ms_len = 0;

    //清空AI对话
    qyai_net_dialogue_clear();

    //播放提示音
    app_play_res_music("rec.mp3");

    //创建qyai对话
    qyai_net_send_pcm_buf_init();
    qyai_net_send_500ms_pcm_buf_push(NULL, 0, 1);
    qyai_net_client_thread_create(1);//参数1则重启对话，0则有对话在线则继续，没有则创建

    //等待提示音播放完成
    if (__this->wait_res_music_stop) {
        wait_completion_del(__this->wait_res_music_stop);
    }
    __this->wait_res_music_stop = wait_completion_add_to_task("sys_timer", app_res_music_stop_get,
                                  (int (*)(void *))qyai_pcm_strat_init, NULL, NULL);
//    //等待提示音播放完成
//    app_waite_res_music(3000);//必须等待音频播放完成才启动
//    qyai_pcm_strat_init();
    return 0;
}
int qyai_pcm_write(char *buf, int len, int sample_rate)
{
    if (!__this->first_500ms_buf) {
        __this->sample_rate = sample_rate;
        __this->first_500ms_buf_size = __this->sample_rate * 2 / 2;//500ms
        __this->first_500ms_buf = malloc(__this->first_500ms_buf_size);//500ms
        __this->first_500ms_len = 0;
    }
    if (__this->first_500ms_buf && len) {
        if ((__this->first_500ms_len + len) <= __this->first_500ms_buf_size) { //还没满buf则继续填满
            memcpy((char *)__this->first_500ms_buf + __this->first_500ms_len, (char *)buf, len);
            __this->first_500ms_len += len;
        } else if (len < __this->first_500ms_buf_size) { //满buf则继续更新最新数据在buf最后区域
            if (__this->first_500ms_len > len) {
                memcpy((char *)__this->first_500ms_buf, (char *)__this->first_500ms_buf + len, __this->first_500ms_len - len);
                memcpy((char *)__this->first_500ms_buf + __this->first_500ms_len - len, (char *)buf, len);
            } else {
                memcpy((char *)__this->first_500ms_buf, (char *)buf, len);
                __this->first_500ms_len = len;
            }
        }
    }
    if (__this->wakeup) {
        if (!__this->enter_qyai_dialogue) {
            if (!qyai_net_license_allow_check()) { //免费授权用完
                __this->wakeup = 0;
                __this->enter_qyai_dialogue = 0;
                __this->pcm_send = 0;
                __this->enter_qyai_dialogue = 0;
                __this->first_500ms_len = 0;
                app_play_res_music("UnbindDef.mp3");//提示被绑定账号
            } else {
                if (__this->start_speak) {
                    if (!__this->pcm_send) {
                        if (qyai_net_send_pcm_buf_start() == 0) { //成功发送起始包
                            if (__this->first_500ms_buf) {
                                qyai_net_send_pcm_buf_push((u8 *)__this->first_500ms_buf, __this->first_500ms_len);//发送数据
                            }
                            qyai_net_send_pcm_buf_push((char *)buf, len);//发送数据
                            __this->pcm_send = true;
                            __this->first_500ms_len = 0;
                        }
                    } else {
                        qyai_net_send_pcm_buf_push((char *)buf, len);//发送数据
                    }
                } else if (__this->pcm_send) {
                    qyai_net_send_pcm_buf_end();//发送结束包
                    __this->pcm_send = 0;
                    __this->enter_qyai_dialogue = true;
                    __this->first_500ms_len = 0;
                }
            }
        }
    } else if (__this->pcm_send) {
        qyai_net_send_pcm_buf_end();//按键下发送结束包
        __this->pcm_send = 0;
        __this->enter_qyai_dialogue = true;
        __this->first_500ms_len = 0;
    }
    return 0;
}
void qyai_pcm_uninit(void)
{
    if (__this->first_500ms_buf) {
        free(__this->first_500ms_buf);
        __this->first_500ms_buf = NULL;
    }
    __this->wakeup = 0;
    __this->pcm_send = 0;
    __this->enter_qyai_dialogue = 0;
}

void qyai_dialogue_start(void)
{
    if (!__this->enter_qyai_dialogue) {
        __this->enter_qyai_dialogue = true;
    }
}
void qyai_dialogue_end(void)
{
    __this->enter_qyai_dialogue = 0;
    puts("-> dialogue_end\n");
}
void qyai_dialogue_timeout(void)
{
    __this->wakeup = 0;
    __this->enter_qyai_dialogue = 0;
    puts("dialogue_timeout_exit\n");
}
static void qyai_key_vad_start_init(void *p)
{
    printf("->aisp_key_vad_start_init\n");
    __this->wait_res_music_stop = 0;
    __this->enter_qyai_dialogue = 0;
    __this->pcm_send = 0;
    __this->start_speak = true;
    __this->wakeup = WAKUP_KEY_VAD;
}
void qyai_key_vad_start(void)
{
    puts("aisp_key_vad_start\n");

    //清空AI对话
    qyai_net_dialogue_clear();

    //清空MIC音频
    __this->first_500ms_len = 0;

    //播放提示音
    app_play_res_music("rec.mp3");

    //创建qyai对话
    qyai_net_send_pcm_buf_init();
    qyai_net_send_500ms_pcm_buf_push(NULL, 0, 1);
    qyai_net_client_thread_create_new(1, 1);//参数1则重启对话，0则有对话在线则继续，没有则创建

    //等待提示音播放完成
    if (__this->wait_res_music_stop) {
        wait_completion_del(__this->wait_res_music_stop);
    }
    __this->wait_res_music_stop = wait_completion_add_to_task("sys_timer", app_res_music_stop_get,
                                  (int (*)(void *))qyai_key_vad_start_init, NULL, NULL);
//    app_waite_res_music(3000);//必须等待音频播放完成才启动
//    aisp_key_vad_start_init();
}
void qyai_key_vad_end(void)
{
    __this->wakeup = 0;
    __this->start_speak = 0;
    puts("aisp_key_vad_end\n");
}
void qyai_speek_start(void)
{
    printf("-> SPEAK_START  %d , %d\n", __this->wakeup, __this->enter_qyai_dialogue);
    if (__this->wakeup == WAKUP_PCM_VAD && !__this->enter_qyai_dialogue) {
        __this->start_speak = true;
        qyai_net_send_pcm_buf_init();
        qyai_net_client_thread_create(0);
    }
}
void qyai_speek_stop(void)
{
    if (__this->start_speak && __this->wakeup == WAKUP_PCM_VAD) {
        __this->start_speak = 0;
    }
}

#endif

