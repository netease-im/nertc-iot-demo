/**
 * @copyright
 *
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright(C) 2018 - 2021 THL A29 Limited, a Tencent company.All rights reserved.
 *
 * Licensed under the MIT License(the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file mqtt_sample.c
 * @brief a simple sample for mqtt client
 * @author fancyxu (fancyxu@tencent.com)
 * @version 1.0
 * @date 2021-05-31
 *
 * @par Change Log:
 * <table>
 * <tr><th>Date       <th>Version <th>Author    <th>Description
 * <tr><td>2021-05-31 <td>1.0     <td>fancyxu   <td>first commit
 * <tr><td>2021-07-08 <td>1.1     <td>fancyxu   <td>fix code standard of IotReturnCode and QcloudIotClient
 * </table>
 */

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qcloud_iot_common.h"

#include "utils_log.h"
#include "twetalk_ws.h"
#include "data_template_app.h"
#include "ota_downloader.h"
#include "test_audio_opus.h"
#include "tc_iot_hal.h"

/**
 * @brief TWeTalk实例句柄
 */
static void *sg_twetalk_handle = NULL;

static TWeTalkAudioType sg_audio_type = TWETALK_AUDIO_TYPE_OPUS;


/**
 * @brief MQTT event callback, @see MQTTEventHandleFun
 *
 * @param[in] client pointer to mqtt client
 * @param[in] handle_context context
 * @param[in] msg msg
 */
static void _mqtt_event_handler(void *client, void *handle_context, MQTTEventMsg *msg)
{
    MQTTMessage *mqtt_message = (MQTTMessage *)msg->msg;
    uintptr_t    packet_id    = (uintptr_t)msg->msg;

    switch (msg->event_type) {
    case MQTT_EVENT_UNDEF:
        Log_i("undefined event occur.");
        break;

    case MQTT_EVENT_DISCONNECT:
        Log_i("MQTT disconnect.");
        break;

    case MQTT_EVENT_RECONNECT:
        Log_i("MQTT reconnect.");
        break;

    case MQTT_EVENT_PUBLISH_RECEIVED:
        Log_i("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
              mqtt_message->topic_len, STRING_PTR_PRINT_SANITY_CHECK(mqtt_message->topic_name),
              mqtt_message->payload_len, STRING_PTR_PRINT_SANITY_CHECK((char *)mqtt_message->payload));
        break;
    case MQTT_EVENT_SUBSCRIBE_SUCCESS:
        Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
        break;

    case MQTT_EVENT_SUBSCRIBE_TIMEOUT:
        Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
        break;

    case MQTT_EVENT_SUBSCRIBE_NACK:
        Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBE_SUCCESS:
        Log_i("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBE_TIMEOUT:
        Log_i("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBE_NACK:
        Log_i("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
        break;

    case MQTT_EVENT_PUBLISH_SUCCESS:
        Log_i("publish success, packet-id=%u", (unsigned int)packet_id);
        break;

    case MQTT_EVENT_PUBLISH_TIMEOUT:
        Log_i("publish timeout, packet-id=%u", (unsigned int)packet_id);
        break;

    case MQTT_EVENT_PUBLISH_NACK:
        Log_i("publish nack, packet-id=%u", (unsigned int)packet_id);
        break;
    default:
        Log_i("Should NOT arrive here.");
        break;
    }
}

/**
 * @brief Setup MQTT construct parameters.
 *
 * @param[in,out] initParams @see MQTTInitParams
 * @param[in] device_info @see DeviceInfo
 * @brief 设置MQTT连接初始化参数
 *
 * @param[in,out] init_params MQTT初始化参数
 * @param[in] device_info 设备信息
 */
static void _setup_connect_init_params(MQTTInitParams *init_params, DeviceInfo *device_info)
{
    init_params->device_info = device_info;
    // init_params->host              = "pre.ap-guangzhou.iothub.tencentdevices.com";  //"183.47.110.23";  // test
    init_params->event_handle.h_fp = _mqtt_event_handler;
}

// ----------------------------------------------------------------------------
// OTA callback
// ----------------------------------------------------------------------------

void _update_firmware_callback(UtilsJsonValue version, UtilsJsonValue url, UtilsJsonValue md5sum, uint32_t file_size,
                               void *usr_data)
{
    Log_i("recv firmware: version=%.*s|url=%.*s|md5sum=%.*s|file_size=%u", version.value_len, version.value,
          url.value_len, url.value, md5sum.value_len, md5sum.value, file_size);
    // only one firmware one time is supportted now
    OTAFirmwareInfo firmware_info;
    memset(&firmware_info, 0, sizeof(OTAFirmwareInfo));
    strncpy(firmware_info.version, version.value, version.value_len);
    strncpy(firmware_info.md5sum, md5sum.value, md5sum.value_len);
    firmware_info.file_size = file_size;
    ota_downloader_info_set(&firmware_info, url.value, url.value_len);
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------
/**
 * @brief 主循环退出标志
 */
static int sg_main_exit = 0;
// twetalk ws推出标志
static int sg_twetalk_ws_exit = 0;

/**
 * @brief Twetalk音频接收回调,请自行处理接收到的音频数据
 *
 * @param[in] recv_data 接收到的音频数据
 * @param[in] recv_len 数据长度
 * @param[in] context 用户上下文
 * @return 0成功，非0失败
 */
static int _twetalk_recv_audio_cb(uint8_t *recv_data, int recv_len, void *context)
{
    if (sg_main_exit) {
        return 0;
    }
    _device_write_voice_data(recv_data, recv_len);
    return 0;
}

/**
 * @brief twetalk事件接收回调
 * @note 该回调在TWeTalk线程中执行，不要做耗时操作
 * @param[in] type 事件类型
 * @param[in] msg 事件消息
 * @param[in] context 用户上下文
 * @return 0成功，非0失败
 */
static int _twetalk_recv_event_cb(TWeTalkEventType type, TWeTalkEventMsg *msg, void *context)
{
    tc_twetalk_call_event_type_print(type);
    switch (type) {
    /**< 机器人开始讲话 */
    case TWETALK_EVENT_BOT_START_SPEAKING:
        Log_i("bot start speaking");
        break;

    /**< 机器人停止讲话 */
    case TWETALK_EVENT_BOT_STOP_SPEAKING:
        Log_i("bot stop speaking");
        break;

    /**< 机器人讲话字幕，可做UI显示，UTF-8编码 */
    case TWETALK_EVENT_BOT_TRANSCRIPTION: {
        for (int i = 0; i < msg->BotTranscription.transcription.value_len; i ++) {
            printf("%c", msg->BotTranscription.transcription.value[i]);
        }
        printf("\n");
//            Log_i("bot(%d): %.*s", msg->BotTranscription.transcription.value_len, msg->BotTranscription.transcription.value_len,
//                  msg->BotTranscription.transcription.value);
    }
    break;

    /**< 用户讲话字幕，可做UI显示，UTF-8编码 */
    case TWETALK_EVENT_USR_TRANSCRIPTION: {
        for (int i = 0; i < msg->UsrTranscription.transcription.value_len; i ++) {
            printf("%c", msg->UsrTranscription.transcription.value[i]);
        }
        printf("\n");
//            Log_i("usr(%d): %.*s", msg->UsrTranscription.transcription.value_len,msg->UsrTranscription.transcription.value_len,
//                  msg->UsrTranscription.transcription.value);
    }
    break;

    /**< 收到小程序呼叫，可做UI显示，可以接听/挂断/不理会 */
    case TWETALK_EVENT_RECV_ROOMID: {
        Log_i("calling roomid: %.*s", msg->RecvCalling.room_id.value_len, msg->RecvCalling.room_id.value);
        // TODO 自定义是否接听
        tc_twetalk_call_response(sg_twetalk_handle, TWETALK_EVENT_DEVICE_ANSWER,
                                 &msg->RecvCalling.room_id);  // 接听
        // tc_twetalk_call_response(sg_twetalk_handle, TWETALK_EVENT_DEVICE_REJECTS, &msg->RecvCalling.room_id); //
        // 拒接
    }
    break;

    /**< 设备呼叫小程序，小程序接听 */
    case TWETALK_EVENT_RECV_USR_ANSWER: {
        Log_i("user answer called: %.*s openid: %.*s", msg->UserAnswer.called.value_len,
              msg->UserAnswer.called.value, msg->UserAnswer.openid.value_len, msg->UserAnswer.openid.value);
    }
    break;

    /**< 小程序挂断 */
    case TWETALK_EVENT_RECV_USR_HANGUP: {
        Log_i("user hangup(%.*s) called: %.*s openid: %.*s", msg->UserHangup.stream.value_len,
              msg->UserHangup.stream.value, msg->UserHangup.called.value_len, msg->UserHangup.called.value,
              msg->UserHangup.openid.value_len, msg->UserHangup.openid.value);
    }
    break;

    /**< 设备呼叫小程序，发生错误❌ */
    case TWETALK_EVENT_RECV_USR_ERROR: {
        Log_w("code : %d", msg->UserError.code);
    }
    break;

/**< 接收错误 */  // TODO : 错误处理
    case TWETALK_EVENT_RECV_ERROR: {
        // TODO 接收错误应该断掉ws连接（不可在回调中调用tc_twetalk_ws_disconnect） 然后根据情况重连
        // msg->RecvError.code <= -1000 为websocket错误码，可参考websocket错误码
        Log_e("recv error: %d", msg->RecvError.code);
        // sg_twetalk_ws_exit = 1;
        sg_main_exit = 1;
    }
    break;
    default:
        Log_w("unknown event type: %d", type);
        break;
    }
    return 0;
}

static int _twetalk_send_opus_audio(void *handle)
{
    /* Log_i("start push opus audio, send_size: %d", TEST_OPUS_OUTPUT_SIZE); */
    /* for (int i = 0; i < TEST_OPUS_OUTPUT_SIZE; i += 180) { */
    /* tc_twetalk_ws_send_audio(handle, (uint8_t *)test_opus_output + i, 180); */
    /* HAL_SleepMs(60); */
    /* } */
#define DEFAULT_READ_SIZE 180
    char audio_buffer[DEFAULT_READ_SIZE];
    while (1) {
        int ret = _device_get_voice_data(audio_buffer, DEFAULT_READ_SIZE);
        if (ret == DEFAULT_READ_SIZE) {
            tc_twetalk_ws_send_audio(handle, (uint8_t *)audio_buffer, DEFAULT_READ_SIZE);
        }
    }
    return 0;
}

static void _twetalk_send_pcm_audio(void *handle)
{
    return;
}

static void _twetalk_send_audio_thread_entry(void *arg)
{
    void *handle = arg;
    if (handle == NULL) {
        Log_e("invalid param");
        return;
    }
    HAL_SleepMs(10000);  // 等说完开场白
    if (sg_audio_type == TWETALK_AUDIO_TYPE_PCM) {
        _twetalk_send_pcm_audio(handle);
    } else {
        _twetalk_send_opus_audio(handle);
    }
    HAL_SleepMs(5000);
    Log_i("send audio thread exit");
    sg_main_exit = IOT_BOOL_TRUE;
}


void twetalk_thread_entry(void *params)
{

    int  rc;
    char buf[512];

    //支持的音频格式有pcm和opus，都是16K采样率，pcm一帧20ms 640字节，opus一帧60ms 180字节，默认opus格式
    audio_stream_init(16000, 16, 1); //初始化音频流收发
    start_audio_stream(); //开启音频流收发
    // 1. init log level
    LogHandleFunc func = DEFAULT_LOG_HANDLE_FUNCS;
    utils_log_init(func, LOG_LEVEL_DEBUG, 2048);
    // 请参考文档：https://doc.weixin.qq.com/doc/w3_AJkAsAbTADsCNhIeEPMZsTXCETJk6?scode=AJEAIQdfAAoJOzOh4fAJkAsAbTADs 获取
    static DeviceInfo device_info = {
        .product_id     = "LTIHOHJW7F",
        .device_name    = "xiaoxing_04",
        .device_secret  = "设备密钥",
        .device_version = "linux_1.0.0",
    };

    // 2. init connection
    MQTTInitParams init_params = DEFAULT_MQTT_INIT_PARAMS;
    _setup_connect_init_params(&init_params, &device_info);

    // 3. create MQTT client and connect with server
    void *client = IOT_MQTT_Construct(&init_params);
    if (client) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("MQTT Construct failed!");
        return ;
    }

    // 4. init data template
    rc = usr_data_template_init(client);
    if (rc) {
        Log_e("usr data template init failed: %d", rc);
        IOT_MQTT_Destroy(&client);
        return ;
    }

    // 5. init ota
    IotOTAUpdateCallback ota_callback = {
        .update_firmware_callback      = _update_firmware_callback,
        .report_version_reply_callback = NULL,
    };

    rc = IOT_OTA_Init(client, ota_callback, NULL);
    if (rc) {
        Log_e("OTA init failed!, rc=%d", rc);
        usr_data_template_deinit(client);
        IOT_MQTT_Destroy(&client);
        return ;
    }

    rc = IOT_OTA_ReportVersion(client, buf, sizeof(buf), device_info.device_version);
    if (rc) {
        Log_e("OTA report version failed!, rc=%d", rc);
        usr_data_template_deinit(client);
        IOT_MQTT_Destroy(&client);
        return ;
    }

    rc = ota_downloader_init(client);
    if (rc) {
        Log_e("OTA downloader init failed!, rc=%d", rc);
        usr_data_template_deinit(client);
        IOT_MQTT_Destroy(&client);
        return ;
    }

    // 6. init twetalk
    TWeTalkWsInitParams twetalk_params = DEFAULT_TWETALK_WS_INIT_PARAMS;
    twetalk_params.mqtt_client         = client;
    twetalk_params.recv_audio_cb       = _twetalk_recv_audio_cb;
    twetalk_params.recv_event_cb       = _twetalk_recv_event_cb;
    twetalk_params.context             = sg_twetalk_handle;
    twetalk_params.audio_type          = TWETALK_AUDIO_TYPE_OPUS;//音频格式
    sg_audio_type                      = twetalk_params.audio_type;
    if (sg_audio_type == TWETALK_AUDIO_TYPE_PCM) {
        // ! PCM 16kHz, 16bit, 单声道 20ms帧长收发(640字节)
        twetalk_params.frame_interval           = 20;
        twetalk_params.push_recv_frame_interval = 20;  // ! 如果收到的音频播放有卡顿，适当减小该值
        twetalk_params.ringbuffer_size          = 20 * 640;
    } else {
        // ! OPUS 16kHz, 16bit, 单声道 60ms帧长收发(180字节)
        twetalk_params.frame_interval           = 60;
        twetalk_params.push_recv_frame_interval = 60;  // ! 如果收到的音频播放有卡顿，适当减小该值
        twetalk_params.ringbuffer_size          = 20 * 180;
    }

    twetalk_params.wxa_appid   = "wx9e8fbc98ceac2628";
    twetalk_params.wxa_modelid = "DYEbVE9kfjAONqnWsOhXgw";
    sg_twetalk_handle          = tc_twetalk_ws_init(&twetalk_params);
    if (sg_twetalk_handle == NULL) {
        Log_e("TWeTalk WebSocket init failed!");
        usr_data_template_deinit(client);
        ota_downloader_deinit();
        IOT_OTA_Deinit(client);
        IOT_MQTT_Destroy(&client);
        return ;
    }

    // TODO 根据实际情况
    usr_report_battery(client, 100);
    usr_report_volume(client, 80);

    TWeCallOpenids openids[1];
    memset(openids, 0, sizeof(openids));
    strcpy(openids[0].name, "张三");
    strcpy(openids[0].open_id, "oOMCN5CR14xUh7vBmxRQdSYjsLek");  // p2p player --> xph
    tc_twetalk_call_sync_openids(sg_twetalk_handle, openids, 1);

    // create thread to send audio
    ThreadParams send_audio_thread_params;
    send_audio_thread_params.thread_name = "_twetalk_send_audio_thread";
    send_audio_thread_params.thread_func = _twetalk_send_audio_thread_entry;
    send_audio_thread_params.user_arg    = sg_twetalk_handle;
    send_audio_thread_params.stack_size  = 1024 * 20;
    send_audio_thread_params.priority    = THREAD_PRIORITY_NORMAL;
    rc                                   = HAL_ThreadCreate(&send_audio_thread_params);
    if (rc) {
        Log_e("ai talk create recv thread failed(%d)\n", rc);
        usr_data_template_deinit(client);
        ota_downloader_deinit();
        IOT_OTA_Deinit(client);
        IOT_MQTT_Destroy(&client);
        tc_twetalk_ws_exit(sg_twetalk_handle);
        return ;
    }

    do {
        ota_downloader_process();
        rc = IOT_MQTT_Yield(client, 200);
        if (rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT) {
            HAL_SleepMs(1000);
            continue;
        } else if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
            Log_e("exit with error: %d", rc);
            break;
        }

        if (sg_twetalk_ws_exit) {
            sg_twetalk_ws_exit = 0;
            tc_twetalk_ws_disconnect(sg_twetalk_handle);
            // TODO 正常情况下，ws被断开后，这里应该是唤醒后再重连，这里为了方便测试语种切换，直接重连
            HAL_SleepMs(1000);
            tc_twetalk_ws_reconnect(sg_twetalk_handle, TWETALK_LANGUAGE_TYPE_EN);
        }

    } while (!sg_main_exit);
    Log_i("main exit");
    rc |= usr_data_template_deinit(client);
    ota_downloader_deinit();
    IOT_OTA_Deinit(client);
    rc |= tc_twetalk_ws_exit(sg_twetalk_handle);
    rc |= IOT_MQTT_Destroy(&client);
    utils_log_deinit();
    return ;
}

static int sg_twetalk_init_flag = 0;

int start_tc_iot_twetalk(void)
{
    if (sg_twetalk_init_flag) {
        Log_w("twetalk allready init.");
        return 0;
    }
    thread_fork("tc_twetalk", 25, 8192, 64, 0, twetalk_thread_entry, NULL);
    sg_twetalk_init_flag = 1;
    return 0;
}

int stop_tc_iot_twetalk(void)
{
    sg_twetalk_init_flag = 0;
    sg_main_exit = 1;
}


