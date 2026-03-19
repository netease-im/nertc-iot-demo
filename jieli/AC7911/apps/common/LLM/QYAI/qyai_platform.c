#include "system/includes.h"
#include "action.h"
#include "app_config.h"
#include "storage_device.h"
#include "generic/log.h"
#include "os/os_api.h"
#include "event/key_event.h"
#include "event/bt_event.h"
#include "event/device_event.h"
#include "event/net_event.h"
#include "wifi/wifi_connect.h"
#include "asm/rtc.h"
#include "asm/system_reset_reason.h"
#include "syscfg_id.h"

#if (defined CONFIG_SXY_QYAI_ENABLE)
int app_play_res_music(const char *name);
int app_stop_res_music(void);
int app_set_res_music_volume(char volume);
int app_get_res_music_volume(char *volume);
int app_waite_res_music(int timeout);


/***********************************************************************************************************************
 *以下为：轻语AI的用户产品接口(默认值只能是测试，随便修改则无法使用)，生产产品需要商务联系深圳市数芯语科技
***********************************************************************************************************************/
int app_user_product_batch(void)
{
    return 1;//批次号
}
int app_user_product_batch_num(void)
{
    return 1;//批次号的数量，单位K
}
char *app_user_product_uuid(void)
{
    return NULL;//客户ID(数芯语提供),如："123456"
}
char *app_user_product_name(void)
{
    return NULL;//产品名称(数芯语提供)，如："AIBOX"的英文名称2-6字符
}

/**********************************************************************************************************************
 *以下为：轻语AI的平台API接口，默认只提供有限度的对话和音乐测试，需要修改请商务联系
***********************************************************************************************************************/
int qyai_sys_connect_net_success(void)//获取网络状态
{
    return (wifi_get_sta_connect_state() == WIFI_STA_NETWORK_STACK_DHCP_SUCC);
}
char *qiay_bt_get_local_name(void)//设置蓝牙名称
{
    return "QYAI_AC";//名称不能改，否则小程序无法识别（用户如果自行开发小程序可进行修改），配网蓝牙名称：QYAI_AC_XXXX
}
void qyai_get_local_mac(char *mac)//获取MAC，需要保证唯一性
{
    if (mac) {
        init_net_device_mac_addr(mac, 0);
    }
}
void qyai_net_dialogue_clear(void)//清空AI对话
{
    //清空qyai的buf 请求断开
    qyai_net_free_lbuf_buf();
    qyai_net_close_request(1);
    qyai_net_dialogue_timeout_del();

    //清空停止qyai的音频播放
    qyai_music_buf_play_stop_all();
    qyai_music_buf_play_set_stop();
    qyai_music_buf_play_free_lbuf();
    qyai_music_buf_play_stop_waite();//等待关闭完成才能播放提示音
}
int qyai_net_music_dec_file(char *url)//网络解码
{
    net_music_dec_file(url);
    return 0;
}
void qyai_net_music_dec_stop(void)//停止网络解码
{
    net_music_dec_stop();
}
int qyai_net_music_play_pause(int pause)//暂停网络解码
{
    return net_music_play_pause(pause);
}
int qyai_net_music_play_pause_status(void)//获取暂停网络解码状态
{
    return net_music_play_pause_status();
}
int qyai_net_music_play_start_status(void)//获取启动网络解码状态
{
    return net_music_play_start_status();
}
int qyai_net_music_play_loop_clear(void)//清空单曲循环
{
    return net_music_play_loop_clear();
}
int qyai_net_music_play_loop(void)//设置单曲循环
{
    return net_music_play_loop();
}
int qyai_net_music_play_last(void)//播放上一首
{
    return net_music_play_last();
}
int qyai_net_music_play_last_chack(void)//检测释放上一首可用
{
    return net_music_play_last_chack();
}
int qyai_music_play_stop_all(void)//停止所有应用层音频播放
{
    return app_stop_res_music();
}
void qyai_dialogue_start_callback(void)//对话开始回调
{
    qyai_dialogue_start();
}
void qyai_dialogue_end_callback(void)//对话结束回调
{
    qyai_dialogue_end();
}
void qyai_net_dialogue_timeout_exit(char notice)//6秒对话超时回调
{
    int qyai_net_recv_net_music_valid(void);//有接收到网络音乐
    int qyai_net_music_supspend_get(void);//语音设置的暂停模式
    int status = net_music_play_pause_status();
    int sup = !qyai_net_music_supspend_get();//不是语音设置的暂停模式则恢复播放

    qyai_dialogue_timeout();//清空VAD和kws标记

    if (status && sup) { //网络音乐处于暂停，并且不是语音控制的恢复网络播放
        qyai_net_music_play_resum(0);
    } else if (!qyai_net_recv_net_music_valid() && notice) { //提示音
        app_play_res_music("ring.mp3");
    }
}
void qyai_audio_play_start_callback(void)//对话音频开始播放
{
}
void qyai_audio_play_end_callback(void)//对话音频结束播放
{
}
void qyai_mic_gain_suspend(void)//麦克风增益挂起
{
}
void qyai_mic_gain_resum(void)//麦克风增益恢复
{
}
int qyai_music_play_server_err(void)//服务器出错
{
    return 0;
}
int qyai_music_play_instruc_word_err(void)//不支持的命令
{
    return 0;
}
int qyai_sys_volume_read(char *volume)//读取音量大小
{
    return app_get_res_music_volume(volume);
}
int qyai_sys_volume_write(char volume)//设置音量大小
{
    return app_set_res_music_volume(volume);
}
int qyai_sys_volume_step(char step)//设置音量大小梯度
{
    return app_set_res_music_volume_step(step);
}
int qyai_sys_alarm_wakup_reset_get(void)//闹钟唤醒复位获取
{
    return system_reset_reason_get() == SYS_RST_ALM_WKUP;
}
int qyai_alarm_save_url(char *url)//保存闹钟的MP3链接，可以保存在flash
{
    return 0;
}
int qyai_alarm_read_url(char *url, int url_buf_size)//读取闹钟的MP3链接，可以从flash读取，返回值：实际读取字节大小
{
    return 0;
}
int qyai_music_play_res_file(void)//闹钟资源提示音回调
{
    app_play_res_music("reminder.mp3");
    return 0;
}
void qyai_alarm_music_play_start(void)//闹钟开始播放回调
{
}
void qyai_sys_power_poweroff(void)//系统关机
{
    /* sys_power_poweroff(); */
}
void qyai_sys_alarm_wakeup_clear(void)//清空系统闹钟唤醒标记
{
    /* system_alarm_wakeup_clear(); */
}
int qyai_alarm_info_read(void *buf, int size)//读取flash的闹钟数据
{
    //return syscfg_read(CFG_USER_ALARM, buf, size);
    return -1;
}
int qyai_alarm_info_write(void *buf, int size)//闹钟数据保存到flash
{
    //return syscfg_write(CFG_USER_ALARM, buf, size);
    return 0;
}



#else
//#error "NO define CONFIG_SXY_QYAI_ENABLE"
#endif
