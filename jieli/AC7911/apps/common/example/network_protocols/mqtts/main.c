/***********************************MQTTS 测试说明*******************************************
 *说明：
 * 	 通过MQTTS协议连接亚马逊, 向亚马逊订阅主题和发布消息
 *********************************************************************************************/
#include "system/includes.h"
#include "http/http_cli.h"
#include "mqtt/MQTTClient.h"
#include "app_config.h"
#include "json_c/json_object.h"
#include "json_c/json.h"
#include "fs/fs.h"
#include "json_c/json_tokener.h"

#ifdef USE_MQTTS_TEST

#if 1
#define	log_info(x, ...)	printf("\n------------[aws_test]###" x "----------------\r\n",##__VA_ARGS__)
#else
#define	log_info(...)
#endif

const char ca_pem[] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"
    "-----END CERTIFICATE-----\r\n";

const char cli_crt[] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDWjCCAkKgAwIBAgIVAIe0p5sPF7RDRTjSn0Mi0hSyWGZbMA0GCSqGSIb3DQEB\r\n"
    "CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\r\n"
    "IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMzAyMDgwNjE2\r\n"
    "MTJaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\r\n"
    "dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDB4U5JbPBBeRPT0HRC\r\n"
    "on/JJ4C/9TQUdYPWx5SwQMbO+a+fqDL9/dVhOduzgO9PQw0sBJyR+qrEeN6fFnX1\r\n"
    "K6Z4xKZVeMEXELXUBi4h3AyBO7fApVCRd57t0a+GvF06TonpdujRRSfiknyw8PgK\r\n"
    "gWWEU2iaUCJTz+eNxDVbqMtCKs/9RT1aNZlBubyMbxvw+pLtZGXp2xxytavlEitd\r\n"
    "MA/x6Hx7US5XUNqNaekCUKpD+DboU5sG3NW380AxYfgVtFCtKG07rzmSd5rxBwLO\r\n"
    "F4qPbWBGEScejXFLPRKWxPV8vk/vSaPS4zW8bC5hnZwT8tO1yRS0gmhB1No950TL\r\n"
    "x3b9AgMBAAGjYDBeMB8GA1UdIwQYMBaAFFSCAkS3vvMJDkbND50yyda5CuLjMB0G\r\n"
    "A1UdDgQWBBRn/KddKJD4U52Fc7cyTnbKxZ12NDAMBgNVHRMBAf8EAjAAMA4GA1Ud\r\n"
    "DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAfM4onZ8j92AsKrcUcuUX4ZkF\r\n"
    "w9angmrtzckuOAm+spDbPU7jz4Krnx6AUz48tbAM8ttCdZOPixPOADdrzz3k3nql\r\n"
    "XdG7hIBizuK7AkKuxRcgRKIS9nLSL1Uk0nGGLedZedO5VpFsv3C5yYl8eoaICww2\r\n"
    "KgtQ9gBkMiFu1bM5xCkl1zqxunrxPBgOBM2F+tvriJZBNVdDcrWHd9ngJS+m0wve\r\n"
    "G5zD1DwW0rKPaXm2rE6RZZ5j2qq1HQPqT3h2LMjkp+IKND1phF6IUCYEy4gF+ZQ/\r\n"
    "QHOheyCcQtu+7VY4vHrKBU3watqkx80j85sI0UN9nu+l/1IY1yl4FjuPxmZkVg==\r\n"
    "-----END CERTIFICATE-----\r\n";


const char pri_key[] =
    "-----BEGIN RSA PRIVATE KEY-----\r\n"
    "MIIEpQIBAAKCAQEAweFOSWzwQXkT09B0QqJ/ySeAv/U0FHWD1seUsEDGzvmvn6gy\r\n"
    "/f3VYTnbs4DvT0MNLASckfqqxHjenxZ19SumeMSmVXjBFxC11AYuIdwMgTu3wKVQ\r\n"
    "kXee7dGvhrxdOk6J6Xbo0UUn4pJ8sPD4CoFlhFNomlAiU8/njcQ1W6jLQirP/UU9\r\n"
    "WjWZQbm8jG8b8PqS7WRl6dsccrWr5RIrXTAP8eh8e1EuV1DajWnpAlCqQ/g26FOb\r\n"
    "BtzVt/NAMWH4FbRQrShtO685knea8QcCzheKj21gRhEnHo1xSz0SlsT1fL5P70mj\r\n"
    "0uM1vGwuYZ2cE/LTtckUtIJoQdTaPedEy8d2/QIDAQABAoIBADdo1uDxO3Da06nT\r\n"
    "h4aLvo2Utp0lzO6T70vw2jPNL4Rzo5ezkG86nkMdMAcLMFmsxhTwdHbb9vYTvTwm\r\n"
    "RSv+f6NAQHFpA3Y+Flk0MvcGVqSQlqF+HZqVX1yEcu/E4bgKPiv1CfDVHOMKDozb\r\n"
    "MWh/Nq4s0FZZ1JWzc2/2xXEeuoh8B/iTbRb6q1GKecA0vSuetchRBnnRGaNPgVel\r\n"
    "+CpUBc/ANq7afKOfa6xzokXZ4Lx+mMXNF53psSS1Vtp1dk2JfyTVHzN0MIVlOURO\r\n"
    "FYXzy/ccHgphPEESphRg6jxN1/wf4iI1Ilqp1a1786gYzsHfBvvIcAuq06l54e9N\r\n"
    "sVa+bVkCgYEA9Cp04lSuRt+daLPhVd/pKAazC8vZLALl00VdyiK9NZMD7xLFe9Ys\r\n"
    "z7q+WT2wxMCKYI+zsexHrZAIHI1GfrFou5Cy5Yqo4WctGuG7ee7PGMmStaNVxdyL\r\n"
    "gkyZlmtyxBrMReCgy9XNkv6ihC0UwELl3XLBlM+1AY+cUUsuI4/jZj8CgYEAy0bq\r\n"
    "2Tm+SkL6W9iTDtto0YF8CU0O7mGG58CLWezy8l/nY63odGVd5P3sxmRfgzxrcRa/\r\n"
    "RdAvGQijxXjNlYG2GdJ1F5tUFnyeW4SoGJp87ipy6U9XGnsuh6a2h5s0nJEaADAS\r\n"
    "7SY46+29Zx9XzneqWZbSTnEa70oF3MVZzQ+cK8MCgYEA4UurvovUo6ECZfJVIiAR\r\n"
    "y9UisOfP5ERoPKHtAO89F4ooPxlbEAF4J9Vwfnfl/CaNiLwQW0Rif0w/r9jMC2s3\r\n"
    "8Jp7+EIRwqqpe/x+OOyaO40DyoaERqZSNFDqB1nt8/WmN3Ql6u/FWLWMz9mpJB6W\r\n"
    "a9fPt7hQlLgmzPxJ/pZuhZECgYEAj7CjlcXBg1DxFDd1TE1APMgg25pC8bKy3+M1\r\n"
    "He3mlfnA4K8v4Ac/Zf0wf9S3x1bQQ/JmZn0QrctzqMUJ3aMBDCO+bV95AyQ39E1D\r\n"
    "XZDmupv6+n0A/1EPxQfNgvHtMLdI/9T7q/MWzX8SIXEAaowgOI/qPVSqS3exySsf\r\n"
    "nVppdG0CgYEA84qGVfhTlCl4FwP+XYkPm3/yj1e+AIPqUfFrxUfzwu+Ugv1Ti3Cp\r\n"
    "+fZBB8M+1LQ9Q3I6MokJu82G4yxsTEGzPoVMjf8EsXC0Zu0a6QFiLOPsiAWadVyD\r\n"
    "O0Ya2hjuKye23rBN4Ryf92ON3Icy+ScAHOaXx762SvPf86VVSnkDFVw=\r\n"
    "-----END RSA PRIVATE KEY-----\r\n";


#define AWS_IOT_ENDPOINT "ayzsyi76x9sg9-ats.iot.us-west-2.amazonaws.com"
/* #define THING_NAME	"test_thing" */
/* #define SHADOW_NAME "123" */

#if 1
#define SHADOW	"name/123/"
#else
#define SHADOW	""
#endif

#define TOPIC1	"$aws/things/test_thing/shadow/"SHADOW"delete/accepted"
#define TOPIC2	"$aws/things/test_thing/shadow/"SHADOW"delete/rejected"
#define TOPIC3	"$aws/things/test_thing/shadow/"SHADOW"get/accepted"
#define TOPIC4	"$aws/things/test_thing/shadow/"SHADOW"get/rejected"
#define TOPIC5	"$aws/things/test_thing/shadow/"SHADOW"update/accepted"
#define TOPIC6	"$aws/things/test_thing/shadow/"SHADOW"update/rejected"
#define TOPIC7	"$aws/things/test_thing/shadow/"SHADOW"update/delta"
#define TOPIC8	"$aws/things/test_thing/shadow/"SHADOW"update/documents"


//两种方式读取证书
/* #define USE_SD_FILE */
#ifdef USE_SD_FILE
//读取TF卡中的证书(或资源文件中的证书)
#define CAS_PEM_PATH 	CONFIG_ROOT_PATH"ca.pem"
#define CLI_PEM_PATH	CONFIG_ROOT_PATH"cli.crt"
#define PKEY_PATH		CONFIG_ROOT_PATH"pri.key"
#else
//读取代码中的数组证书
#define CAS_PEM_PATH 	ca_pem
#define CLI_PEM_PATH	cli_crt
#define PKEY_PATH		pri_key
#endif

#define AWS_MQTT_PORT 	8883
#define AWS_QOS 	QOS0
#define BUF_SIZE 	1024*10

static struct Network network;
static struct Client client;
static unsigned char send_buffer[BUF_SIZE];
static unsigned char recv_buffer[BUF_SIZE];
static bool client_connected;
static OS_MUTEX aws_mutex;

static u8 local_net_ret = 2;


void aws_shadow_get_info(void);
void aws_shadow_update_info(void);



//获取状态函数
static void aws_status_get()
{
    MQTTMessage message = {0};
    message.id 			= 	0;
    message.qos 		= 	AWS_QOS;
    message.retained 	=	false;
    message.dup			=	false;
    char topic[] = "$aws/things/test_thing/shadow/"SHADOW"get";
    if (client.isconnected) {
        MQTTPublish(&client, topic, &message);
    }
}

//更新状态函数
static void aws_status_update(char *buf)
{

    MQTTMessage message = {0};
    message.id 			= 	0;
    message.qos 		= 	AWS_QOS;
    message.retained 	=	false;
    message.dup			=	false;
    message.payload		=	buf;
    message.payloadlen	=	strlen(buf);
    char topic[] = "$aws/things/test_thing/shadow/"SHADOW"update";
    MQTTPublish(&client, topic, &message);

}

static void callback(MessageData *md)
{
    //给字符串一个结束符'\0'
    *((char *)md->message->payload +  md->message->payloadlen)  = 0;
    log_info("\n callback  buf = %s\n", md->message->payload);
}

//e.g.歌曲播放完毕后回调函数
static void *music_end_handler(int arg)		//网络歌曲播放完毕回调函数
{
    printf("callback when song end\r\n");
    return NULL;
}

//订阅的回调函数, 消息接收函数
static void delta_callback(MessageData *md)
{
    os_mutex_pend(&aws_mutex, 0);
    int arg = 0;
    char *url;
    json_object *root_node = NULL, *first_node = NULL, *second_node = NULL, *third_node = NULL;
    json_object *state_node = NULL, *report_node = NULL, *music_node = NULL, *light_node = NULL, *voice_node = NULL, *timer_node = NULL, *alarm_node = NULL;
    *((char *)md->message->payload +  md->message->payloadlen)  = 0;
    log_info("\n\n\n delta = %s\n\n\n", md->message->payload);
    //e.g.do something
#if 0
    root_node = json_tokener_parse(md->message->payload);
    if (!root_node) {
        log_info("%d root_node NULL", __LINE__);
        return ;
    }
    if (json_object_object_get_ex(root_node, "state", &first_node)) {
        //create update string
        report_node = json_object_new_object();

        if (json_object_object_get_ex(first_node, "music", &second_node)) {	//音乐主题
            music_node = json_object_new_object();
            if (json_object_object_get(second_node, "id")) {

                arg = json_object_get_int(json_object_object_get(second_node, "id"));
                //
                if (1) { //success
                    json_object_object_add(music_node, "id", json_object_new_string("id_num"));
                } else {
                    json_object_object_add(music_node, "id", json_object_new_string("yuanlai"));
                }
            }
            if (json_object_object_get(second_node, "url")) {

                //
                url = json_object_get_string(json_object_object_get(second_node, "url"));

                if (1) { //success
                    json_object_object_add(music_node, "url", json_object_new_string("url_test"));
                } else {
                    json_object_object_add(music_node, "url", json_object_new_string("yuanlaui"));
                }
            }
#if 0
            if (json_object_object_get(second_node, "name")) {

                int num = json_object_get_int(json_object_object_get(second_node, "name"));
                printf("num = %d  arg = %d ret = %d url = %s\r\n", num, arg, local_net_ret, url);

                if (num == 1) {
                    printf("into net_local_switch\r\n");
                    local_net_ret = net_local_play_mode_switch(arg);	//(本地播放模式 返回 1)  与(网络播放模式 返回 2) 切换 返回当前播放模式	参数为1时切换播放模式,非1不切换
                    printf("ret = %d\r\n", local_net_ret);
                } else if (num == 2) {
                    if (local_net_ret == 1) {
                        char buf[] = "STAND-ALONE.mp3";
                        printf("into local search ,buf = %s\r\n", buf);
                        local_music_search(buf);				//蓝牙点歌，本地搜索歌曲播放
                    }
                } else if (num == 3) {
                    if (local_net_ret == 1) {
                        printf("into switch music\r\n");
                        local_switch_music(arg);			//本地切换歌曲   0 上一首  1 下一首
                    }
                } else if (num == 4) {
                    printf("into  stop\r\n");
                    set_music_play();
                } else if (num == 5) {
                    printf("into  step\r\n");
                    set_music_step(arg);
                } else if (num == 6) {
                    if (local_net_ret == 2) {
                        printf("into net play music \r\n");
                        /* net_play_music_file("http://music.163.com/song/media/outer/url?id=512359195.mp3",music_end_handler,0);//通过url 获取网络歌曲, music_end_handler是歌曲播放完成后回调函数 */
                        net_play_music_file(url, music_end_handler, 0); //通过url 获取网络歌曲, music_end_handler是歌曲播放完成后回调函数
                        /* net_play_music_file("https://speaker123.s3.us-west-2.amazonaws.com/speaker-admin/batchUpload/speaker-admin/batchUpload/2020-11-19/0d749703-2cd2-46d2-aa19-a35f80b88054.mp3 ",music_end_handler,0);//通过url 获取网络歌曲, music_end_handler是歌曲播放完成后回调函数 */
                    }
                } else if (num == 7) {
                    if (local_net_ret == 1) {
                        printf("into get local list\r\n");
                        local_music_list();		//根据实际情况修改
                    }
                } else if (num == 8) {
                    if (local_net_ret == 1) {
                        printf("into get current name\r\n");
                        printf("current_name = %s\r\n", local_current_name());	//根据实际情况修改
                    }

                } else if (num == 9) {
                    printf("into volume\r\n");
                    set_music_volume(arg);
                } else if (num == 10) {
                    printf("into power_off\r\n");
                    set_power_off();
                }



                //
                if (1) { //success
                    json_object_object_add(music_node, "name", json_object_new_string("music_name"));
                } else {
                    json_object_object_add(music_node, "name", json_object_new_string("myuanlai"));
                }
            }
#endif
            if (json_object_object_get(second_node, "url")) {

                //

                if (1) { //success
                    json_object_object_add(music_node, "url", json_object_new_string("url_test"));
                } else {
                    json_object_object_add(music_node, "url", json_object_new_string("yuanlaui"));
                }
            }
            if (json_object_object_get(second_node, "albumId")) {
                //
                if (1) { //success
                    json_object_object_add(music_node, "albumId", json_object_new_string("albumid_test"));
                } else {
                    json_object_object_add(music_node, "albumId", json_object_new_string("yuanlai"));
                }
            }
            if (json_object_object_get(second_node, "albumName")) {
                //
                if (1) { //success
                    json_object_object_add(music_node, "albumName", json_object_new_string("213414121"));
                } else {
                    json_object_object_add(music_node, "albumName", json_object_new_string("yuan;lai"));
                }
            }
            json_object_object_add(report_node, "music", music_node);
            json_object_put(second_node);
        }

        if (json_object_object_get_ex(first_node, "light", &second_node)) {	//
            light_node = json_object_new_object();
            if (json_object_object_get(second_node, "color")) {
                //
                if (1) {
                    json_object_object_add(light_node, "color", json_object_new_string("red"));
                } else {
                    json_object_object_add(light_node, "color", json_object_new_string("yuanlai"));
                }
            }
            if (json_object_object_get(second_node, "bright")) {

                //
                if (1) {
                    json_object_object_add(light_node, "bright", json_object_new_int(15));
                } else {
                    json_object_object_add(light_node, "bright", json_object_new_int(26));
                }
            }
            json_object_object_add(report_node, "light", light_node);
            json_object_put(second_node);
        }

        if (json_object_object_get_ex(first_node, "voice", &second_node)) {	//
            voice_node = json_object_new_object();
            if (json_object_object_get(second_node, "playVolume")) {
                //
                if (0) {
                    json_object_object_add(voice_node, "playVolume", json_object_new_string("voluime_num"));
                } else {
                    json_object_object_add(voice_node, "playVolume", json_object_new_string("vyuanlaui"));
                }
            }
            if (json_object_object_get(second_node, "playSwitch")) {
                //
                if (1) {
                    json_object_object_add(voice_node, "playSwitch", json_object_new_string("swtich_num"));
                } else {
                    json_object_object_add(voice_node, "playSwitch", json_object_new_string("yuanlaim"));
                }
            }
            json_object_object_add(report_node, "voice", voice_node);
            json_object_put(second_node);
        }

        if (json_object_object_get_ex(first_node, "timer", &second_node)) {	//
            timer_node = json_object_new_object();
            if (json_object_object_get(second_node, "region")) {
                //
                if (1) {
                    json_object_object_add(timer_node, "region", json_object_new_int(12));
                } else {
                    json_object_object_add(timer_node, "region", json_object_new_int(8));
                }
            }
            if (json_object_object_get(second_node, "year")) {
                //
                if (1) {
                    json_object_object_add(timer_node, "year", json_object_new_int(2002));
                } else {
                    json_object_object_add(timer_node, "year", json_object_new_int(2020));
                }
            }
            if (json_object_object_get(second_node, "month")) {
                //
                if (1) {
                    json_object_object_add(timer_node, "month", json_object_new_int(12));
                } else {
                    json_object_object_add(timer_node, "month", json_object_new_int(8));
                }
            }
            if (json_object_object_get(second_node, "day")) {
                //
                if (1) {
                    json_object_object_add(timer_node, "day", json_object_new_int(12));
                } else {
                    json_object_object_add(timer_node, "day", json_object_new_int(8));
                }
            }
            if (json_object_object_get(second_node, "second")) {
                //
                if (1) {
                    json_object_object_add(timer_node, "second", json_object_new_int(12));
                } else {
                    json_object_object_add(timer_node, "second", json_object_new_int(8));
                }
            }
            json_object_object_add(report_node, "timer", timer_node);
            json_object_put(second_node);
        }

        if (json_object_object_get_ex(first_node, "alarm", &second_node)) {	//
            alarm_node = json_object_new_object();
            if (json_object_object_get(second_node, "year")) {
                //
                if (1) {
                    json_object_object_add(alarm_node, "year", json_object_new_int(2002));
                } else {
                    json_object_object_add(alarm_node, "year", json_object_new_int(2020));
                }
            }
            if (json_object_object_get(second_node, "month")) {
                //
                if (1) {
                    json_object_object_add(alarm_node, "month", json_object_new_int(12));
                } else {
                    json_object_object_add(alarm_node, "month", json_object_new_int(8));
                }
            }
            if (json_object_object_get(second_node, "day")) {
                //
                if (1) {
                    json_object_object_add(alarm_node, "day", json_object_new_int(12));
                } else {
                    json_object_object_add(alarm_node, "day", json_object_new_int(8));
                }
            }
            if (json_object_object_get(second_node, "second")) {
                //
                if (1) {
                    json_object_object_add(alarm_node, "second", json_object_new_int(12));
                } else {
                    json_object_object_add(alarm_node, "second", json_object_new_int(8));
                }
            }
            if (json_object_object_get(second_node, "playVolume")) {
                //
                if (1) {
                    json_object_object_add(alarm_node, "playVolume", json_object_new_int(12));
                } else {
                    json_object_object_add(alarm_node, "playVolume", json_object_new_int(8));
                }
            }
            if (json_object_object_get(second_node, "bright")) {
                //
                if (1) {
                    json_object_object_add(alarm_node, "bright", json_object_new_int(12));
                } else {
                    json_object_object_add(alarm_node, "bright", json_object_new_int(8));
                }
            }
            json_object_object_add(report_node, "alarm", alarm_node);
            json_object_put(second_node);
        }

        third_node = json_object_new_object();
        state_node = json_object_new_object();
        //在这里拼接update的字符串
        json_object_object_add(state_node, "reported", report_node);
        json_object_object_add(third_node, "state", state_node);

        log_info("report_buf = %s", json_object_get_string(third_node));
        //上传更新下状态
        aws_status_update(json_object_get_string(third_node));
        log_info("status update success!");
    }

    if (root_node) {
        json_object_put(root_node);
    }
    /* if(third_node){ */
    /* json_object_put(third_node); */
    /* } */
#endif
    os_mutex_post(&aws_mutex);

}

//mqtt连接函数
static int aws_mqtt_connect()
{
    char topic[128] = {0};

    //1.初始化TLS
    NetWorkSetTLS(&network);
    //2.初始化回调函数
    SetNetworkCb(&network, NULL, NULL);

    printf("---%s---%s---%d CA = \n%s PEM = \n%s KEY= \n%s\n\r", __FILE__, __func__, __LINE__, CAS_PEM_PATH, CLI_PEM_PATH, PKEY_PATH);
    //3.添加证书
    NetWorkSetTLS_key(&network, CAS_PEM_PATH, strlen(CAS_PEM_PATH), CLI_PEM_PATH, strlen(CLI_PEM_PATH), PKEY_PATH, strlen(PKEY_PATH));

    //4.连接sock
    if (0 != ConnectNetwork(&network, AWS_IOT_ENDPOINT, AWS_MQTT_PORT)) {
        printf("---%s---%s---%d \n\r", __FILE__, __func__, __LINE__);
        network.disconnect(&network);
        return -1;
    }

    //5.设置连接超时时间
    SetNetworkRecvTimeout(&network, 1000);

    //6.初始化mqtt
    MQTTClient(&client, &network, 1000, send_buffer, BUF_SIZE, recv_buffer, BUF_SIZE);

    //7.设置mqtt参数
    u8 mac_addr[6];
    wifi_get_mac(mac_addr);
    char clientid[20] = {0};
    sprintf(clientid, "%02x%02x%02x%02x%02x%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    printf("---%s---%s---%d client = %s\n\r", __FILE__, __func__, __LINE__, clientid);
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion       = 4;
    data.clientID.cstring  = clientid;
    data.keepAliveInterval = 10;
    data.cleansession      = 1;
    data.willFlag          = 0;

    //8.连接mqtt
    if (0 != MQTTConnect(&client, &data)) {
        printf("---%s---%s---%d \n\r", __FILE__, __func__, __LINE__);
        network.disconnect(&network);
        return -1;
    }

    //9.订阅主题
    //不需要的订阅可以删除
    if (0 != MQTTSubscribe(&client, TOPIC3, AWS_QOS, callback)) {
        printf("---%s---%s---%d \n\r", __FILE__, __func__, __LINE__);
        MQTTDisconnect(&client);
        network.disconnect(&network);
        return -1;
    }
    if (0 != MQTTSubscribe(&client, TOPIC4, AWS_QOS, callback)) {
        printf("---%s---%s---%d \n\r", __FILE__, __func__, __LINE__);
        MQTTDisconnect(&client);
        network.disconnect(&network);
        return -1;
    }
    if (0 != MQTTSubscribe(&client, TOPIC5, AWS_QOS, NULL)) {
        printf("---%s---%s---%d \n\r", __FILE__, __func__, __LINE__);
        MQTTDisconnect(&client);
        network.disconnect(&network);
        return -1;
    }
    if (0 != MQTTSubscribe(&client, TOPIC6, AWS_QOS, callback)) {
        printf("---%s---%s---%d \n\r", __FILE__, __func__, __LINE__);
        MQTTDisconnect(&client);
        network.disconnect(&network);
        return -1;
    }
    if (0 != MQTTSubscribe(&client, TOPIC7, AWS_QOS, delta_callback)) {
        printf("---%s---%s---%d \n\r", __FILE__, __func__, __LINE__);
        MQTTDisconnect(&client);
        network.disconnect(&network);
        return -1;
    }
    if (0 != MQTTSubscribe(&client, TOPIC8, AWS_QOS, NULL)) {
        printf("---%s---%s---%d \n\r", __FILE__, __func__, __LINE__);
        MQTTDisconnect(&client);
        network.disconnect(&network);
        return -1;
    }


    /* aws_shadow_get_info(); */
    /* aws_shadow_update_info(); */
    log_info("success");
    client_connected = TRUE;

    return 0;
}

//创建mqtt线程
void aws_mqtt_task()
{
#ifdef USE_SD_FILE
    while (!storage_device_ready()) {
        os_time_dly(10);
        puts("wait storage_device_ready ");
    }
#endif
    u8 rc;
    u32 delay_cnt = 0, delay = 500;
    os_mutex_create(&aws_mutex);
    while (1) {
        if (client_connected == FALSE) {
            if (delay_cnt) {
                --delay_cnt;
                os_time_dly(50);
                continue;
            } else {
                if (delay < 16 * 1000) {
                    delay <<= 1;
                }
                delay_cnt = delay / 500;
            }
            aws_mqtt_connect();
        } else {
            /* int msg[32]; */
            /* os_taskq_accept(ARRAY_SIZE(msg), msg); */
            rc = MQTTYield(&client, 1000);
            if (rc) {
                MQTTDisconnect(&client);
                network.disconnect(&network);
                client_connected = FALSE;
            }
        }
    }
}



//https,暂时用不到
void aws_shadow_get_info(void)
{
    int ret;
    char url[256] = {0};
    /* char buf[1024] = {0}; */
    char *buf = (char *)calloc(1, 1024);
    http_body_obj http_recv_body = {0};
    httpcli_ctx ctx = {0};
    http_recv_body.recv_len = 0;
    http_recv_body.buf_len = 1024;
    http_recv_body.buf_count = 1;
    strcpy(url, "https://"AWS_IOT_ENDPOINT"/things/test_thing/shadow?name=123");
    http_recv_body.p = buf;

    //添加证书
    HttpsSetTLS_key(&ctx, CAS_PEM_PATH, CLI_PEM_PATH, PKEY_PATH);


    ctx.wait_content_length = 1;
    ctx.timeout_millsec = 5000;
    ctx.priv = &http_recv_body;
    ctx.url = url;
    ctx.connection = "close";
    ret = httpcli_get(&ctx);
    if (ret) {
        printf("%s %d , ret = %d\r\n", __func__, __LINE__, ret);
        return;
    }
    if (http_recv_body.p != NULL) {
        log_info("http_buf = %s\r\n", (unsigned char *)http_recv_body.p);
    }

}

void aws_shadow_update_info(void)
{
    int ret;
    char url[256] = {0}, buf[1024] = {0};
    json_object *root_node = NULL;
    http_body_obj http_recv_body = {0};
    httpcli_ctx ctx = {0};
    http_recv_body.recv_len = 0;
    http_recv_body.buf_len = 1024;
    http_recv_body.buf_count = 1;
    strcpy(url, "https://"AWS_IOT_ENDPOINT"/things/test_thing/shadow?name=123");
    http_recv_body.p = buf;

    //添加证书
    HttpsSetTLS_key(&ctx, CAS_PEM_PATH, CLI_PEM_PATH, PKEY_PATH);


    char post_buf[1024] = "{\"state\":{\"color\":\"22\",\"power\":\"on\"}}";
    ctx.data_format = "application/json";
    ctx.data_len = strlen(post_buf);
    ctx.post_data = post_buf;

    ctx.wait_content_length = 1;
    ctx.timeout_millsec = 5000;
    ctx.priv = &http_recv_body;
    ctx.url = url;
    ctx.connection = "close";
    ret = httpcli_post(&ctx);
    if (ret) {
        printf("%s %d , ret = %d\r\n", __func__, __LINE__, ret);
        return;
    }
    if (http_recv_body.p != NULL) {
        log_info("buf = %s\r\n", (unsigned char *)http_recv_body.p);
    }

}

//应用程序入口,需要运行在STA模式下
static void c_main()
{
    if (thread_fork("aws_mqtt_task", 20, 2048, 512, NULL, aws_mqtt_task, NULL)) {
        printf("thread fork fail\n");
    }
}

late_initcall(c_main);

#endif
