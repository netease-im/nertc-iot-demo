#include "system/includes.h"
#include "server/audio_server.h"
#include "app_config.h"
#include "os/os_api.h"

#ifdef USE_VIRTUAL_ENC_TEST


struct enc_server_handle {
    void *enc_server;
    FILE *in_file;
    FILE *out_file;
};

static struct enc_server_handle enc_server_handle;
#define __this      (&enc_server_handle)

//关闭virtual编码操作
static enc_server_test_close(void)
{
    puts("--------enc_server_test_close---------");

    union audio_req req = {0};

    if (__this->enc_server) {
        req.enc.cmd = AUDIO_ENC_CLOSE;
        server_request(__this->enc_server, AUDIO_REQ_ENC, &req);
        server_close(__this->enc_server);
        __this->enc_server = NULL;
    }

    if (__this->out_file) {
        fclose(__this->out_file);
        __this->out_file = NULL;
    }
}

//编码服务注册的回调函数
static void enc_server_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        printf("VIRTUAL_AUDIO_SERVER_EVENT_END");
        enc_server_test_close();
        break;
    case AUDIO_SERVER_EVENT_SPEAK_START:
        printf("speak start ! \n");
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:
        printf("speak stop ! \n");
        break;
    default:
        break;
    }
}

//编码虚拟源输入
static u32 enc_server_read_input(u8 *buf, u32 len)
{
    int rlen;
    if (__this->in_file) {
        rlen = fread(buf, len, 1, __this->in_file);
        if (rlen == len) {
            return rlen;
        } else {
            fclose(__this->in_file);
            __this->in_file = NULL;
            return rlen;
        }
    } else {
        return -1;   //read_input返回-1表示输入数据完成，没有数据输入,让编码服务发结束消息出来AUDIO_SERVER_EVENT_END去进行结束编码的操作
    }
}

//编码输出数据
static int enc_server_vfs_fwrite(void *file, void *data, u32 len)
{
    //不要做长时间堵塞操作
    return fwrite(data, len, 1, __this->out_file);
}

static int enc_server_vfs_fclose(void *file)
{
    return 0;
}

static const struct audio_vfs_ops enc_server_vfs_ops = {
    .fwrite = enc_server_vfs_fwrite,
    .fclose = enc_server_vfs_fclose,
};

//线程接收编码服务回调消息队列方式
/*static void server_back()
{
	while(1){
	 int msg[32];
	 os_time_dly(2);
	 os_taskq_pend("taskq",msg,ARRAY_SIZE(msg));
	}
}*/

static int enc_server_thread()
{
    int err = 0;

    //1.等待sd挂载，打开输入文件和创建输出文件
    extern int storage_device_ready(void);
    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        printf("wait sd mount in");
        os_time_dly(2);
    }

    __this->in_file = fopen(CONFIG_ROOT_PATH"4.pcm", "r");
    if (__this->in_file == NULL) {
        printf("open  in_file  fail!!!!");
    }


    __this->out_file = fopen(CONFIG_ROOT_PATH"out_file.mp3", "w+");
    if (__this->out_file == NULL) {
        printf("open  out_file  fail!!!!");
    }

    //2.开启编码服务和注册编码服务回调
    if (!__this->enc_server) {
        __this->enc_server = server_open("audio_server", "enc");
        if (!__this->enc_server) {
            return -1;
        }
        //a.将编码服务回调函数注册到“app_core”消息队列  常用方式
        server_register_event_handler_to_task(__this->enc_server, NULL, enc_server_server_event_handler, "app_core");
        //b.将编码服务回调函数注册到创建的线程中接收消息队列
        /* thread_fork("server_back", 10, 1024, 32, NULL, server_back, NULL); */
        /* server_register_event_handler_to_task(__this->enc_server, NULL, enc_server_server_event_handler,"server_back"); */
    } else {
        return -1;
    }


    //3.设置编码参数，请求打开编码器
    union audio_req req = {0};
    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = 2;
    req.enc.volume = 100;
    req.enc.output_buf = NULL;
    req.enc.output_buf_len = 3200 * 8;
    req.enc.sample_rate = 16000;
    req.enc.format = "mp3";
    req.enc.frame_size = 3200;   //frame_size为read_input的len
    req.enc.sample_source = "virtual";
    req.enc.read_input = enc_server_read_input;
    req.enc.vfs_ops = &enc_server_vfs_ops;
    req.enc.vir_data_wait = 1;  //等待数据编码完成才进行下一帧数据编码，数据完整
    /* req.enc.no_header = 1; */
    /* req.enc.bitrate = 16000;  */

    err = server_request(__this->enc_server, AUDIO_REQ_ENC, &req);

    return 0;
}

static void main()
{
    puts("----------virtual_enc_test--------");
    thread_fork("enc_server_thread", 12, 1024, 0, 0, enc_server_thread, 0);
}
late_initcall(main);

#endif
