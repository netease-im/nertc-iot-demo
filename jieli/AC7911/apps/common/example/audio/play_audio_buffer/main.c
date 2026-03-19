#include "system/includes.h"
#include "server/audio_server.h"
#include "app_config.h"

#ifdef USE_AUDIO_BUFFER_TEST

enum {
    AUDIO_SUCC 	= 0,
    AUDIO_EFOPEN,
    AUDIO_EREQ_OPEN,
    AUDIO_EREQ_START,
    AUDIO_EMEM,
    AUDIO_ESER_OPEN,
};

struct audio_buffer_hdl {
    char volume;
    int play_time;
    FILE *file;
    u8 read_file_end_flag;
    int audio_data_offset;
    struct server *dec_server;
    volatile u8 run_flag;
    OS_SEM r_sem;
    OS_SEM w_sem;
    u8 *cache_buf;
    cbuffer_t save_cbuf;
    int sample_rate;
    int channel;

};

static struct audio_buffer_hdl audio_buffer_handler;

#define __this 	(&audio_buffer_handler)

#define FIRST_INFO_SECTOR	512
#define READ_DATA_SIZE			512

static int audio_buffer_dec_stop(void)
{
    int err = 0;
    union audio_req req = {0};

    if (!__this->file) {
        return 0;
    }

    req.dec.cmd = AUDIO_DEC_STOP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    int argv[2];
    argv[0] = AUDIO_SERVER_EVENT_END;
    argv[1] = (int)__this->file;
    server_event_handler_del(__this->dec_server, 2, argv);

    fclose(__this->file);
    __this->file = NULL;

    return 0;
}
extern void audio_uninit();
static void dec_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
        log_i("audio_buffer: AUDIO_SERVER_EVENT_ERR\n");
    case AUDIO_SERVER_EVENT_END:
        log_i("audio_buffer: AUDIO_SERVER_EVENT_END\n");
        audio_buffer_dec_stop();
        audio_uninit();
        break;
    case AUDIO_SERVER_EVENT_CURR_TIME:
        log_d("play_time: %d\n", argv[1]);
        __this->play_time = argv[1];
        break;
    }
}
extern u8 audio_get_read_file_is_end();
extern void audio_pend_read_semaphore();
extern void audio_post_write_semaphore();
static int audio_vfs_fread(void *file, void *data, u32 len)
{
    cbuffer_t *cbuf = (cbuffer_t *)file;
    u32 rlen;

    do {
        rlen = cbuf_get_data_size(cbuf);
        if (rlen < len) {
            if (audio_get_read_file_is_end()) {
                return cbuf_read(cbuf, data, rlen);
            }
            audio_post_write_semaphore();
            audio_pend_read_semaphore();
            continue;
        }
        if (cbuf_read(cbuf, data, len) > 0) {
            return len;
        }
    } while (__this->run_flag);

    return 0;
}

static int audio_vfs_fclose(void *file)
{
    return 0;
}

static int audio_vfs_flen(void *file)
{
    return flen(__this->file);
}

static int audio_vfs_fseek(void *file, u32 offset, int orig)
{
    __this->audio_data_offset = offset;
    return 0;
}

static const struct audio_vfs_ops audio_vfs_ops = {
    .fread  = audio_vfs_fread,
    .fclose = audio_vfs_fclose,
    .flen   = audio_vfs_flen,
    .fseek	= audio_vfs_fseek,
};

static void init_audio_buffer_hdl()
{
    memset(__this, 0, sizeof(__this));
}

static u8 audio_is_info_read(int read_offset)
{
    return read_offset ? 1 : 0;
}
static void audio_file_data_offset()
{
    fseek(__this->file, __this->audio_data_offset, SEEK_SET);
}

static int audio_check_cbuf_is_write_able(int write_len)
{
    return cbuf_is_write_able(&__this->save_cbuf, write_len);
}

static void audio_semaphore_del()
{
    os_sem_del(&__this->r_sem, 0);
    os_sem_del(&__this->w_sem, 0);
}

static void audio_semaphore_create()
{
    os_sem_create(&__this->r_sem, 0);
    os_sem_create(&__this->w_sem, 0);
}

static void audio_post_write_semaphore()
{
    os_sem_set(&__this->w_sem, 0);
    os_sem_post(&__this->w_sem);
}

static void audio_pend_write_semaphore()
{
    os_sem_pend(&__this->w_sem, 0);
}

static void audio_post_read_semaphore()
{
    os_sem_set(&__this->r_sem, 0);
    os_sem_post(&__this->r_sem);
}

static void audio_pend_read_semaphore()
{
    os_sem_pend(&__this->r_sem, 0);
}

static void audio_set_read_file_is_end()
{
    __this->read_file_end_flag = 1;
}
static u8 audio_get_read_file_is_end()
{
    return __this->read_file_end_flag;
}

static int prewriting_audio_data()
{
    u8 buf[READ_DATA_SIZE];
    int rlen = 0;
    rlen = fread(buf, 1, READ_DATA_SIZE, __this->file);
    ASSERT(rlen == READ_DATA_SIZE);
    rlen = cbuf_write(&__this->save_cbuf, buf, rlen);

    return rlen;
}

static int file_data_write_cbuf()
{
    u8 buf[READ_DATA_SIZE];
    int rlen;
    rlen = fread(buf, 1, READ_DATA_SIZE, __this->file);
    rlen = cbuf_write(&__this->save_cbuf, buf, rlen);
    return rlen;
}

static u8 is_audio_file_data_end(int rlen)
{
    return rlen != READ_DATA_SIZE ? 1 : 0;
}

static void read_file_to_cbuf(void *priv)
{
    int rlen;

    prewriting_audio_data();
    audio_pend_write_semaphore();
    audio_file_data_offset();

    while (__this->run_flag) {
        if (!audio_check_cbuf_is_write_able(READ_DATA_SIZE)) {
            audio_pend_write_semaphore();
            continue;
        }
        rlen = file_data_write_cbuf();
        audio_post_read_semaphore();
        if (is_audio_file_data_end(rlen)) {
            audio_set_read_file_is_end();
            break;
        }
    }
}

static void audio_free_init_cbuf()
{
    if (__this->cache_buf) {
        free(__this->cache_buf);
        __this->cache_buf = NULL;
    }
}

static void close_file_from_sdcard()
{
    if (__this->file) {
        close(__this->file);
        __this->file = NULL;
    }
}

static void dec_server_uninit()
{
    server_close(__this->dec_server);
    __this->dec_server = NULL;
}

static int audio_malloc_init_cbuf()
{

    __this->cache_buf = malloc(__this->sample_rate * __this->channel * 10);
    if (!__this->cache_buf) {
        return AUDIO_EMEM;
    }
    cbuf_init(&__this->save_cbuf, __this->cache_buf, __this->sample_rate * __this->channel * 10);

    return AUDIO_SUCC;
}

static void set_audio_parameter()
{
    __this->volume = 100;
    __this->sample_rate = 44100;
    __this->channel = 2;

    __this->run_flag = 1;
}
static int open_file_from_sdcard()
{
    __this->file = fopen(CONFIG_ROOT_PATH"1.mp3", "r");
    if (!__this->file) {
        return AUDIO_EFOPEN;
    }
    return AUDIO_SUCC;
}

static void audio_uninit()
{
    audio_free_init_cbuf();
    close_file_from_sdcard();
    dec_server_uninit();
    audio_semaphore_del();
    __this->run_flag = 0;
}

static void audio_init()
{
    int ret = AUDIO_SUCC;

    init_audio_buffer_hdl();

    set_audio_parameter();
    audio_semaphore_create();
    ASSERT(ret = dec_server_init_and_register() == AUDIO_SUCC, "ret = %d\n", ret);
    ASSERT(ret = open_file_from_sdcard() == AUDIO_SUCC, "ret = %d\n", ret);
    ASSERT(ret = audio_malloc_init_cbuf() == AUDIO_SUCC, "ret = %d\n", ret);
}

static int audio_buffer_data_in()
{
    thread_fork("read_file_to_cbuf", 20, READ_DATA_SIZE * 2, 20, 0, read_file_to_cbuf, NULL);
    return AUDIO_SUCC;
}

static int audio_buffer_data_out()
{
    union audio_req req = {0};

    req.dec.volume			= __this->volume;
    req.dec.output_buf_len	= 4 * 1024;
    req.dec.sample_rate		= __this->sample_rate;
    req.dec.channel 		= __this->channel;
    req.dec.vfs_ops			= &audio_vfs_ops;
    req.dec.dec_type 		= "mp3";
    req.dec.sample_source	= "dac";
    req.dec.file			= (FILE *)&__this->save_cbuf;

    req.dec.cmd				= AUDIO_DEC_OPEN;
    int err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        if (__this->cache_buf) {
            free(__this->cache_buf);
            __this->cache_buf = NULL;
        }
        __this->run_flag = 0;
        return AUDIO_EREQ_OPEN;
    }

    req.dec.cmd = AUDIO_DEC_START;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        if (__this->cache_buf) {
            free(__this->cache_buf);
            __this->cache_buf = NULL;
        }
        __this->run_flag = 0;
        return AUDIO_EREQ_START;
    }


    return AUDIO_SUCC;
}


static int dec_server_init_and_register()
{
    __this->dec_server = server_open("audio_server", "dec");
    if (!__this->dec_server) {
        return AUDIO_ESER_OPEN;
    }
    server_register_event_handler_to_task(__this->dec_server, NULL, dec_server_event_handler, "app_core");
    return AUDIO_SUCC;
}

static int audio_buffer_play()
{
    int ret = AUDIO_SUCC;
    audio_init();
    audio_buffer_data_in();
    ASSERT(ret == audio_buffer_data_out(), "ret = %d\n", ret);

    return 0;
}


static void audio_buffer_thread()
{
    while (!storage_device_ready()) {
        os_time_dly(20);
    }
    audio_buffer_play();
}

static void main()
{
    thread_fork("audio_buffer", 20, 1024, 0, 0, audio_buffer_thread, 0);
}
late_initcall(main);

#endif
