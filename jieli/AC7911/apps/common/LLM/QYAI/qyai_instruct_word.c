#include "web_socket/websocket_api.h"
#include "wifi/wifi_connect.h"
#include "system/includes.h"
#include "event/key_event.h"
#include "os/os_api.h"
#include "fs/fs.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "app_config.h"

#if (defined CONFIG_SXY_QYAI_ENABLE)

//=================系统指令=====================//
//////////////////////////////////////////
static const char *sys_close[] = {
    "关了", "关闭", "关掉", "停了", "退下", "对话关了", "对话关闭", "关掉对话", "对话关掉", "对话停了",
    "结束对话", "关闭对话", "滚", "不想和你说了", "不想和你说话了",
    NULL,
};
static const char *sys_str_power_off[] = {
    "关机",
    NULL,
};
/////////////////////////////////////////
static const char *sys_30min[] = {//先长后短
    "一个半小时过后", "一个半小时以后", "一个半小时后",
    "半个小时过后", "半个小时以后", "半个小时后",
    "半小时过后", "半小时以后", "半小时后",
    NULL,
};
static const char *sys_90min[] = {
    "一个小时半过后", "一个小时半以后", "一个小时半后",
    NULL,
};
/////////////////////////////////////////
static const char *sys_hour_timeout[] = {//先长后短
    "个小时过后", "个小时以后", "个小时后", "小时过后", "小时以后", "小时后",
    NULL,
};
static const char *sys_min_timeout[] = {//先长后短
    "分钟过后", "分钟以后", "分钟后",
    NULL,
};
/////////////////////////////////////////
static const char *sys_countown_timeout[] = {//先长后短
    "倒计时", "计时",
    NULL,
};

//static const char *sys_sec_timeout[] = {//先长后短
//    "秒过后","秒以后","秒后",
//    NULL,
//};
/////////////////////////////////////////
//=================音乐指令=====================//
static const char *sys_music_play[] = {
    "播放", "播放歌曲", "播放音乐",
    NULL,
};
static const char *sys_music_continue[] = {
    "继续播放", "继续播歌", "继续播放音乐",
    NULL,
};
static const char *sys_music_stop[] = {
    "暂停", "暂停播放", "暂停播放音乐",
    NULL,
};
static const char *sys_music_close[] = {
    "停止播歌", "停止播放", "停止播放音乐", "关闭音乐", "音乐关闭", "关闭歌曲", "歌曲关闭", "音乐停止", "停止音乐",
    NULL,
};
static const char *sys_music_next[] = {
    "下一首", "播放下一首", "换一首", "换一首歌", "换一首音乐", "切歌", "换歌",
    NULL,
};
static const char *sys_music_last[] = {
    "上一首", "播放上一首", "播放上一首歌", "播放上一首音乐",
    NULL,
};
static const char *sys_music_loop[] = {
    "单曲循环",
    NULL,
};
//=================设备控制指令=====================//
static const char *sys_music_app_ai[] = {
    "AI模式", "AI智能体模式", "智能体模式",
    NULL,
};
static const char *sys_music_app_udisk[] = {
    "播放U盘", "U盘播放模式", "U盘模式",
    NULL,
};
static const char *sys_music_app_sd[] = {
    "播放SD卡", "播放TF卡", "SD卡播放模式", "TF卡播放模式", "SD卡模式", "TF卡模式",
    NULL,
};
static const char *sys_music_app_bt[] = {
    "播放蓝牙", "蓝牙模式", "蓝牙播放模式",
    NULL,
};
static const char *sys_music_app_aux[] = {
    "线路模式", "线路输入模式",
    NULL,
};
//=================闹钟指令=====================//
static const char *sys_alarm_del[] = {
    "删除闹钟", "取消闹钟", "取消提醒",
    NULL,
};
static const char *sys_alarm_del_all[] = {
    "删除所有闹钟", "取消所有闹钟", "取消所有提醒",
    NULL,
};
//=================音量调节指令=====================//
#define SYS_VOLUME_SET  "音量调到"
static const char *sys_music_volume[] = {
    "小声一点", "音乐小声一点", "音量调低", "音量调小",
    "大声一点", "音乐大声一点", "音量调高", "音量调大",
    SYS_VOLUME_SET,
    NULL,
};


enum {
    MUSIC_INIT = 0,
    MUSIC_STOP,
    MUSIC_CLOSE,
    MUSIC_CONTTINUE,
    MUSIC_PLAY,
    MUSIC_NEXT,
    MUSIC_LAST,
    MUSIC_LOOP,
    MUSIC_SET_VOLUME,
};
enum {
    MUSIC_APP_AI = 0,//下面顺序务必和audio_app_table[]一样
#ifdef CONFIG_BT_MUSIC_MODE_ENABLE
    MUSIC_APP_BT,
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
    MUSIC_APP_USB_DISK,
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    MUSIC_APP_SD,
#endif
#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
    MUSIC_APP_AUX,
#endif
};

int ai_speaker_app(void)
{
    return 1;//如果多个APP进行切换，在这里添加判断
}
static void respon_ok(void *waite)
{
    app_play_res_music("reply.mp3");
    if (waite) {
        app_waite_res_music(3000);
    }
}
static void volume_respon(void *waite, int dec)
{
    respon_ok(waite);
}
static void sys_close_timeout_check(void)
{
//    if(keyworld_start){
//        websockets_free_lbuf_buf();
//        websockets_close_request(1);
//    }
}
static int aisp_sys_power_off(void)
{
    sys_power_poweroff();
}
static int sys_music_app_switch_check(char *word)
{
    int app_status = 0;
    int i;

    for (i = 0; sys_music_app_ai[i] != NULL; i++) { //AI模式
        if (!strcmp(word, sys_music_app_ai[i])) {
            respon_ok(1);//好的
            app_status = MUSIC_APP_AI;
            puts("-> close_dialuge\n");
            goto exit;
        }
    }
#ifdef CONFIG_BT_MUSIC_MODE_ENABLE
    for (i = 0; sys_music_app_bt[i] != NULL; i++) { //蓝牙盘模式
        if (!strcmp(word, sys_music_app_bt[i])) {
            respon_ok(1);//好的
            app_status = MUSIC_APP_BT;
            puts("-> close_dialuge\n");
            goto exit;
        }
    }
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
    for (i = 0; sys_music_app_udisk[i] != NULL; i++) { //U盘模式
        if (!strcmp(word, sys_music_app_udisk[i])) {
            respon_ok(1);//好的
            app_status = MUSIC_APP_USB_DISK;
            puts("-> close_dialuge\n");
            goto exit;
        }
    }
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    for (i = 0; sys_music_app_sd[i] != NULL; i++) { //SD卡模式
        if (!strcmp(word, sys_music_app_sd[i])) {
            respon_ok(1);//好的
            app_status = MUSIC_APP_SD;
            puts("-> close_dialuge\n");
            goto exit;
        }
    }
#endif
#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
    for (i = 0; sys_music_app_aux[i] != NULL; i++) { //AUX模式
        if (!strcmp(word, sys_music_app_aux[i])) {
            respon_ok(1);//好的
            app_status = MUSIC_APP_AUX;
            puts("-> close_dialuge\n");
            goto exit;
        }
    }
#endif
    return 0;
exit:
    //audio_app_mode_num_switch(app_status, 1);
    return 1;//1立马关闭对话
}

int qyai_sys_instruction_word_callback(char *word, char *asr)
{
    struct key_event key = {0};
    int music_status = 0;
    int close_type = 0;
    int i;

    for (i = 0; sys_close[i] != NULL; i++) { //关闭设备对话
        if (!strcmp(word, sys_close[i])) {
            printf("-> close_dialuge : %s %s\n", word, sys_close[i]);
            music_status = MUSIC_CLOSE;
            respon_ok(1);//好的
            //audio_app_mode_num_switch(MUSIC_APP_AI, 0);//关闭系统默认切回AI智能体模式
            close_type = 1;//1立马关闭对话
#if 1//def CONFIG_ASR_POWER_OFF_ENABLE
            if (asr && strstr(asr, "关机") && strlen(asr) <= 9) {
                sys_timeout_add_to_task("sys_timer", NULL, aisp_sys_power_off, 1000);
            } else {
                sys_timeout_add_to_task("sys_timer", NULL, sys_close_timeout_check, 1000);//1秒后检查时钟主动退出，没有则断开连接
            }
#endif
            goto exit;
        }
    }
    for (i = 0; sys_alarm_del_all[i] != NULL; i++) { //删除所有闹钟
        if (!strcmp(word, sys_alarm_del_all[i])) {
            puts("-> larm_del_all\n");
            music_status = MUSIC_CLOSE;
            respon_ok(1);//好的
            //alarm_clean();
            close_type = 1;//1立马关闭对话
            goto exit;
        }
    }

    for (i = 0; sys_music_play[i] != NULL; i++) { //播放歌曲
        if (!strcmp(word, sys_music_play[i])) {
            music_status = MUSIC_PLAY;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_PLAY\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_stop[i] != NULL; i++) { //暂停播放
        if (!strcmp(word, sys_music_stop[i])) {
            music_status = MUSIC_STOP;
            close_type = 2;//2立马关闭对话不恢复音乐播放
            puts("-> MUSIC_STOP\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_close[i] != NULL; i++) { //停止播放
        if (!strcmp(word, sys_music_close[i])) {
            music_status = MUSIC_CLOSE;
            respon_ok(1);//好的
            close_type = 1;//2立马关闭对话不恢复音乐播放
            puts("-> MUSIC_CLOSE\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_continue[i] != NULL; i++) { //继续播放
        if (!strcmp(word, sys_music_continue[i])) {
            music_status = MUSIC_CONTTINUE;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_CONTTINUE\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_next[i] != NULL; i++) { //下一首播放
        if (!strcmp(word, sys_music_next[i])) {
            music_status = MUSIC_NEXT;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_NEXT\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_last[i] != NULL; i++) { //上一首播放
        if (!strcmp(word, sys_music_last[i])) {
            music_status = MUSIC_LAST;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_LAST\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_loop[i] != NULL; i++) { //单曲循环
        if (!strcmp(word, sys_music_loop[i])) {
            respon_ok(1);//好的
            music_status = MUSIC_LOOP;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_LOOP\n");
            goto exit;
        }
    }

    for (i = 0; sys_music_volume[i] != NULL; i++) { //音量条件
        if (!strcmp(word, sys_music_volume[i])) {
            puts("-> MUSIC_VOLUME\n");
            music_status = MUSIC_SET_VOLUME;
            struct key_event key = {0};
            key.action = KEY_EVENT_CLICK;
            key.type = KEY_EVENT_USER;
            if (i < 8) {
                char dec = i < 4 ? 1 : 0;
//                key.value = dec ? KEY_VOLUME_DEC : KEY_VOLUME_INC;
//                key_event_notify(KEY_EVENT_FROM_USER, &key);
//                if(dec && sys_volume_read(NULL) > MIN_VOLUME_VALUE + VOLUME_STEP){
//                    volume_respon(0, dec);
//                }else if(!dec && sys_volume_read(NULL) < MAX_VOLUME_VALUE - VOLUME_STEP){
//                    volume_respon(0, dec);
//                }
                volume_respon(0, dec);
                if (dec) {
                    qyai_sys_volume_step(-10);
                } else if (!dec) {
                    qyai_sys_volume_step(10);
                }
            }
            close_type = 1;//1立马关闭对话
            goto exit;
        } else if (strstr(word, SYS_VOLUME_SET)) {
            puts("-> music vulem set\n");
            music_status = MUSIC_SET_VOLUME;
            int volume = atoi(word + strlen(SYS_VOLUME_SET));
            if (volume >= 10 && volume <= 100) {
                //ai_speaker_volume_set(volume, 0);
                respon_ok(1);//好的
                qyai_sys_volume_write(volume);
                close_type = 1;//1立马关闭对话
                goto exit;
            } else {
                char *n = word + strlen(SYS_VOLUME_SET);
                volume = 50;
                if (!strcmp(n, "十")) {
                    volume = 10;
                } else if (!strcmp(n, "二十")) {
                    volume = 20;
                } else if (!strcmp(n, "三十")) {
                    volume = 30;
                } else if (!strcmp(n, "四十")) {
                    volume = 40;
                } else if (!strcmp(n, "五十")) {
                    volume = 50;
                } else if (!strcmp(n, "六十")) {
                    volume = 60;
                } else if (!strcmp(n, "七十")) {
                    volume = 70;
                } else if (!strcmp(n, "八十")) {
                    volume = 80;
                } else if (!strcmp(n, "九十")) {
                    volume = 90;
                } else if (!strcmp(n, "一百")) {
                    volume = 100;
                }
                //ai_speaker_volume_set(volume, 0);
                respon_ok(1);//好的
                qyai_sys_volume_write(volume);
                close_type = 1;//1立马关闭对话
                goto exit;
            }
        }
    }
    close_type = sys_music_app_switch_check(word);
    return close_type;
exit:
    if (music_status) {
        switch (music_status) {
        case MUSIC_STOP:
            if (ai_speaker_app()) {
                if (net_music_play_pause_status()) {
                    respon_ok(0);//好的
                }
                net_music_play_puase();
            }
//            else{
//                key.type = KEY_EVENT_USER;
//                key.action = KEY_EVENT_CLICK;
//                key.value = KEY_SUPSPEND;
//                key_event_notify(KEY_EVENT_FROM_USER, &key);
//            }
            break;
        case MUSIC_CLOSE:
            if (ai_speaker_app()) {
                net_music_play_set_stop_notic();
            }
//            else{
//                key.type = KEY_EVENT_USER;
//                key.action = KEY_EVENT_CLICK;
//                key.value = KEY_SUPSPEND;
//                key_event_notify(KEY_EVENT_FROM_USER, &key);
//            }
            break;
        case MUSIC_CONTTINUE:
            if (ai_speaker_app()) {
                if (music_buf_play_supspend()) {
                    extern void qyai_net_music_play_resum(void);
                    sys_timeout_add_to_task("sys_timer", NULL, qyai_net_music_play_resum, 1000);//延时1s可能在播放会在播放前socke先关闭有再见的提示音
                } else {
                    net_music_play_next();
                }
            }
//            else{
//                key.type = KEY_EVENT_USER;
//                key.action = KEY_EVENT_CLICK;
//                key.value = KEY_RESUM;
//                key_event_notify(KEY_EVENT_FROM_USER, &key);
//            }
            break;
        case MUSIC_PLAY:
            net_music_play_next();
            break;
        case MUSIC_NEXT:
            if (ai_speaker_app()) {
                net_music_play_next();
            }
//            else{
//                key.type = KEY_EVENT_USER;
//                key.action = KEY_EVENT_CLICK;
//                key.value = KEY_DOWN;
//                key_event_notify(KEY_EVENT_FROM_USER, &key);
//            }
            break;
        case MUSIC_LAST:
            if (ai_speaker_app()) {
                if (net_music_play_last_chack()) {
                    net_music_play_last_request();
                } else {
                    net_music_play_next();
                }
            }
//            else{
//                key.type = KEY_EVENT_USER;
//                key.action = KEY_EVENT_CLICK;
//                key.value = KEY_UP;
//                key_event_notify(KEY_EVENT_FROM_USER, &key);
//            }
            break;
        case MUSIC_LOOP:
            net_music_play_loop();
            break;
//        default:
//            sys_timeout_add_to_task("sys_timer", 0, websockets_client_dialogue_timeout_exit, 1000);
//            break;
        }
    }
    return close_type;
}
static void sys_timeout_stop(void)
{
    if (ai_speaker_app()) {
        qyai_net_dialogue_clear();//清空对话
        qyai_net_music_dec_stop();//停止网络播放
        qyai_dialogue_timeout();//关闭唤醒词唤醒状态
    }
}
static void sys_timeout_power_off(void)
{
    struct intent it;
    struct application *app = get_current_app();
    if (app) {
        init_intent(&it);
        it.name = app->name;
        it.action = ACTION_STOP;    //退出当前模式
        start_app(&it);
    }
    sys_power_poweroff();//关闭时候不提示
}
static void sys_timeout_countdown(void)//倒计时
{
//    struct key_event key = {0};
//    aisp_app_suspend();
//    key.type = KEY_EVENT_USER;
//    key.action = KEY_EVENT_HOLD;
//    key.value = index;
//    key_event_notify(KEY_EVENT_FROM_USER, &key);
}
static int asr_time_min_get(char *start_str, int slen, float *hour)
{
    int i, j, k, n;
    int min_timeout = 0;
    int min_hltimeout = 0;
    int min_10 = 0;
    char fstr = 0;
    float h = 0;
    char *asr = start_str;
    char *find_1 = NULL;
    short *ch_utf8 = NULL;

    char utft2[3] = {0xE4, 0xB8, 0xA4}; //两
    char utft1to10[11][3] = {
        0x00, 0x00, 0x00, //
        0xE4, 0xB8, 0x80, //一
        0xE4, 0xBA, 0x8C, //二
        0xE4, 0xB8, 0x89, //三
        0xE5, 0x9B, 0x9B, //四
        0xE4, 0xBA, 0x94, //五
        0xE5, 0x85, 0xAD, //六
        0xE4, 0xB8, 0x83, //七
        0xE5, 0x85, 0xAB, //八
        0xE4, 0xB9, 0x9D, //九
        0xE5, 0x8D, 0x81, //十
    };
    if (asr) {
        put_buf(asr, slen);
        j = slen;//你帮我在二十五分钟后关闭，你帮我在25分钟后关闭
        if (j == 1) { //1-9
            j--;
            if (asr[j] >= '0' && asr[j] <= '9') {
                min_timeout = asr[j] - '0';
                printf("单数字timeout : %d \n", min_timeout);
            }
        } else {
            k = j - 1;
            while (j > 0) {
                if (asr[k] >= '0' && asr[k] <= '9') {
                    while (--k) {
                        if ((asr[k] >= '0' && asr[k] <= '9')) {
                            continue;
                        }
                        if (asr[k] == '.') {
                            fstr = 1;
                            continue;
                        } else {
                            break;
                        }
                    }
                    find_1 = &asr[k];
                    if (fstr) {
                        h = atof(find_1);
                        if (h <= 0) {
                            h = 0;
                        } else if (hour) {
                            *hour = h;
                        }
                        min_timeout = (int)h;
                    } else {
                        min_timeout = atoi(find_1);
                        if (min_timeout < 0) {
                            min_timeout = 0;
                        }
                    }
                    printf("多数字timeout : %d \n", min_timeout);
                    break;
                } else {
                    int f = 0;
                    for (int n = 0; n < 11; n++) {
                        if (memcmp(&utft1to10[n][0], &asr[k - 2], 3) == 0 || memcmp(&utft2, &asr[k - 2], 3) == 0) {
                            if (memcmp(&utft2, &asr[k - 2], 3) == 0) {
                                f = 2;
                            } else {
                                f = n;
                            }
                            if ((min_hltimeout & 0xFF) == 0 && f < 10 && min_10 == 0) { //低八位=0
                                min_hltimeout |= f;
                            } else {
                                if (f == 10 && !min_10) { //低八位=0
                                    min_10 = 10;
                                } else if (f < 10) {
                                    if (((min_hltimeout >> 8) & 0xFF) == 0 && f < 10) { //高八位=0
                                        min_hltimeout |= (f << 8);
                                    }
                                }
                            }
                            break;
                        }
                    }
                    if (!f) { //不是一到十汉字
                        printf("->no found 一到十\n");
                        break;
                    }
                    j -= 3;
                    k -= 3;
                }
            }
            if (!min_timeout && (min_10 || min_hltimeout)) {
                min_timeout = (min_hltimeout >> 8) * 10 + (min_hltimeout & 0xFF);
                printf("汉字timeout : %d \n", min_timeout);
            }
        }
    }
    return min_timeout;
}
static int sys_timeout_stop_off_callback(char *asr, int len)
{
    struct key_event key = {0};
    int music_status = 0;
    int close_type = 0;
    float hour = 0;
    int i, j, k;
    int min_timeout = 0;
    int timeout = 0;
    char *find = NULL;
    char src_str[128];

#ifdef CONFIG_ASR_POWER_OFF_ENABLE
    if (asr && strstr(asr, "关机") && strlen(asr) <= 9) {
        int aisp_sys_power_off(void);
        sys_timeout_add_to_task("sys_timer", NULL, aisp_sys_power_off, 1000);
        close_type = 1;//1立马关闭对话
        goto exit;
    }
#endif
    //printf("->asr = %s \n",asr);
    for (i = 0; sys_30min[i] != NULL; i++) { //关闭
        for (j = 0; sys_close[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_30min[i], sys_close[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 30;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->30min_timeout close \n");
                sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                goto exit;
            }
        }
    }
    for (i = 0; sys_30min[i] != NULL; i++) { //暂停音乐
        for (j = 0; sys_music_stop[j] != NULL; j++) { //暂停音乐
            sprintf(src_str, "%s%s", sys_30min[i], sys_music_stop[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 30;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->30min_timeout close \n");
                sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                goto exit;
            }
        }
    }
    for (i = 0; sys_30min[i] != NULL; i++) { //关闭音乐
        for (j = 0; sys_music_close[j] != NULL; j++) { //关闭音乐
            sprintf(src_str, "%s%s", sys_30min[i], sys_music_close[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 30;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->30min_timeout close \n");
                sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                goto exit;
            }
        }
    }
    for (i = 0; sys_30min[i] != NULL; i++) {
        for (j = 0; sys_str_power_off[j] != NULL; j++) { //关机
            sprintf(src_str, "%s%s", sys_30min[i], sys_str_power_off[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 30;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->30min_timeout off \n");
                sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_power_off, timeout);
                goto exit;
            }
        }
    }
    for (i = 0; sys_90min[i] != NULL; i++) {
        for (j = 0; sys_close[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_90min[i], sys_close[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 90;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->90min_timeout close \n");
                sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                goto exit;
            }
        }
    }
    for (i = 0; sys_90min[i] != NULL; i++) {
        for (j = 0; sys_music_stop[j] != NULL; j++) { //关闭音乐
            sprintf(src_str, "%s%s", sys_90min[i], sys_music_stop[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 90;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->90min_timeout close \n");
                sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                goto exit;
            }
        }
    }
    for (i = 0; sys_90min[i] != NULL; i++) {
        for (j = 0; sys_music_close[j] != NULL; j++) { //关闭音乐
            sprintf(src_str, "%s%s", sys_90min[i], sys_music_close[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 90;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->90min_timeout close \n");
                sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                goto exit;
            }
        }
    }
    for (i = 0; sys_90min[i] != NULL; i++) {
        for (j = 0; sys_str_power_off[j] != NULL; j++) { //关机
            sprintf(src_str, "%s%s", sys_90min[i], sys_str_power_off[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 90;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->90min_timeout off \n");
                sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_power_off, timeout);
                goto exit;
            }
        }
    }
    for (i = 0; sys_min_timeout[i] != NULL; i++) { //n分钟
        for (j = 0; sys_close[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_min_timeout[i], sys_close[j]);
            //printf("-> min asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if ((hour / 60) > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 1000;
                    } else {
                        if ((min_timeout / 60) > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->min_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_min_timeout[i] != NULL; i++) { //n分钟
        for (j = 0; sys_music_stop[j] != NULL; j++) { //关闭音乐
            sprintf(src_str, "%s%s", sys_min_timeout[i], sys_music_stop[j]);
            //printf("-> min asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if ((hour / 60) > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 1000;
                    } else {
                        if ((min_timeout / 60) > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->min_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_min_timeout[i] != NULL; i++) { //n分钟
        for (j = 0; sys_music_close[j] != NULL; j++) { //关闭音乐
            sprintf(src_str, "%s%s", sys_min_timeout[i], sys_music_close[j]);
            //printf("-> min asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if ((hour / 60) > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 1000;
                    } else {
                        if ((min_timeout / 60) > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->min_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_min_timeout[i] != NULL; i++) { //n分钟
        for (j = 0; sys_str_power_off[j] != NULL; j++) { //关机
            sprintf(src_str, "%s%s", sys_min_timeout[i], sys_str_power_off[j]);
            //printf("-> hour asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if ((hour / 60) > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 1000;
                    } else {
                        if ((min_timeout / 60) > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->min_timeout off : %d %0.2f %d\n", min_timeout, hour, timeout);
                    sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_power_off, timeout);
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_hour_timeout[i] != NULL; i++) { //n小时
        for (j = 0; sys_close[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_hour_timeout[i], sys_close[j]);
            //printf("-> hour asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if (hour > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 60 * 1000;
                    } else {
                        if (min_timeout > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->hour_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_hour_timeout[i] != NULL; i++) { //n小时
        for (j = 0; sys_music_stop[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_hour_timeout[i], sys_music_stop[j]);
            //printf("-> hour asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if (hour > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 60 * 1000;
                    } else {
                        if (min_timeout > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->hour_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_hour_timeout[i] != NULL; i++) { //n小时
        for (j = 0; sys_music_close[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_hour_timeout[i], sys_music_close[j]);
            //printf("-> hour asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if (hour > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 60 * 1000;
                    } else {
                        if (min_timeout > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->hour_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_stop, timeout);
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_hour_timeout[i] != NULL; i++) { //n小时
        for (j = 0; sys_str_power_off[j] != NULL; j++) { //关机
            sprintf(src_str, "%s%s", sys_hour_timeout[i], sys_str_power_off[j]);
            //printf("-> hour asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        timeout = hour * 60 * 60 * 1000;
                        if (hour > 72) {
                            return 0;
                        }
                    } else {
                        if (min_timeout > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->hour_timeout off : %d %0.2f %d\n", min_timeout, hour, timeout);
                    sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_power_off, timeout);
                    goto exit;
                }
            }
        }
    }

    for (i = 0; sys_countown_timeout[i] != NULL; i++) { //计时三十五分钟，计时三十五秒，计时三十五小时，
        find = strstr(asr, sys_countown_timeout[i]);
        if (find) {
            find += strlen(sys_countown_timeout[i]);
            k = 0;
            char sec_mode = 0;
            char *end = strstr(find, "小时");
            if (end) {
                k = end - find;
                sec_mode = 0;
            } else {
                end = strstr(find, "分钟");
                if (end) {
                    k = end - find;
                    sec_mode = 1;
                } else {
                    end = strstr(find, "秒");
                    if (end) {
                        k = end - find;
                        sec_mode = 2;
                    }
                }
            }
            if (k > 0 && k < 10) {
                min_timeout = asr_time_min_get(find, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0 && sec_mode == 0) {
                        timeout = hour * 60 * 60 * 1000;
                        if (hour > 72) {
                            return 0;
                        }
                    } else {
                        if (sec_mode == 2) {
                            timeout = min_timeout * 1000;
                        } else if (sec_mode == 1) {
                            timeout = min_timeout * 60 * 1000;
                        } else if (sec_mode == 0) {
                            if (min_timeout > 72) {
                                return 0;
                            }
                            timeout = min_timeout * 60 * 60 * 1000;
                        }
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->timeout countdown : %d sec\n", timeout / 1000);
                    sys_timeout_add_to_task("sys_timer", NULL, sys_timeout_countdown, timeout);
                    goto exit;
                }
            }
        }
    }

    return 0;
exit:
    respon_ok(1);//好的
    return close_type;
}

int qyai_asr_txt_recv_callback(char *buf, int len)//AI对话语音识别文本回调
{
    int ret = sys_timeout_stop_off_callback(buf, len);
    return ret;
}
int qyai_dialogue_txt_recv_callback(char *buf, int len)//AI对话文本回调
{
    printf("resp_txt : %s \n", buf);
}
#endif
