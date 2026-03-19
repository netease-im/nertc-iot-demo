#include "app_config.h"
#include "device/device.h"//u8
#include "lcd_config.h"//LCD_h
#include "system/includes.h"//late_initcall
#include "yuv_soft_scalling.h"//YUV420p_Soft_Scaling
#include "get_yuv_data.h"//get_yuv_init
#include "asm/includes.h"

//测试用例将摄像头数据任意放大并保存jpg到SD卡 需要注意长宽只能为16的整数倍
//测试结果40M数据4.6s加写卡
#ifdef LARGE_SCALE_PHOTOGRAPHY

#define IN_W 640                    //图像输入尺寸 宽
#define IN_H 480                    //图像输入尺寸 高
#define AIM_W 8448                  //图像输出尺寸 宽
#define AIM_H 4736                  //图像输出尺寸 宽
#define CALCULATE_H 16              //图像每次处理行数 //只能处理16行的整数倍
#define CNT                         (AIM_H/CALCULATE_H) //累计需要处理次数为输出高/每次行数
#define YUV_BUF_16_LINE_SIZE        (AIM_W*CALCULATE_H*3/2) //一次处理数据需要的内存长度
#define JPG_BUF_LEN                 (2*1024*1024) //jpg数据处理需要的内存长度 //需要评估最大内存
#define JPG_Q                       10  //输出jpg的质量 最大16

int isc_log_en()//如果定义该函数丢帧信息屏蔽
{
    return 0;
}

struct scale_img {
    int yuv_pixformat;
    int in_w;
    int in_h;
    u8 *y_in;
    u8 *u_in;
    u8 *v_in;
    int out_w;
    int out_h;
    u8 *y_out;
    u8 *u_out;
    u8 *v_out;
    u8 *line_buf;
    int line_buf_size;
    u8 line;
    u8 jpg_q;
    u32 jpg_buf_size;
    u8 *jpg_buf;
    struct jpeg_encoder_fd *jpg;
    struct YUV_frame_data *input_frame;
};

static struct scale_img *hdl;


/********保存YUV数据到SD卡中************/
static void save_YUV_date_ontime1(u8 *buf, u32 len)
{
    if (storage_device_ready()) {
        char file_name[64];//定义路径存储
        snprintf(file_name, 64, CONFIG_ROOT_PATH"YUV/Y1UV***.bin");
        FILE *fd = fopen(file_name, "wb+");
        fwrite(buf, 1, len, fd);
        fclose(fd);
        printf("YUV save ok name=YUV\r\n");
    }
}
/********保存JPG图片到SD卡中************/
static void save_jpg_ontime(u8 *buf, u32 len)
{
    if (storage_device_ready()) {//自动重命名文件数量限制配置搜索 FILE_AUTO_RENAME_NUM
        char file_name[64];//定义路径存储
        snprintf(file_name, 64, CONFIG_ROOT_PATH"JPG/JPG****.JPG");
        FILE *fd = fopen(file_name, "wb+");
        printf("[msg]>>>>>>>>>>>file_name=%s", file_name);
        fwrite(buf, 1, len, fd);
        fclose(fd);
        printf("JPG save ok name=JPG\r\n");
    }
}


static u8 *jpg_buf = NULL;

/* static u8 test_buf[1280*720]; */

/******数据流程******************/
/******yuv回调出数据后转为对应屏幕大小YUV交给下一个线程处理***********/
/******这样显示和yuv资源占用差不多才能同步均匀*************************************/
static void get_yuv(u8 *yuv_buf, u32 len, int yuv_in_w, int yuv_in_h)//YUV数据回调线程
{
    u32 time, time1;
    u16 i;
    int err;

    if (!storage_device_ready()) {
        printf("\n [msg] %s -[please insert SD card] %d\n", __FUNCTION__, __LINE__);
        os_time_dly(100);
        return;
    }

    hdl->jpg = mjpg_image_enc_open(NULL, JPEG_ENC_MANU_IMAGE);
    if (!hdl->jpg) {
        printf("mjpg_image_enc_open err\n");
        goto exit;
    }

    time1 = timer_get_ms();
    //缩放处理
    for (i = 0; i < CNT; i++) {
        hdl->y_in = yuv_buf;
        hdl->y_out = hdl->line_buf;
        //16行yuv有16行y
        jl_nearest_inter_line(hdl->line, i, hdl->in_w, hdl->in_h, hdl->out_w, hdl->out_h, hdl->y_in, hdl->y_out);
        hdl->u_in = yuv_buf + hdl->in_w * hdl->in_h;
        hdl->u_out = hdl->line_buf + hdl->out_w * hdl->line;
        //16行yuv有8行u
        jl_nearest_inter_line(hdl->line / 2, i, hdl->in_w / 2, hdl->in_h / 2, hdl->out_w / 2, hdl->out_h / 2, hdl->u_in, hdl->u_out);
        hdl->v_in = yuv_buf + hdl->in_w * hdl->in_h * 5 / 4;
        hdl->v_out = hdl->line_buf + hdl->out_w * hdl->line * 5 / 4;
        //16行yuv有8行v
        jl_nearest_inter_line(hdl->line / 2, i, hdl->in_w / 2, hdl->in_h / 2, hdl->out_w / 2, hdl->out_h / 2, hdl->v_in, hdl->v_out);
        //6.每1行MCU(16行像素点)编码一次
        hdl->input_frame->line_num = i * hdl->line;
        hdl->input_frame->y = hdl->y_out;
        hdl->input_frame->u = hdl->u_out;
        hdl->input_frame->v = hdl->v_out;
        err = mjpg_image_enc_start(hdl->jpg, hdl->input_frame, hdl->jpg_buf, hdl->jpg_buf_size, hdl->jpg_q);
        if (err < 0) {
            printf("\n [ERROR] %s -[yuyu] %d\n", __FUNCTION__, __LINE__);
            goto exit;
        }
    }

    save_jpg_ontime(hdl->jpg_buf, hdl->jpg->file_size);
    printf("[msg]>>>>>>>>>>>hdl->jpg->file_size=%dk", hdl->jpg->file_size / 1024);

    time = timer_get_ms() - time1;
    printf("[msg]>>>>>>>>>>>time=%d", time);

    mjpg_image_enc_close(hdl->jpg);

    return;
exit:
    while (1) {
        printf("\n [ERROR] %s -[yuyu] %d\n", __FUNCTION__, __LINE__);
        os_time_dly(100);
    }
}

static void large_scale_photography_task(void)
{
    //1.申请全局结构体内存
    hdl = malloc(sizeof(struct scale_img));
    if (hdl == NULL) {
        printf("[error]>>>>>>>>>>>>malloc fail");
    }

    //2.申请16行buf
    hdl->line_buf = malloc(YUV_BUF_16_LINE_SIZE);
    if (!hdl->line_buf) {
        goto exit;
    }

    //3.申请jpg输入帧结构体
    hdl->input_frame = malloc(sizeof(struct YUV_frame_data));
    if (hdl->input_frame == NULL) {
        goto exit;
    }

    //4.申请jpg_buf用于输出jpg照片
    hdl->jpg_buf = malloc(JPG_BUF_LEN);
    if (hdl->jpg_buf == NULL) {
        goto exit;
    }

    //5.配置全局参数配置
    hdl->in_w = IN_W;                                  //配置输入图像yuv420宽
    hdl->in_h = IN_H;                                  //配置输入图像yuv420高
    hdl->out_w = AIM_W;                                //配置输出图像yuv420宽
    hdl->out_h = AIM_H;                                //配置输出图像yuv420高
    hdl->line = CALCULATE_H;                           //配置每次放大行数必须为16的整数倍
    hdl->line_buf_size = YUV_BUF_16_LINE_SIZE;         //配置yuv420 16行buf长度
    hdl->jpg_buf_size = JPG_BUF_LEN;                   //配置jpg内存长度
    hdl->yuv_pixformat = JPG_SAMP_FMT_YUV420;          //配置jpg编码数据格式
    hdl->jpg_q = JPG_Q;                                //配置编码q值
    hdl->input_frame->width = hdl->out_w;              //配置jpg编码器输出jpg图像宽
    hdl->input_frame->height = hdl->out_h;             //配置jpg编码器输出jpg图像高
    hdl->input_frame->data_height = hdl->line;         //配置jpg编码器每次编码行数
    hdl->input_frame->pixformat = JPG_SAMP_FMT_YUV420; //配置jpg编码器编码数据格式

    /******YUV数据回调初始化**********/
    get_yuv_init(get_yuv);

    return;
exit:
    while (1) {
        printf("\n [ERROR] %s -[yuyu] %d\n", __FUNCTION__, __LINE__);
        os_time_dly(100);
    }
}

static int large_scale_photography_task_init(void)
{
    puts("large_scale_photography_task_init \n\n");
    return thread_fork("large_scale_photography_task", 11, 512, 32, 0, large_scale_photography_task, NULL);
}
late_initcall(large_scale_photography_task_init);

#endif
