#include "app_config.h"
#include "system/includes.h"
#include "MIDI_CTRL_API.h"
#include "fs/fs.h"
#include "server/audio_server.h"
#include "server/server_core.h"

#ifdef CONFIG_MIDI_CTRL_DEC_ENABLE
/*
 *如遇到多按琴键同时按下或重复快速按某琴键出现卡顿的情况
 *处理方法：1、减少同时发声MIDI_KEY_NUM 个数
 *          2、提高外挂flash的 波特率（60M、90M）,调整spi读线宽的为2线模式或4线
 *          3、提高系统时钟
 * */

#define MIDI_KEY_NUM  (10)//(支持多少个key同时发声(1~18)， 越大，需要的时钟越大)

extern const int MIDI_DEC_SR;

static const u16 midi_samplerate_tab[9] = {
    48000,
    44100,
    32000,
    24000,
    22050,
    16000,
    12000,
    11025,
    8000
};

struct _midi_obj {
    u8 channel;
    u32 sample_rate;
    u32 id;				// 唯一标识符，随机值
    u32 start : 1;		// 正在解码
    char *path;         //音色文件路径
};


//midi文件播放时，对应的音色文件路径(用户可修改路径)
#ifndef CONFIG_MIDI_DEC_ADDR
//音色文件支持在外部存储卡或者外挂flash,sdk默认使用本方式
#define MIDI_FILE_PATH  "storage/sd0/C/MIDI.mdb"
/* #define MIDI_FILE_PATH  "mnt/sdfile/res/cfg/MIDI.mdb" */
#else
#define MIDI_FILE_PATH  CONFIG_MUSIC_PATH_FLASH"MIDI.mdb"
#endif

void midi_ctrl_set_ioctl(u32 cmd, void *priv);

int midi_get_cfg_addr(void **addr)
{
#ifndef CONFIG_MIDI_DEC_ADDR
    //音色文件支持在外部存储卡或者外挂flash,sdk默认使用本方式
    //获取音色文件
    FILE  *file = fopen(MIDI_FILE_PATH, "r");
    if (!file) {
        printf("MIDI.bin open err\n");
        return -1;
    }
    *addr = (void *)file;
    printf("midi_file %p\n", file);
#else
    //音色文件仅支持在内置flash
    FILE  *file = fopen(MIDI_FILE_PATH, "r");
    if (!file) {
        printf("MIDI.bin open err\n");
        return -1;
    }

    struct vfs_attr attr = {0};
    fget_attrs(file, &attr);
    *addr = (void *)attr.sclust;
    fclose(file);
#endif

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    midi音色文件读
   @param
   @return
   @note     内部调用
*/
/*----------------------------------------------------------------------------*/
int midi_fread_api(void *file, void *buf, u32 len)
{
#ifndef CONFIG_MIDI_DEC_ADDR
    FILE *hd = (FILE *)file;
    if (hd) {
        len = fread(buf, len, 1, hd);

        /* printf("MT:%d\n",len); */
        /* put_buf(buf,len); */
    }
#endif
    return len;
}
/*----------------------------------------------------------------------------*/
/**@brief    midi音色文件seek
   @param
   @return
   @note     内部调用
*/
/*----------------------------------------------------------------------------*/
int midi_fseek(void *file, u32 offset, int seek_mode)
{
#ifndef CONFIG_MIDI_DEC_ADDR
    FILE *hd = (FILE *)file;
    if (hd) {
        fseek(hd, offset, seek_mode);
    }
#endif
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    midi ctrl初始化函数，该函数由库调用
   @param
   @return   0
   @note
*/
/*----------------------------------------------------------------------------*/
int midi_ctrl_init(void *info)
{
    void *cache_addr;
    if (midi_get_cfg_addr(&cache_addr)) {
        return -1;
    }
    midi_ctrl_open_parm *parm = (midi_ctrl_open_parm *)info;
    parm->ctrl_parm.tempo = 1024;//解码会改变播放速度,1024是正常值
    parm->ctrl_parm.track_num = 1;//支持音轨的最大个数0~15

    parm->sample_rate = MIDI_DEC_SR;//midi_samplerate_tab[5]; 设置采样率
    parm->cfg_parm.player_t = MIDI_KEY_NUM; //(支持多少个key同时发声,用户可修改)
    parm->cfg_parm.spi_pos = (unsigned int)cache_addr;
    parm->cfg_parm.fread = midi_fread_api;
    parm->cfg_parm.fseek = midi_fseek;
    parm->cfg_parm.bitwidth = 16;  //输出pcm数据位宽  16 或者24
    parm->cfg_parm.out_channel = 2;        //输出通道
    parm->cfg_parm.OutdataBit = 0;//输出数据位宽  0->16bit  1->32bit

    printf("midi_ctrl_init :%d,%x\n", __LINE__, parm->cfg_parm.spi_pos); //算法要求spi_pos要求两字节对齐
    ASSERT(((u32)parm->cfg_parm.spi_pos & 0x01) == 0); //算法要求spi_pos要求两字节对齐
    for (int i = 0; i < ARRAY_SIZE(midi_samplerate_tab); i++) {
        if (parm->sample_rate == midi_samplerate_tab[i]) {
            parm->cfg_parm.sample_rate = i;
            break;
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    midi ctrl音色文件关闭 该函数由库调用
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int midi_ctrl_uninit(void *priv)
{
#ifndef CONFIG_MIDI_DEC_ADDR
    if (priv) {
        fclose((FILE *)priv);
        priv = NULL;
    }
#endif
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    midi ctrl控制函数
	@param   cmd:
			 MIDI_CTRL_NOTE_ON,     //按键按下，参数结构对应struct note_on_parm
			 MIDI_CTRL_NOTE_OFF,    //按键松开，参数结构对应struct note_off_parm
			 MIDI_CTRL_SET_PROG,    //更改乐器，参数结构对应struct set_prog_parm
			 MIDI_CTRL_PITCH_BEND,  //弯音轮，参数结构对应struct pitch_bend_parm
		     CMD_MIDI_CTRL_TEMPO,    //改变节奏,参数结构对应 MIDI_PLAY_CTRL_TEMPO
			 MIDI_CTRL_VEL_VIBRATE, //抖动幅度,参数结构对应 struct vel_vibrate_parm
			 MIDI_CTRL_QUERY_PLAY_KEY,//查询指定通道的key播放,参数结构对应struct query_play_key_parm
   @param    priv:对应cmd的参数地址
   @return   0
   @note    midi解码控制例程
*/
/*----------------------------------------------------------------------------*/
void midi_ctrl_ioctrl(u32 cmd, void *priv)
{
    //需在server_open打开midi解码的audio_server定义解码server的ioctl命令控制
    midi_ctrl_set_ioctl(cmd, priv);
}

/*----------------------------------------------------------------------------*/
/**@brief   乐器更新
   @param   prog:乐器号
   @param   trk_num :音轨 (0~15)
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void midi_ctrl_set_porg(u8 prog, u8 trk_num)
{
    struct set_prog_parm parm = {0};
    parm.prog = prog;
    parm.trk_num = trk_num;
    midi_ctrl_ioctrl(MIDI_CTRL_SET_PROG, &parm);
}

/*----------------------------------------------------------------------------*/
/**@brief   按键按下
   @param   nkey:按键序号（0~127）
   @param   nvel:按键力度（0~127）
   @param   chn :通道(0~15)
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void midi_ctrl_note_on(u8 nkey, u8 nvel, u8 chn)
{
    struct note_on_parm parm = {0};
    parm.nkey = nkey;
    parm.nvel = nvel;
    parm.chn = chn;
    midi_ctrl_ioctrl(MIDI_CTRL_NOTE_ON, &parm);
}

/*----------------------------------------------------------------------------*/
/**@brief   按键松开
   @param   nkey:按键序号（0~127）
   @param   chn :通道(0~15)
   @param   time ：time为衰减时间ms，若为0则使用音色中的衰减
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void midi_ctrl_note_off(u8 nkey, u8 chn, u16 time)
{
    struct note_off_parm parm = {0};
    parm.nkey = nkey;
    parm.chn = chn;
    parm.time = time;
    midi_ctrl_ioctrl(MIDI_CTRL_NOTE_OFF, &parm);
}

/*----------------------------------------------------------------------------*/
/**@brief  midi 配置接口
   @param   cmd:命令
   @param   priv:对应cmd的结构体
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void midi_ctrl_confing(u32 cmd, void *priv)
{
    midi_ctrl_ioctrl(cmd, priv);
}

/*----------------------------------------------------------------------------*/
/**@brief   midi keyboard 设置按键按下音符发声的衰减系数
   @param   val:((衰减系数&0x7fff) | (延时系数&0x1f <<11))
   @return
   @note    低11bit为调节衰减系数，值越小，衰减越快， 1024为默认值， 范围：0~1024
            大于11bit为延时系数，节尾音长度的，值越大拉德越长，0为默认值,31延长1s,范围：0~31(延时系数无效)
   @example midi_ctrl_confing_set_melody_decay((衰减系数&0x7fff) | (延时系数&0x1f <<11));
			u16 val = ((1024&0x7ff) | (31&0x1f << 11));
			midi_ctrl_confing_set_melody_decay(val);
*/
/*----------------------------------------------------------------------------*/
void midi_ctrl_confing_set_melody_decay(u16 val)
{
    u32 cmd = CMD_MIDI_CTRL_TEMPO;
    MIDI_PLAY_CTRL_TEMPO tempo = {0};
    for (int i = 0; i < 16; i++) {
        tempo.decay_val[i] = val;
    }
    tempo.tempo_val = 1024;//设置为固定1024即可
    midi_ctrl_confing(cmd, (void *)&tempo);
}

/*----------------------------------------------------------------------------*/
/**@brief   midi keyboard 设置每个通道按下音符发声的衰减系数
   @param   val:16个值的数组，每个值组成组成结构((衰减系数&0x7fff) | (延时系数&0x1f <<11))
   @return
   @note    低11bit为调节衰减系数，值越小，衰减越快， 1024为默认值， 范围：0~1024
            大于11bit为延时系数，节尾音长度的，值越大拉德越长，0为默认值,31延长1s,范围：0~31(延时系数无效)
   @example u16 val[16];
   			for (int i = 0; i< 16; i++){
			    val[i] = ((1024&0x7ff) | (31&0x1f << 11));
			}
   			midi_ctrl_confing_set_melody_decay(val);
*/
/*----------------------------------------------------------------------------*/
void midi_ctrl_confing_set_melody_decay_each_chn(u16 *val)
{
    u32 cmd = CMD_MIDI_CTRL_TEMPO;
    MIDI_PLAY_CTRL_TEMPO tempo = {0};
    if (val) {
        memcpy(tempo.decay_val, val, sizeof(tempo.decay_val));
    }
    tempo.tempo_val = 1024;//设置为固定1024即可
    midi_ctrl_confing(cmd, (void *)&tempo);
}

/*----------------------------------------------------------------------------*/
/**@brief  弯音轮配置
   @param   pitch_val:弯音轮值,1 - 65535 ；256是正常值,对音高有作用
   @param   chn :通道(0~15)
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void midi_ctrl_pitch_bend(u16 pitch_val, u8 chn)
{
    struct pitch_bend_parm parm = {0};
    parm.pitch_val = pitch_val;
    parm.chn = chn;
    midi_ctrl_ioctrl(MIDI_CTRL_PITCH_BEND, &parm);
}


struct midi_dec_server_handle {
    void *midi_dec_server;
};

static struct midi_dec_server_handle midi_dec_server_handle;
#define __this      (&midi_dec_server_handle)

static void dec_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        printf("AUDIO_SERVER_EVENT_END");
        break;
    default:
        break;
    }
}

void midi_ctrl_set_ioctl(u32 cmd, void *priv)
{
    if (__this->midi_dec_server) {
        union audio_req req = {0};
        req.ioctl.cmd = cmd;
        req.ioctl.priv = priv;
        server_request(__this->midi_dec_server, AUDIO_REQ_IOCTL, &req);
    }
}


int midi_ctrl_dec_open()
{
    int err = 0;
    union audio_req req = {0};

    printf("----------dec_server_thread----------\n");

    __this->midi_dec_server = server_open("audio_server", "dec");
    if (!__this->midi_dec_server) {
        printf("midi_dec_server open fail\n");
        goto __err;
    }

    server_register_event_handler_to_task(__this->midi_dec_server, NULL, dec_server_event_handler, "app_core");

    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = 60;
    req.dec.output_buf_len  = 6 * 1024;
    req.dec.channel         = 0; //没有输入数据无需设置
    req.dec.sample_rate     = 0; //解码器读取midi_ctrl_init中设置采样率，此处无需设置
    req.dec.dec_type        = "midi";
    req.dec.sample_source   = "dac";
    req.dec.file            = NULL;

    err = server_request(__this->midi_dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        goto __err;
    }

    req.dec.cmd = AUDIO_DEC_START;
    server_request(__this->midi_dec_server, AUDIO_REQ_DEC, &req);

    return 0;

__err:
    if (__this->midi_dec_server) {
        server_close(__this->midi_dec_server);
        __this->midi_dec_server = NULL;
    }

    return -1;
}

void midi_ctrl_dec_close()
{
    if (__this->midi_dec_server) {
        printf("midi_ctrl_dec_close\n");

        union audio_req req = {0};
        req.dec.cmd = AUDIO_DEC_STOP;
        server_request(__this->midi_dec_server, AUDIO_REQ_DEC, &req);

        server_close(__this->midi_dec_server);
        __this->midi_dec_server = NULL;
    }
}



#define MIDI_DEC_TEST 0
#if MIDI_DEC_TEST

#define MIDI_SWITCH_OPEN_CLOSE 0
#define MIDI_SET_KEY 1
#define MIDI_PLAY_KEY 2
/*----------------------------------------------------------------------------*/
/**@brief   midi key 测试样例
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void midi_paly_test(u32 key)
{
    static u8 open_close = 0;
    static u8 change_prog = 0;
    static u8 note_on_off = 0;
    static u8 midi_dec_open_err = 0;

    if (midi_dec_open_err) {
        printf("midi_dec_open_err");
        return;
    }

    switch (key) {
    case MIDI_SWITCH_OPEN_CLOSE:
        if (!open_close) {
            midi_dec_open_err = midi_ctrl_dec_open(); //打开midi琴解码
        } else {
            midi_ctrl_dec_close();//关闭midi琴解码
        }
        open_close = !open_close;
        break;
    case MIDI_SET_KEY:
        if (!change_prog) {
            midi_ctrl_set_porg(0, 0); //设置0号乐器，音轨0
        } else {
            midi_ctrl_set_porg(1, 0); //设置1号乐器，音轨0
        }

        change_prog = !change_prog;
        break;
    case MIDI_PLAY_KEY:
        //模拟按键57、58、59、60、61、62,以力度127，通道0，按下/松开测试
        midi_ctrl_note_on(57, 127, 0);
        os_time_dly(10);
        midi_ctrl_note_off(57,  0, 0);
        os_time_dly(100);

        midi_ctrl_note_on(58, 127, 0);
        os_time_dly(1);
        midi_ctrl_note_off(58,  0, 0);
        os_time_dly(100);

        midi_ctrl_note_on(59, 127, 0);
        os_time_dly(1);
        midi_ctrl_note_off(59,  0, 0);
        os_time_dly(100);

        midi_ctrl_note_on(60, 127, 0);
        os_time_dly(1);
        midi_ctrl_note_off(60,  0, 0);
        os_time_dly(100);

        midi_ctrl_note_on(61, 127, 0);
        os_time_dly(1);
        midi_ctrl_note_off(61,  0, 0);
        os_time_dly(100);

        midi_ctrl_note_on(62, 127, 0);
        os_time_dly(1);
        midi_ctrl_note_off(62,  0, 0);
        os_time_dly(100);
        break;
    default:
        break;
    }
}


void midi_test_demo(void)
{
    midi_paly_test(MIDI_SWITCH_OPEN_CLOSE);
    midi_paly_test(MIDI_SET_KEY);
    midi_paly_test(MIDI_PLAY_KEY);
    midi_paly_test(MIDI_SWITCH_OPEN_CLOSE);
}

static void test_main()
{
    if (!strncmp(MIDI_FILE_PATH, "storage/sd", 10)) {
        extern int storage_device_ready(void);
        while (!storage_device_ready()) {//等待sd文件系统挂载完成
            printf("midi test wait sd ready");
            os_time_dly(2);
        }
    }

    thread_fork("midi_test_main", 12, 1024, 0, 0, midi_test_demo, 0);
}
late_initcall(test_main);
#endif //MIDI_DEC_TEST

#endif
