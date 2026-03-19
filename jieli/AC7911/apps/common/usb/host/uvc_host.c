
/*****************************************************************
>file name : uvc_dev.c
>author : lichao
>create time : Tue 13 Jun 2017 04:20:02 PM HKT
*****************************************************************/
#include "device/usb_stack.h"
#include "asm/dma.h"
#include "asm/uvc_device.h"
#include "usb_config.h"
#include "host_uvc.h"
#include "video/video_ioctl.h"
#include "video/video.h"
#include "video/videobuf.h"
#include "video/video_ppbuf.h"
#include "os/os_api.h"
#include "os/os_type.h"
#include "app_config.h"

#if TCFG_HOST_UVC_ENABLE

#define UVC_CHANNEL_MAX			2
#define UVC_REC_JPG_HEAD_SIZE	8
#define UVC_REC_JPG_ALIGN     	512
#define UVC_JPEG_HEAD 			0xE0FFD8FF
#define UVC_JPEG_HEAD1 			0xC0FFD8FF
#define USB_DMA_CP_ENABLE       (0)//注意：使用硬件dma copy需要异步接收推送数据，否则出现速度快数据错乱的问题


#ifdef CONFIG_UI_ENABLE
#define UVC_RECV_BUFF_SHARE		1 //1:UVC接收数据不受应用层缓存影响，使用共享帧单独给UVC一直接收数据，使用uvc_host_jpg_callback_register注册进行数据获取，应用在屏显等
#else
#define UVC_RECV_BUFF_SHARE        0
#endif
#define UVC_RECV_BUFF_SHARE_SIZE	(100*1024) //UVC一帧JPEG大小，建议100K

static void (*uvc_host_jpg_cb)(char *jpg_buf, u32 jpg_len);
void uvc_host_jpg_callback_register(void (*cb)(char *jpg_buf, u32 jpg_len))
{
    uvc_host_jpg_cb = cb;
}

static int uvc_jpeg_head_check(u32 head)
{
    if (uvc_host_get_fmt()) {
        return true;
    }

    if (head == UVC_JPEG_HEAD) {
        return true;
    } else if (head == UVC_JPEG_HEAD1) {
        return true;
    }
    return 0;
}

struct uvc_dev_control {
    OS_SEM sem;
    struct list_head dev_list;
};
struct mjpg_uvc_image {
    u8 *buf;
    u32 size;
};
struct uvc_fh {
    u8 id;
    u8 eof;
    u8 fps;
    u8 real_fps;
    u8 drop_frame;
    u8 streamon;
    u8 streamoff;
    u8 uvc_out;
    u8 channel;
    u8 alloc_channel;
    u8 open[UVC_CHANNEL_MAX];
    u8 streamon_ch[UVC_CHANNEL_MAX];
    u32 frame_cnt;
    u32 drop_frame_cnt;
    struct list_head entry;
    int free_size;
    u8 *buf;
    u8 *share_buf;
    int b_offset;
    struct dma_list dma_list[16];
    u8 bfmode;
    u8 uvc_video_q;
    void *ppbuf;
    struct videobuf_queue video_q;
    struct video_device *video_dev[UVC_CHANNEL_MAX];
    struct device device[UVC_CHANNEL_MAX];
    void *private_data;
    OS_SEM sem;
    OS_SEM img_sem;
    struct mjpg_uvc_image img;
    volatile u8 image_req;
};
static struct uvc_dev_control uvc_dev;

#define __this  (&uvc_dev)

#define list_for_each_uvc(fh) \
        list_for_each_entry(fh, &__this->dev_list, entry)

#define list_add_uvc(fh) \
        list_add(&fh->entry, &__this->dev_list)

#define list_del_uvc(fh) \
        list_del(&fh->entry);


static int uvc_img_cap(void *_fh, u32 arg)
{
    struct uvc_fh *fh = (struct uvc_fh *)_fh;
    struct video_image_capture *icap = (struct video_image_capture *)arg;
    u32 size;
    u32 *head;
    int err = 0;
    if (!icap) {
        return -EINVAL;
    }
    if (fh) {
        fh->img.buf = icap->baddr;
        fh->img.size = icap->size;
        os_sem_create(&fh->img_sem, 0);
        err = os_sem_pend(&fh->img_sem, 500);
        if (err) {
            log_e("image err");
        }
        fh->image_req = false;
        head = (u32 *)(fh->img.buf + 8);
        if (uvc_jpeg_head_check(*head)) {
            fh->img.size -= 8;
            memcpy(icap->baddr, icap->baddr + 8, fh->img.size);
        }
        os_sem_del(&fh->img_sem, 0);
        if (icap->size < fh->img.size) {
            log_e("image capture size is too small !!!\n");
        } else if (!err) {
            log_d("image capture : %d.%dKB\n", fh->img.size / 1024, (fh->img.size % 1024) * 10 / 1024);
        }
        icap->size = fh->img.size;
        return err;
    } else {
        log_e("image err in on open uvc\n");
    }
    return err;
}
void *uvc_buf_malloc(struct uvc_fh *fh, u32 size, u8 channel)//uvc视频流内存申请
{
    void *b;
    if (fh->uvc_video_q) {
        struct videobuf_buffer *vb;
        vb = videobuf_stream_alloc(&fh->video_q, size);
        if (!vb) {
            return NULL;
        }
        vb->len = size;
        //	printf("video_buf: %x\n", vb, vb->data);
        return vb->data;
    }
    if (!channel) {
        for (int i = 0; i < UVC_CHANNEL_MAX; i++) {
            if (fh->open[i] && fh->streamon_ch[i]) {
                channel = i + 1;
                break;
            }
        }
        if (!channel) {
            return NULL;
        }
    }
    fh->alloc_channel = channel;
    if (!fh->open[channel - 1] || !fh->streamon_ch[channel - 1]) {
        return NULL;
    }
    struct video_device *video_dev = (struct video_device *)fh->video_dev[fh->alloc_channel - 1];
    if (!video_dev) {
        return NULL;
    }
    if (video_dev->bfmode == VIDEO_PPBUF_MODE) {
        b = video_ppbuf_malloc(video_dev->ppbuf, size);
    } else {
        b = video_buf_malloc(video_dev, size);
    }
    return b;
}
void *uvc_buf_realloc(struct uvc_fh *fh, void *buf, int size, u8 channel)//uvc视频流内存重新申请
{
    if (fh->uvc_video_q) {
        struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

        b->len = size;
        videobuf_stream_realloc(&fh->video_q, b, size);

        return b->data;
    }
    if (!channel || !fh->open[channel - 1] || !fh->streamon_ch[channel - 1]) {
        return NULL;
    }
    struct video_device *video_dev = (struct video_device *)fh->video_dev[channel - 1];
    if (video_dev->bfmode == VIDEO_PPBUF_MODE) {
        return video_ppbuf_realloc(video_dev->ppbuf, buf, size);
    } else {
        return video_buf_realloc(video_dev, buf, size);
    }
}
void uvc_buf_free(struct uvc_fh *fh, void *buf, u8 channel)//uvc视频流内存释放
{
    if (fh->uvc_video_q) {
        struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

        ASSERT(buf != NULL, "uvc_buf_free\n");

        videobuf_stream_free(&fh->video_q, b);

        return;
    }
    if (!channel || !fh->open[channel - 1] || !fh->streamon_ch[channel - 1]) {
        return;
    }
    struct video_device *video_dev = (struct video_device *)fh->video_dev[channel - 1];
    if (!video_dev) {
        return ;
    }
    if (video_dev->bfmode == VIDEO_PPBUF_MODE) {
        return video_ppbuf_free(video_dev->ppbuf, buf);
    } else {
        return video_buf_free(video_dev, buf);
    }
}
u32 uvc_buf_free_space(struct uvc_fh *fh, u8 channel)//uvc视频流内存剩余空间
{
    if (fh->uvc_video_q) {
        return videobuf_stream_free_space(&fh->video_q);
    }
    if (!channel) {
find_open:
        for (int i = 0; i < UVC_CHANNEL_MAX; i++) {
            if (fh->open[i] && fh->streamon_ch[i]) {
                channel = i + 1;
                break;
            }
        }
        if (!channel) {
            return 0;
        }
        fh->alloc_channel = channel;
    }
    if (!fh->open[channel - 1] || !fh->streamon_ch[channel - 1]) {
        return 0;
    }
    struct video_device *video_dev = (struct video_device *)fh->video_dev[channel - 1];
    if (!video_dev) {
        channel = 0;
        goto find_open;
    }
    if (video_dev->bfmode == VIDEO_PPBUF_MODE) {
        return video_ppbuf_free_size(video_dev->ppbuf);
    } else {
        return video_buf_free_space(video_dev);
    }
}
void uvc_buf_stream_finish(struct uvc_fh *fh, void *buf, u8 channel)//uvc视频流推流
{
    if (fh->uvc_video_q) {
        struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);
        videobuf_stream_finish(&fh->video_q, b);
        return;
    }
    if (!channel || !buf || !fh->open[channel - 1] || !fh->streamon_ch[channel - 1]) {
        return;
    }
    struct video_device *video_dev = (struct video_device *)fh->video_dev[channel - 1];
    if (!video_dev) {
        return;
    }
    u32 size;
    if (video_dev->bfmode == VIDEO_PPBUF_MODE) {
        if (os_sem_valid(&fh->img_sem) && fh->image_req) {//有拍照申请则复制数据到拍照缓存
            size = video_ppbuf_size(video_dev->ppbuf, buf);
            size = MIN(size, fh->img.size);
            memcpy(fh->img.buf, buf, size);
            fh->img.size = size;
            os_sem_post(&fh->img_sem);
            fh->image_req = false;
        }
        video_ppbuf_output(video_dev->ppbuf, buf);//ppbuf推流
    } else {
        if (os_sem_valid(&fh->img_sem) && fh->image_req) {//有拍照申请则复制数据到拍照缓存
            size = video_buf_size(buf);
            size = MIN(size, fh->img.size);
            memcpy(fh->img.buf, buf, size);
            fh->img.size = size;
            os_sem_post(&fh->img_sem);
            fh->image_req = false;
        }
        video_buf_stream_finish(video_dev, buf);//lbuf推流
    }
}
static int uvc_dev_reqbufs(void *_fh, struct uvc_reqbufs *b)
{

    struct uvc_fh *fh = (struct uvc_fh *)_fh;
    struct video_reqbufs vb_req = {0};

    vb_req.buf = b->buf;
    vb_req.size = b->size;

    videobuf_queue_init(&fh->video_q, 32, "uvc_dev");
    fh->uvc_video_q = 1;
    return videobuf_reqbufs(&fh->video_q, (struct video_reqbufs *)&vb_req);

    /* return 0; */
}

static int uvc_dev_qbuf(void *_fh, struct video_buffer *b)
{

    struct uvc_fh *fh = (struct uvc_fh *)_fh;

    return videobuf_qbuf(&fh->video_q, b);

    /* return 0; */
}


static int uvc_dev_dqbuf(void *_fh, struct video_buffer *b)
{

    struct uvc_fh *fh = (struct uvc_fh *)_fh;

    if (fh->uvc_out) {
        return -ENODEV;
    }
    if (!fh->streamon) {
        return -ENODEV;
    }
    return videobuf_dqbuf(&fh->video_q, b);

    /* return 0; */
}

int uvc_mjpg_stream_out(void *fd, int cnt, void *stream_list, int state)
{
    struct uvc_fh *fh = (struct uvc_fh *)fd;
    struct uvc_stream_list *list = (struct uvc_stream_list *)stream_list;
    int offset;
    int copy_size;
    int i;
    int err = 0;
    u32 tmp_jiffies = 0, req_size;

    if ((cnt < 0) || !list) {
        /*putchar('E');*/
        if (fh->buf) {
            if (fh->buf != fh->share_buf) {//没有多通道共享内存才释放
                uvc_buf_free(fh, fh->buf, fh->alloc_channel);
            }
            fh->buf = NULL;
            if (state != STREAM_SOF) { // eof == 2 next frame start
                fh->drop_frame = 1;
            }
            err = -EINVAL;
        } else if (state == STREAM_SOF) {
            fh->drop_frame = 0;
        }
        goto _exit;
    }

    if (fh->drop_frame) {
        if (state == STREAM_EOF) {
            fh->drop_frame = 0;
        }
        goto _exit;
    }

    if (!fh->buf) {
        if (fh->share_buf) {//有共享内存则使用共享内存大小（一般打开两个通道或者开启UI）
            fh->buf = fh->share_buf;
            fh->free_size = uvc_host_get_fmt() ? UVC_RECV_BUFF_SHARE_SIZE : UVC_RECV_BUFF_SHARE_SIZE - UVC_REC_JPG_ALIGN;
            fh->alloc_channel = 0;
        } else {//没有使用共享内存，则请求剩余空间足够再申请内存
            fh->free_size = uvc_buf_free_space(fh, fh->alloc_channel);
            if (fh->free_size > 1024) {
                fh->buf = uvc_buf_malloc(fh, fh->free_size, fh->alloc_channel);
                if (!uvc_host_get_fmt()) {
                    fh->free_size -= UVC_REC_JPG_ALIGN;//减512防止realloc时512对齐断言
                }
            }
        }
        if (!fh->buf) {
            err = -ENOMEM;
            goto _exit;
        }
        fh->b_offset = 0;
    }

    for (i = 0; i < cnt; i++) {
        if ((fh->b_offset + list[i].length + UVC_REC_JPG_HEAD_SIZE) > fh->free_size) {//数据超过当前可用的内存空间则释放当前帧
            if (fh->buf != fh->share_buf) {
                uvc_buf_free(fh, fh->buf, fh->alloc_channel);
            }
            fh->buf = NULL;
            fh->drop_frame = 1;
            /*putchar('d');*/
            err = -EFAULT;
            goto _exit;
        }
        //复制数据到buf缓存
#if USB_DMA_CP_ENABLE
        fh->dma_list[i].src_addr = list[i].addr;
        fh->dma_list[i].dst_addr = fh->buf + UVC_REC_JPG_HEAD_SIZE + fh->b_offset;
        fh->dma_list[i].len = list[i].length;
#else
        memcpy(fh->buf + UVC_REC_JPG_HEAD_SIZE + fh->b_offset, list[i].addr, list[i].length);
#endif
        fh->b_offset += list[i].length;
    }

#if USB_DMA_CP_ENABLE
    dma_task_copy(fh->dma_list, cnt);//注意：使用硬件dma copy需要异步接收推送数据，否则出现速度快数据错乱的问题
#endif

    if (state == STREAM_EOF) {//USB结束帧则表明一帧图像数据接收完成
        fh->frame_cnt++;
#ifdef CONFIG_UVC_DROP_FRAME_ENABLE
        if (fh->real_fps && fh->fps && fh->real_fps < fh->fps) {//检测USB的帧率和当前应用层请求的帧率是否一直是否需要丢帧处理
            u32 drop = fh->frame_cnt * (fh->fps - fh->real_fps) / fh->fps;
            if (fh->drop_frame_cnt != drop) {//不一致需要丢帧处理
                fh->drop_frame_cnt = drop;
                if (fh->buf != fh->share_buf) {//丢帧
                    uvc_buf_free(fh, fh->buf, fh->alloc_channel);
                }
                fh->buf = NULL;
                goto _exit;
            }
        }
#endif
        if (uvc_host_get_fmt()) {
            req_size = fh->b_offset + UVC_REC_JPG_HEAD_SIZE;
        } else {
            req_size = ADDR_ALIGNE(fh->b_offset + UVC_REC_JPG_HEAD_SIZE, UVC_REC_JPG_ALIGN);//JPEG图像数据需要512对齐
        }
        if (uvc_host_jpg_cb) {//有注册回调则先进入回调函数
            uvc_host_jpg_cb((char *)fh->buf + UVC_REC_JPG_HEAD_SIZE, fh->b_offset);
        }
        if (fh->buf == fh->share_buf && fh->share_buf) {//共享数据，则为多通道
            for (int ch = 1; ch <= UVC_CHANNEL_MAX; ch++) {//多个通道检测
                if (uvc_buf_free_space(fh, ch) > 1024) {//检测剩余空间足够用再申请复制数据
                    u8 *pbuf = uvc_buf_malloc(fh, req_size, ch);
                    if (pbuf) {
                        memcpy(pbuf + UVC_REC_JPG_HEAD_SIZE, fh->buf + UVC_REC_JPG_HEAD_SIZE, fh->b_offset);
                        fh->buf = pbuf;
                        if (fh->buf) {
                            /*memset(fh->buf + fh->b_offset + UVC_REC_JPG_HEAD_SIZE, 0, req_size - fh->b_offset - UVC_REC_JPG_HEAD_SIZE);*/
                            u32 *head = (u32 *)(fh->buf + UVC_REC_JPG_HEAD_SIZE);
                            if (uvc_jpeg_head_check(*head)) {//头部校验
                                uvc_buf_stream_finish(fh, fh->buf, ch);//推流往应用层
                            } else {
                                uvc_buf_free(fh, fh->buf, ch);//数据出错释放当前帧
                            }
                        }
                    }
                }
            }
        } else {//单通道
            fh->buf = uvc_buf_realloc(fh, fh->buf, req_size, fh->alloc_channel);//重新申请内存，会释放多余的空间
            if (fh->buf) {
                /*memset(fh->buf + fh->b_offset + UVC_REC_JPG_HEAD_SIZE, 0, req_size - fh->b_offset - UVC_REC_JPG_HEAD_SIZE);*/
                u32 *head = (u32 *)(fh->buf + UVC_REC_JPG_HEAD_SIZE);
                if (uvc_jpeg_head_check(*head)) {//头部校验
                    uvc_buf_stream_finish(fh, fh->buf, fh->alloc_channel);//推流往应用层
                } else {
                    uvc_buf_free(fh, fh->buf, fh->alloc_channel);//数据出错释放当前帧
                }
            }
        }
        fh->buf = NULL;
    }

_exit:
    fh->streamoff = 0;
    return err;
}
int uvc_h264_stream_out(void *fd, int cnt, void *stream_list, int state)
{
    struct uvc_fh *fh = (struct uvc_fh *)fd;
    struct uvc_stream_list *list = (struct uvc_stream_list *)stream_list;
    int offset;
    int copy_size;
    int i;
    int err = 0;
    u32 tmp_jiffies = 0;
    if ((cnt < 0) || !list) {
        /* putchar('E'); */
        if (fh->buf) {
            if (fh->buf != fh->share_buf) {//没有多通道共享内存才释放
                uvc_buf_free(fh, fh->buf, 0);
            }
            fh->buf = NULL;
            if (state != STREAM_SOF) { // eof == 2 next frame start
                fh->drop_frame = 1;
            }
            err = -EINVAL;
        } else if (state == STREAM_SOF) {
            fh->drop_frame = 0;
        }
        goto _exit;
    }

    if (fh->drop_frame) {
        /* puts("drop\n"); */
        if (state == STREAM_EOF) {
            fh->drop_frame = 0;
        }
        goto _exit;
    }

    if (!fh->buf) {
        fh->free_size = uvc_buf_free_space(fh, 0);
        if (fh->free_size > 1024) {
            fh->buf = uvc_buf_malloc(fh, fh->free_size, 0);
        }
        if (!fh->buf) {
            /* puts("no mem\n"); */
            err = -ENOMEM;
            goto _exit;
        }
        fh->b_offset = 0;
    }

    for (i = 0; i < cnt; i++) {
        if ((fh->b_offset + list[i].length) > fh->free_size) {
            uvc_buf_free(fh, fh->buf, 0);
            fh->buf = NULL;
            fh->drop_frame = 1;
            putchar('d');
            /* printf("%d", fh->id); */
            err = -EFAULT;
            goto _exit;
        }
#if USB_DMA_CP_ENABLE
        fh->dma_list[i].src_addr = list[i].addr;
        fh->dma_list[i].dst_addr = fh->buf + fh->b_offset;
        fh->dma_list[i].len = list[i].length;
#else
        memcpy(fh->buf + fh->b_offset, list[i].addr, list[i].length);
#endif
        fh->b_offset += list[i].length;
    }

#if USB_DMA_CP_ENABLE
    dma_task_copy(fh->dma_list, cnt);
#endif

    if (state == STREAM_EOF) {
        /* puts("h264\n"); */
        fh->buf = uvc_buf_realloc(fh, fh->buf, fh->b_offset, 0);
        uvc_buf_stream_finish(fh, fh->buf, 0);
        fh->buf = NULL;
    }

_exit:
    fh->streamoff = 0;
    return err;
}
static int uvc_stream_on(void *device, void *_fh, int index)
{
    int err = 0;
    u8 channel = 0;
    struct uvc_fh *fh = (struct uvc_fh *)_fh;

    os_sem_pend(&fh->sem, 0);

    for (int i = 0; i < UVC_CHANNEL_MAX; i++) {
        if ((u32)&fh->device[i] == (u32)device) {
            channel = i + 1;
            break;
        }
    }
    ASSERT(channel, "err int uvc_stream_on\n");
    if (fh->uvc_out) {
        os_sem_post(&fh->sem);
        return -EINVAL;
    }

    fh->eof = 1;
    fh->drop_frame = 0;

    if (fh->streamon) {
        fh->streamon++;
        fh->streamon_ch[channel - 1]++;
        os_sem_post(&fh->sem);
        return 0;
    }
    err = uvc_host_open_camera(fh->private_data);//打开USB，并配置UVC的视频流
    if (err) {
        printf("uvc_stream_on err\n");
        os_sem_post(&fh->sem);
        return err;
    }
    fh->streamon++;
    fh->streamon_ch[channel - 1]++;
    if (fh->uvc_video_q) {
        videobuf_streamon(&fh->video_q, (u8 *)&channel);
    }
    os_sem_post(&fh->sem);
    return err;
}
static int uvc_set_real_fps(void *_fh, u8 fp)
{
    int err;
    struct uvc_fh *fh = (struct uvc_fh *)_fh;
    fh->real_fps = fp;
    return 0;
}
static int uvc_stream_off(void *device, void *_fh, int index)
{
    int err = 0;
    struct uvc_fh *fh = (struct uvc_fh *)_fh;
    u32 time = jiffies + msecs_to_jiffies(10);
    u8 channel = 0;

    os_sem_pend(&fh->sem, 0);
    for (int i = 0; i < UVC_CHANNEL_MAX; i++) {
        if ((u32)&fh->device[i] == (u32)device) {
            channel = i + 1;
            break;
        }
    }
    ASSERT(channel, "err int uvc_stream_off\n");
    fh->streamon_ch[channel - 1]--;
    if (fh->streamon && --fh->streamon) {
        os_sem_post(&fh->sem);
        return 0;
    }
    if (!fh->uvc_out) {
        uvc_host_close_camera(fh->private_data);//关闭UVC视频流
    }
    fh->streamoff = 1;
    while (fh->streamoff) {
        if (time_after(jiffies, time)) {
            break;
        }
    }

    if (fh->buf) {
        if (fh->buf != fh->share_buf) {
            uvc_buf_free(fh, fh->buf, fh->alloc_channel);
        }
        fh->buf = NULL;
        fh->b_offset = 0;
    }
    if (fh->uvc_video_q) {
        err = videobuf_streamoff(&fh->video_q, 0);
    }
    os_sem_post(&fh->sem);
    return err;

}

static int uvc_dev_init(const struct dev_node *node, void *_data)
{
    //struct uvc_platform_data *data = (struct uvc_platform_data *)_data;
    os_sem_create(&__this->sem, 1);
    INIT_LIST_HEAD(&__this->dev_list);
    return 0;
}

static bool uvc_dev_online(const struct dev_node *node)
{
    int err = 0;
    int id;

    id = uvc_host_online();
    if (id < 0) {
        return false;
    }

    return true;
}

static int uvc_dev_out(void *fd)
{
    struct uvc_fh *fh = (struct uvc_fh *)fd;

    os_sem_pend(&fh->sem, 0);

    fh->uvc_out = 1;

    os_sem_post(&fh->sem);
    return 0;

}

static int uvc_dev_open(const char *_name, struct device **device, void *arg)
{
    struct uvc_fh *fh = NULL;
    struct uvc_host_param param = {0};
    char name[8];
    int id, i;
    u8 channel = 0;

    struct video_var_param_info *info = (struct video_var_param_info *)arg;
    id = info->f->uvc_id;
    list_for_each_uvc(fh) {
        if (fh->id == id) {//已经被打开过则使用另外通道
            for (i = 0; i < UVC_CHANNEL_MAX; i++) {
                if (!fh->open[i]) {
                    channel = i;
                    break;
                }
            }
            if (i >= UVC_CHANNEL_MAX) {
                log_e("uvc no channel \n");
                os_sem_post(&__this->sem);
                return -EINVAL;
            }
            *device = &fh->device[channel];
            (*device)->private_data = fh;
            fh->video_dev[channel] = (struct video_device *)info->priv;
            if (fh->video_dev[channel]->bfmode == VIDEO_PPBUF_MODE && !fh->video_dev[channel]->ppbuf) {
                fh->video_dev[channel]->ppbuf = video_ppbuf_open();//另外通道的ppbuf则需要重新设置
                if (!fh->video_dev[channel]->ppbuf) {
                    log_e("video_ppbuf_open no men");
                    os_sem_post(&__this->sem);
                    return -ENOMEM;
                }
            }
            if (!fh->share_buf) {//两个通道则使用共享内存，防止一个通道卡住另一个通道也被卡住问题
                fh->share_buf = malloc(UVC_RECV_BUFF_SHARE_SIZE);
            }
            fh->open[channel] = 1;
            fh->channel++;
            os_sem_post(&__this->sem);
            return 0;
        }
    }

    fh = (struct uvc_fh *)zalloc(sizeof(*fh));
    if (!fh) {
        os_sem_post(&__this->sem);
        return -ENOMEM;
    }
    sprintf(name, "uvc%d", id);
    param.name = name;//node->name;
    if (info->f->pixelformat & VIDEO_PIX_FMT_H264) {
        param.uvc_stream_out = uvc_h264_stream_out;
    } else {
        param.uvc_stream_out = uvc_mjpg_stream_out;
    }
    param.uvc_out = uvc_dev_out;
    param.priv = fh;
    printf("open uvc name = %s \n", name);
    fh->private_data = uvc_host_open(&param);//初始化UVC主机
    if (!fh->private_data) {
        free(fh);
        os_sem_post(&__this->sem);
        return -EINVAL;
    }
    if (info->priv) {
        fh->video_dev[channel] = (struct video_device *)info->priv;
        if (fh->video_dev[channel]->bfmode == VIDEO_PPBUF_MODE && !fh->video_dev[channel]->ppbuf) {
            fh->video_dev[channel]->ppbuf = video_ppbuf_open();//使用ppbuf
            if (!fh->video_dev[channel]->ppbuf) {
                log_e("video_ppbuf_open no men");
                free(fh);
                os_sem_post(&__this->sem);
                return -ENOMEM;
            }
        }
    }
#if UVC_RECV_BUFF_SHARE
    if (!fh->share_buf) {
        fh->share_buf = malloc(UVC_RECV_BUFF_SHARE_SIZE);
        if (!fh->share_buf) {
            log_e("uvc share_buf no men");
            free(fh);
            os_sem_post(&__this->sem);
            return -ENOMEM;
        }
    }
#endif
    os_sem_create(&fh->sem, 0);
    list_add_uvc(fh);//打开后添加到链表，在多通道打开则需要查找链表
    os_sem_post(&fh->sem);
    *device = &fh->device[channel];
    (*device)->private_data = fh;
    fh->id = id;
    fh->open[channel] = 1;
    fh->channel++;
    os_sem_post(&__this->sem);
    return 0;
}

static int uvc_querycap(struct uvc_fh *fh, struct uvc_capability *cap)//获取分辨率
{
    int num;
    struct uvc_frame_info *reso_table;

    os_sem_pend(&fh->sem, 0);
    num = uvc_host_get_pix_table(fh->private_data, &reso_table);

    if (num > (sizeof(cap->reso) / sizeof(cap->reso[0]))) {
        num = sizeof(cap->reso) / sizeof(cap->reso[0]);
    }

    memcpy(cap->reso, reso_table, sizeof(cap->reso[0]) * num);
    cap->fmt = UVC_CAMERA_FMT_MJPG;
    cap->reso_num = num;
    cap->fps = uvc_host_get_fps(fh->private_data);
    fh->fps = cap->fps;
    //printf("defualt : uvc_querycap fps = %d, %d*%d\n", cap->fps, cap->reso[cap->reso_num - 1].width, cap->reso[cap->reso_num - 1].height);
    os_sem_post(&fh->sem);
    return 0;
}

static int uvc_req_processing_unit(struct uvc_fh *fh, struct uvc_processing_unit *pu)
{
    int err = 0;

    os_sem_pend(&fh->sem, 0);

    if (!fh->uvc_out) {
        err = uvc_host_req_processing_unit(fh->private_data, pu);
    }

    os_sem_post(&fh->sem);
    return err;
}

static int uvc_dev_reset(struct uvc_fh *fh)
{
    int err = 0;
    os_sem_pend(&fh->sem, 0);

    if (!fh->uvc_out) {
        err = uvc_force_reset(fh->private_data);
    }

    os_sem_post(&fh->sem);

    return err;
}

static int usb_ioctl(struct uvc_fh *fh, u32 cmd, void *arg)
{
    int err = 0;
    if (!fh) {
        return -EINVAL;
    }
    os_sem_pend(&fh->sem, 0);

    if (!fh->uvc_out) {
        err = uvc2usb_ioctl(fh->private_data, cmd, arg);
    }

    os_sem_post(&fh->sem);

    return err;
}

static int uvc_dev_ioctl(struct device *device, u32 cmd, u32 arg)
{
    int ret = 0;
    struct uvc_fh *fh = (struct uvc_fh *)device->private_data;

    switch (cmd) {
    case UVCIOC_QUERYCAP:
        ret = uvc_querycap(fh, (struct uvc_capability *)arg);
        break;
    case UVCIOC_SET_CAP_SIZE:
        ret = uvc_host_set_pix(fh->private_data, (int)arg);
        break;
    case UVCIOC_REQBUFS:
        ret = uvc_dev_reqbufs(fh, (struct uvc_reqbufs *)arg);
        break;
    case UVCIOC_DQBUF:
        ret = uvc_dev_dqbuf(fh, (struct video_buffer *)arg);
        break;
    case UVCIOC_QBUF:
        ret = uvc_dev_qbuf(fh, (struct video_buffer *)arg);
        break;
    case UVCIOC_STREAM_ON:
        ret = uvc_stream_on(device, fh, arg);
        break;
    case UVCIOC_STREAM_OFF:
        ret = uvc_stream_off(device, fh, arg);
        break;
    case UVCIOC_GET_IMAMGE:
        ret = uvc_img_cap(fh, arg);
        break;
    case UVCIOC_RESET:
        ret = uvc_dev_reset(fh);
        break;
    case UVCIOC_REQ_PROCESSING_UNIT:
        ret = uvc_req_processing_unit(fh, (struct uvc_processing_unit *)arg);
        break;
    case UVCIOC_SET_CUR_GRAY:
        /*set_uvc_gray(arg);*/
        break;
    case UVCIOC_SET_CUR_FPS:
        uvc_set_real_fps(fh, (u8)arg);
        break;
    default:
        ret = usb_ioctl(fh, cmd, (void *)arg);
        break;
    }

    return ret;
}

static int uvc_dev_close(struct device *device)
{
    struct uvc_fh *fh = (struct uvc_fh *)device->private_data;
    struct video_device *video_dev;
    if (fh && --fh->channel == 0) {//两个通道都关闭了
        os_sem_pend(&fh->sem, 0);
        uvc_host_close(fh->private_data);
        for (int i = 0; i < UVC_CHANNEL_MAX; i++) {//多通道关闭
            if (&fh->device[i] == device) {//关闭是当前通道
                fh->open[i] = 0;
                if (fh->video_dev[i]) {
                    video_dev = (struct video_device *)fh->video_dev[i];
                    if (video_dev->bfmode == VIDEO_PPBUF_MODE) {
                        video_ppbuf_close(video_dev->ppbuf);
                    }
                    fh->video_dev[i] = NULL;
                }
                memset(&fh->device[i], 0, sizeof(struct device));
            }
        }
        if (fh->uvc_video_q) {
            videobuf_queue_release(&fh->video_q);
        }
        list_del_uvc(fh);
        if (fh->share_buf) {
            free(fh->share_buf);
            fh->share_buf = NULL;
        }
        free(fh);
    } else if (fh) {//关闭1个通道
        os_sem_pend(&fh->sem, 0);
        for (int i = 0; i < UVC_CHANNEL_MAX; i++) {
            if (&fh->device[i] == device) {//只关闭是当前通道
                fh->open[i] = 0;
                video_dev = (struct video_device *)fh->video_dev[i];
                if (video_dev->bfmode == VIDEO_PPBUF_MODE) {
                    video_ppbuf_close(video_dev->ppbuf);
                }
                fh->video_dev[i] = NULL;
                memset(&fh->device[i], 0, sizeof(struct device));
            }
        }
        if (fh->uvc_video_q) {
            videobuf_queue_release(&fh->video_q);
        }
        os_sem_post(&fh->sem);
    }
    return 0;
}



const struct device_operations uvc_dev_ops = {
    .init  =  uvc_dev_init,
    .open  =  uvc_dev_open,
    .ioctl =  uvc_dev_ioctl,
    .close =  uvc_dev_close,
    .online = uvc_dev_online,
};
#endif
