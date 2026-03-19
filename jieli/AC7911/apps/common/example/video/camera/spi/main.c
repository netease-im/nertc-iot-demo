#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "asm/spi.h"
#include "camera.h"
#include "device/gpio.h"
#include "device/device.h"//u8
#include "storage_device.h"//SD
#include "server/video_dec_server.h"//fopen
#include "system/includes.h"//GPIO
#include "asm/clock.h"
#include "asm/pwm.h"
#include "lcd_config.h"//LCD_h
#include "system/includes.h"//late_initcall
#include "yuv_soft_scalling.h"//YUV420p_Soft_Scaling
#include "get_yuv_data.h"//get_yuv_init
#include "lcd_drive.h"
#include "yuv_to_rgb.h"
#include "spi_video.h"
#include "lcd_data_driver.h"

#ifdef USE_CAMERA_SPI_SHOW_TO_LCD_DEMO

static u16 recv_cnt = 2;

static struct spi_video *hdl;

struct lbuf_test_head {
    u8 data[0];
};

static void *lbuf_ptr = NULL;
static struct lbuff_head *lbuf_handle = NULL;
static struct lbuff_head *lib_system_lbuf_init(u32 buf_size)
{
    struct lbuff_head *lbuf = NULL;
    lbuf_ptr = malloc(buf_size);
    if (lbuf_ptr == NULL) {
        printf("lbuf malloc buf err");
        return NULL;
    }
    lbuf = lbuf_init(lbuf_ptr, buf_size, 4, sizeof(struct lbuf_test_head));

    return lbuf;
}

static ___interrupt void spi_irq_cb(void); //中断函数，指定在内部sram
static void Calculation_frame(void)
{
    static u32 tstart = 0, tdiff = 0;
    static u8 fps_cnt = 0;
    fps_cnt++ ;
    if (!tstart) {
        tstart = timer_get_ms();
    } else {
        tdiff = timer_get_ms() - tstart;
        if (tdiff >= 1000) {
            printf("\n [MSG]spi_video_fps_count = %d\n", fps_cnt *  1000 / tdiff);
            tstart = 0;
            fps_cnt = 0;
        }
    }
}

/********保存YUV数据到SD卡中************/
static void save_YUV_date_ontime(u8 *buf, u32 len)
{
    if (storage_device_ready()) {
        char file_name[64];//定义路径存储
        snprintf(file_name, 64, CONFIG_ROOT_PATH"YUV/YUV***.bin");
        FILE *fd = fopen(file_name, "wb+");
        fwrite(buf, 1, len, fd);
        fclose(fd);
        printf("YUV save ok name=YUV\r\n");
    }
}
static void debug_io_init(void)
{
#ifdef SPI_IO_DBG
    DBG_PORT->DIR &= ~DBG_PORT_BIT;
    DBG_PORT->PU &= ~DBG_PORT_BIT;
    DBG_PORT->PD &= ~DBG_PORT_BIT;
    DBG_PORT->OUT &= ~DBG_PORT_BIT;
#endif
}

static void *pwm_dev = NULL;
static void spi_xclk_open(void)
{
    struct pwm_platform_data pwm = {0};
    if (!pwm_dev) {
        pwm_dev = dev_open("pwm1", &pwm);
    }
    if (!pwm_dev) {
        printf("open spi_xclk pwm fail\r\n");
        return;
    }
}
static void spi_xclk_close(void)
{
    struct pwm_platform_data pwm = {0};

    if (!pwm_dev) {
        printf("spi_xclk pwm no open\r\n");
        return;
    }
    dev_ioctl(pwm_dev, PWM_REMOV_CHANNEL, (u32)&pwm);
    dev_ioctl(pwm_dev, PWM_STOP, (u32)&pwm);
    dev_close(pwm_dev);
    pwm_dev = NULL;
}

const short Table_fv1[256] = { -180, -179, -177, -176, -174, -173, -172, -170, -169, -167, -166, -165, -163, -162, -160, -159, -158, -156, -155, -153, -152, -151, -149, -148, -146, -145, -144, -142, -141, -139, -138, -137,  -135, -134, -132, -131, -130, -128, -127, -125, -124, -123, -121, -120, -118, -117, -115, -114, -113, -111, -110, -108, -107, -106, -104, -103, -101, -100, -99, -97, -96, -94, -93, -92, -90,  -89, -87, -86, -85, -83, -82, -80, -79, -78, -76, -75, -73, -72, -71, -69, -68, -66, -65, -64, -62, -61, -59, -58, -57, -55, -54, -52, -51, -50, -48, -47, -45, -44, -43, -41, -40, -38, -37,  -36, -34, -33, -31, -30, -29, -27, -26, -24, -23, -22, -20, -19, -17, -16, -15, -13, -12, -10, -9, -8, -6, -5, -3, -2, 0, 1, 2, 4, 5, 7, 8, 9, 11, 12, 14, 15, 16, 18, 19, 21, 22, 23, 25, 26, 28, 29, 30, 32, 33, 35, 36, 37, 39, 40, 42, 43, 44, 46, 47, 49, 50, 51, 53, 54, 56, 57, 58, 60, 61, 63, 64, 65, 67, 68, 70, 71, 72, 74, 75, 77, 78, 79, 81, 82, 84, 85, 86, 88, 89, 91, 92, 93, 95, 96, 98, 99, 100, 102, 103, 105, 106, 107, 109, 110, 112, 113, 114, 116, 117, 119, 120, 122, 123, 124, 126, 127, 129, 130, 131, 133, 134, 136, 137, 138, 140, 141, 143, 144, 145, 147, 148,  150, 151, 152, 154, 155, 157, 158, 159, 161, 162, 164, 165, 166, 168, 169, 171, 172, 173, 175, 176, 178 };
const short Table_fv2[256] = { -92, -91, -91, -90, -89, -88, -88, -87, -86, -86, -85, -84, -83, -83, -82, -81, -81, -80, -79, -78, -78, -77, -76, -76, -75, -74, -73, -73, -72, -71, -71, -70, -69, -68, -68, -67, -66, -66, -65, -64, -63, -63, -62, -61, -61, -60, -59, -58, -58, -57, -56, -56, -55, -54, -53, -53, -52, -51, -51, -50, -49, -48, -48, -47, -46, -46, -45, -44, -43, -43, -42, -41, -41, -40, -39, -38, -38, -37, -36, -36, -35, -34, -33, -33, -32, -31, -31, -30, -29, -28, -28, -27, -26, -26, -25, -24, -23, -23, -22, -21, -21, -20, -19, -18, -18, -17, -16, -16, -15, -14, -13, -13, -12, -11, -11, -10, -9, -8, -8, -7, -6, -6, -5, -4, -3, -3, -2, -1, 0, 0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 27, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 37, 38, 39, 40, 40, 41, 42, 42, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50, 50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 57, 58, 59, 60, 60, 61, 62, 62, 63, 64, 65, 65, 66, 67, 67, 68, 69, 70, 70, 71, 72, 72, 73, 74, 75, 75, 76, 77, 77, 78, 79, 80, 80, 81, 82, 82, 83, 84, 85, 85, 86, 87, 87, 88, 89, 90, 90 };
const short Table_fu1[256] = { -44, -44, -44, -43, -43, -43, -42, -42, -42, -41, -41, -41, -40, -40, -40, -39, -39, -39, -38, -38, -38, -37, -37, -37, -36, -36, -36, -35, -35, -35, -34, -34, -33, -33, -33, -32, -32, -32, -31, -31, -31, -30, -30, -30, -29, -29, -29, -28, -28, -28, -27, -27, -27, -26, -26, -26, -25, -25, -25, -24, -24, -24, -23, -23, -22, -22, -22, -21, -21, -21, -20, -20, -20, -19, -19, -19, -18, -18, -18, -17, -17, -17, -16, -16, -16, -15, -15, -15, -14, -14, -14, -13, -13, -13, -12, -12, -11, -11, -11, -10, -10, -10, -9, -9, -9, -8, -8, -8, -7, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -3, -3, -3, -2, -2, -2, -1, -1, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43 };
const short Table_fu2[256] = { -227, -226, -224, -222, -220, -219, -217, -215, -213, -212, -210, -208, -206, -204, -203, -201, -199, -197, -196, -194, -192, -190, -188, -187, -185, -183, -181, -180, -178, -176, -174, -173, -171, -169, -167, -165, -164, -162, -160, -158, -157, -155, -153, -151, -149, -148, -146, -144, -142, -141, -139, -137, -135, -134, -132, -130, -128, -126, -125, -123, -121, -119, -118, -116, -114, -112, -110, -109, -107, -105, -103, -102, -100, -98, -96, -94, -93, -91, -89, -87, -86, -84, -82, -80, -79, -77, -75, -73, -71, -70, -68, -66, -64, -63, -61, -59, -57, -55, -54, -52, -50, -48, -47, -45, -43, -41, -40, -38, -36, -34, -32, -31, -29, -27, -25, -24, -22, -20, -18, -16, -15, -13, -11, -9, -8, -6, -4, -2, 0, 1, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19, 21, 23, 24, 26, 28, 30, 31, 33, 35, 37, 39, 40, 42, 44, 46, 47, 49, 51, 53, 54, 56, 58, 60, 62, 63, 65, 67, 69, 70, 72, 74, 76, 78, 79, 81, 83, 85, 86, 88, 90, 92, 93, 95, 97, 99, 101, 102, 104, 106, 108, 109, 111, 113, 115, 117, 118, 120, 122, 124, 125, 127, 129, 131, 133, 134, 136, 138, 140, 141, 143, 145, 147, 148, 150, 152, 154, 156, 157, 159, 161, 163, 164, 166, 168, 170, 172, 173, 175, 177, 179, 180, 182, 184, 186, 187, 189, 191, 193, 195, 196, 198, 200, 202, 203, 205, 207, 209, 211, 212, 214, 216, 218, 219, 221, 223, 225 };

#define TIME_DEBUG 0
void yuyv422_soft_rgb565(u8 *in_yuyv422, u8 *out_rgb_565, int sw, int sh, int dw, int dh) sec(.volatile_ram_code)
{
#if TIME_DEBUG
    u32 time1 = timer_get_ms();
#endif

    float Qw = sw / dh;
    float Qh = sh / dw;
    u8 *yuyv422;
    u8 *rgb565;
    u8 y, u, v;
    int r, g, b;
    u16 i, j;
    static u8 count_flag = 0;
    yuyv422 = in_yuyv422;
    rgb565  = (u8 *)out_rgb_565;

    //图像会镜像 可以调整屏的镜像 或者 摄像头镜像处理
    for (j = 0; j < dh / 2; j++) {
        for (i = 0; i < dw; i++) {
            y = yuyv422[(sw * (u16)(i * Qh) + (u16)(j * Qw)) * 2];
            u = yuyv422[(sw * (u16)(i * Qh) + (u16)(j * Qw) - (j % 2)) * 2 + 1];
            v = yuyv422[(sw * (u16)(i * Qh) + (u16)(j * Qw) - (j % 2)) * 2 + 3];
            r = y + Table_fv1[v];
            g = y - Table_fu1[u] - Table_fv2[v];
            b = y + Table_fu2[u];
            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);
            *rgb565++ = ((r & 0xf8) | ((g & 0xe0) >> 5));
            *rgb565++ = (((g & 0x1c) << 3) | (b & 0xf8) >> 3);
        }
    }

#if TIME_DEBUG
    u32 time2 = timer_get_ms();
    printf("[msg]>>>>>>>>>>>time2-time1=%d", time2 - time1);
#endif
}

void yuyv422_soft_rgb565_1(u8 *in_yuyv422, u8 *out_rgb_565, int sw, int sh, int dw, int dh) sec(.volatile_ram_code)
{
#if TIME_DEBUG
    u32 time1 = timer_get_ms();
#endif
    float Qw = sw / dh;
    float Qh = sh / dw;
    u8 *yuyv422;
    u8 *rgb565;
    u8 y, u, v;
    int r, g, b;
    u16 i, j;
    static u8 count_flag = 0;
    yuyv422 = in_yuyv422;
    rgb565  = (u8 *)out_rgb_565;

    rgb565 = rgb565 + dh * dw;
    for (j = dh / 2; j < dh; j++) {
        for (i = 0; i < dw; i++) {
            y = yuyv422[(sw * (u16)(i * Qh) + (u16)(j * Qw)) * 2];
            u = yuyv422[(sw * (u16)(i * Qh) + (u16)(j * Qw) - (j % 2)) * 2 + 1];
            v = yuyv422[(sw * (u16)(i * Qh) + (u16)(j * Qw) - (j % 2)) * 2 + 3];
            r = y + Table_fv1[v];
            g = y - Table_fu1[u] - Table_fv2[v];
            b = y + Table_fu2[u];
            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);
            *rgb565++ = ((r & 0xf8) | ((g & 0xe0) >> 5));
            *rgb565++ = (((g & 0x1c) << 3) | (b & 0xf8) >> 3);
        }
    }

#if TIME_DEBUG
    u32 time2 = timer_get_ms();
    printf("[msg11]>>>>>>>>>>>time2-time1=%d", time2 - time1);
#endif
}

static u8 test_buf1[640 * 480 * 2];

static void rgb565_task1(void *p)
{
    while (1) {
        os_sem_pend(&hdl->rgb1, 0);
        yuyv422_soft_rgb565(hdl->get_frame, test_buf1, 640, 480, 320, 480);
        os_sem_post(&hdl->rgb1_ok);
    }
}

static void rgb565_task2(void *p)
{
    while (1) {
        os_sem_pend(&hdl->rgb2, 0);
        yuyv422_soft_rgb565_1(hdl->get_frame, test_buf1, 640, 480, 320, 480);
        os_sem_post(&hdl->rgb2_ok);
    }
}
static int spi_camera_open(void)
{
    //1.申请全局结构体内存
    hdl = ram_malloc(sizeof(struct spi_video));
    if (hdl == NULL) {
        printf("[error]>>>>>>>>>>>>malloc fail");
        goto exit;
    }
    os_sem_create(&hdl->rgb1, 0);
    os_sem_create(&hdl->rgb2, 0);
    os_sem_create(&hdl->rgb1_ok, 0);
    os_sem_create(&hdl->rgb2_ok, 0);

    hdl->buf = ram_malloc(LINE_LBUF_SIZE);
    if (!hdl->buf) {
        printf("err spi no buf \n");
        goto exit;
    }

    lbuf_handle = lib_system_lbuf_init(YUV422_LBUF_LEN + 128);
    if (!lbuf_handle) {
        printf("err lbuf_handle no buf \n");
        goto exit;
    }


    hdl->yuv422_buf = malloc(YUV422_LBUF_LEN);
    if (!hdl->buf) {
        printf("err spi no buf \n");
        goto exit;
    }

    hdl->recv_move_cnt = 0;
    hdl->frame_addr_move_cnt = 0;
    hdl->camera_reinit = 0;
    hdl->kill = 0;

    //这里需要注意顺序 需要先开摄像头 再开spi
    gpio_output_channle(CAM_XCLK_PORT, CH1_PLL_24M);
    /* spi_xclk_open(); */
    hdl->video = dev_open("video1.0", NULL);
    os_time_dly(20);//过滤异常数据包

    hdl->spi = dev_open(JL_SPI_NAME, NULL);
    if (!hdl->spi) {
        printf("spi open err \n");
        goto exit;
    } else {
        dev_ioctl(hdl->spi, IOCTL_SPI_SET_IRQ_FUNC, (u32)spi_irq_cb);
        JL_SPI->CON &= ~BIT(2);
        JL_SPI->CON &= ~BIT(11);
        JL_SPI->CON |= BIT(10);
        JL_SPI->CON |= BIT(12);//接收模式
        JL_SPI->CON |= BIT(13);//开启中断
        JL_SPI->CON |= BIT(1);//从机模式
        JL_SPI->CON |= BIT(0);//开启SPI
    }


//用于触发逻辑分析仪检测
#ifdef SPI_IO_DBG
    DBG_PORT->OUT ^= DBG_PORT_BIT;
    DBG_PORT->OUT ^= DBG_PORT_BIT;
    DBG_PORT->OUT ^= DBG_PORT_BIT;
    DBG_PORT->OUT ^= DBG_PORT_BIT;
#endif
    os_task_create(rgb565_task1, NULL, 10, 1024, 0, "rgb565_task1");
    os_task_create(rgb565_task2, NULL, 10, 1024, 0, "rgb565_task2");
    printf("camera open ok\n");
    return 0;
exit:
    return 0;
}

int spi_camera_close(void)
{
    recv_cnt = 2;

    extern void GC0310_in_sleep_mode(void);
    GC0310_in_sleep_mode();

    task_kill("rgb565_task1");
    task_kill("rgb565_task2");
    dev_close(hdl->spi);
    gpio_clear_output_channle(CAM_XCLK_PORT, CH1_PLL_24M);
    dev_close(hdl->video);
    os_sem_del(&hdl->rgb1, 0);
    os_sem_del(&hdl->rgb2, 0);
    os_sem_del(&hdl->rgb1_ok, 0);
    os_sem_del(&hdl->rgb2_ok, 0);
    free(hdl->yuv422_buf);
    lbuf_clear(lbuf_handle);
    free(lbuf_ptr);
    ram_free(hdl->buf);
    ram_free(hdl);


    /*printf("JL_SPI->CON == %x\n",JL_SPI->CON);*/


    return 0;
}

//该中断的所有数据均要在内部ram进行处理 在SDRAM可能概率收错
//并将将中断函数里面要尽可能精简 高效 行间隔只有100us只有 PCLK24M
//PCLK80M xclk40M FPS20fps 两线spi
static ___interrupt void spi_irq_cb(void) sec(.volatile_ram_code)
{
    u32 recv_len;
    JL_SPI->CON |= BIT(14);//清除中断标记
    u32 id = 0;

#ifdef SPI_IO_DBG
    DBG_PORT->OUT ^= DBG_PORT_BIT;
#endif

    //////////处理上一次收到的数据///////////
    if (recv_cnt == 1) { //开始新的一轮帧接收
        if (*(u32 *)hdl->irq_recv_buf != CAM_LSTART_VALUE) {
            goto recv_err;
        }
        hdl->cpy_addr = hdl->next_frame_addr;
        hdl->real_data_addr = hdl->irq_last_recv_buf + CAM_FHEAD_SIZE;
        /* 告诉lbuf这个数据包完整了  */
        /* id = dma_copy_async(hdl->cpy_addr, hdl->real_data_addr, CAM_LINE_SIZE); */
        /* dma_copy_async_wait(id);//硬件DMA的要等待DMA是否完成 */
        lbuf_push(hdl->next_frame_addr, BIT(0));//把数据块推送更新到lbuf的通道0
        /* 开始行的一轮buf */
        if (lbuf_free_space(lbuf_handle) < YUV422_PACK_LEN) { //查询LBUF空闲数据块是否有足够长度
            //如果内存不足直接读一个buf块释放
            hdl->next_frame_addr = lbuf_pop(lbuf_handle, BIT(0));//从lbuf的通道0读取数据块
            lbuf_free(hdl->next_frame_addr);
        }
        hdl->next_frame_addr = lbuf_alloc(lbuf_handle, YUV422_PACK_LEN); //lbuf内申请一块空间
    } else if (recv_cnt == 2) { //只有第一行有帧头 取余都是去行头

        if (*(u32 *)hdl->irq_last_recv_buf != CAM_VSTART_VALUE) {//校验数据
            goto recv_err;
        }

        hdl->cpy_addr = hdl->next_frame_addr;
        hdl->real_data_addr = hdl->irq_last_recv_buf + CAM_FHEAD_SIZE + CAM_LHEAD_SIZE;
    } else {
        hdl->cpy_addr = hdl->cpy_addr + CAM_LINE_SIZE;
        hdl->real_data_addr = hdl->irq_last_recv_buf + CAM_LHEAD_SIZE;
    }

    //使用DMA拷贝数据减缓CPU开销
    //使用行缓存拷贝需要十分注意一点 需要确保行间隔足够长 不然会追尾
    //使用行缓存拷贝会存在一个问题 就是图像模糊
    //dma 拷贝速度 128us 拷贝 1280个数据极限速度
    //这个地方使用了DMA拷贝别的地方就不能使用只有一个DMA拷贝模块
    dma_copy_async(hdl->cpy_addr, hdl->real_data_addr, CAM_LINE_SIZE);

    //////////处理下一次数据的接收///////////
    //行缓存
    hdl->irq_recv_buf = hdl->buf + hdl->recv_move_cnt * DATA_FIRST_PACK_SIZE;
    hdl->recv_move_cnt = ++hdl->recv_move_cnt % RECV_DATA_LINE_PACK_TIME;
    if (recv_cnt == 1) {            //接收第1行
        JL_SPI->CNT = DATA_FIRST_PACK_SIZE;
    } else if (recv_cnt < CAMERA_H) { //接收2~479行
        JL_SPI->CNT = DATA_LINE_PACK_SIZE;
    } else if (recv_cnt == CAMERA_H) { //接收480行
        JL_SPI->CNT =  DATA_END_PACK_SIZE;
    }// 0-479 一共480行
    JL_SPI->ADR = hdl->irq_recv_buf;

    //保存一下当前接收地址下一次进中断就能知道上一次的接收地址
    hdl->irq_last_recv_buf = hdl->irq_recv_buf;

    recv_cnt = ++recv_cnt % (CAMERA_H + 1); //0~481循环
    if (recv_cnt == 0) { //去除0 从1开始计数 就是1~480循环
        recv_cnt = 1;
    }
    return;

recv_err:
    hdl->camera_reinit = 1;
}


static void spi_video_task(void *p)
{
start:
    debug_io_init();
    if (spi_camera_open()) {
        goto exit;
    }

    hdl->irq_recv_buf = hdl->buf;
    hdl->irq_last_recv_buf = hdl->irq_recv_buf;
    //第一次buf使用可以不用检测里面有无内存 肯定是有的 刚刚申请
    hdl->next_frame_addr = lbuf_alloc(lbuf_handle, YUV422_PACK_LEN); //lbuf内申请一块空间

    JL_SPI->ADR = hdl->irq_recv_buf;
    JL_SPI->CNT = DATA_FIRST_PACK_SIZE;

    while (1) {
        if (hdl->camera_reinit) {
            goto restart;
        }
        if (hdl->kill) {
            goto exit;
        }
        if (!lbuf_empty(lbuf_handle)) { //查询LBUF内是否有数据帧
            hdl->get_frame = lbuf_pop(lbuf_handle, BIT(0));//从lbuf的通道0读取数据块
            lcd_interface_non_block_wait();
            os_sem_post(&hdl->rgb1);
            os_sem_post(&hdl->rgb2);
            os_sem_pend(&hdl->rgb1_ok, 0);
            os_sem_pend(&hdl->rgb2_ok, 0);
            lcd_show_frame_to_dev(test_buf1, 320 * 480 * 2);
            Calculation_frame();
            lbuf_free(hdl->get_frame);
        }
    }
restart:
    spi_camera_close();
    hdl->camera_reinit = 0;
    goto start;
exit:
    printf("\n [msg] %s -[kill_spi_video] %d\n", __FUNCTION__, __LINE__);
    spi_camera_close();
    hdl->kill = 0;
    while (1) { //等待其他任务杀此任务
        os_time_dly(10);//
    }
}

int spi_video_task_kill(void)
{
    set_lcd_show_data_mode(UI);
    printf("---> spi_video_task_kill\n");
    hdl->kill = 1;
    while (hdl->kill) {
        os_time_dly(10);
    }
    task_kill("spi_video_task");

    return 0;
}

int init_spi_video(void)
{
    set_lcd_show_data_mode(CAMERA);
    os_task_create(spi_video_task, NULL, 1, 1024, 0, "spi_video_task");
    return 0;
}

static int reinit_spi_video(void)
{
    spi_video_task_kill();
    os_task_create(spi_video_task, NULL, 1, 1024, 0, "spi_video_task");
    return 0;
}


static int spi_video_task_init(void)
{
    os_time_dly(100);
    os_time_dly(100);
    os_time_dly(100);
    os_time_dly(100);
    os_time_dly(100);
    for (u8 i = 0; i < 5; i++) {
        os_time_dly(500);
        spi_video_task_kill();
        os_time_dly(300);
        os_task_create(spi_video_task, NULL, 1, 1024, 0, "spi_video_task");
    }
    os_time_dly(200);
    spi_video_task_kill();
    printf("\n [msg] %s -[test_end] %d\n", __FUNCTION__, __LINE__);
    while (1) {
        os_time_dly(100);
    }
    return 0;
}

static void spi_video_open_close_test(void *p)
{
    spi_video_task_init();
}
static int c_main(void)
{
    /******ui_lcd_一起初始化数据***********/
    extern void user_ui_lcd_init(void);
    user_ui_lcd_init();
    lcd_bl_pinstate(BL_ON);
    /*****默认为UI模式*****/
    set_lcd_show_data_mode(CAMERA);

    os_task_create(spi_video_task, NULL, 1, 1024, 0, "spi_video_task");
    /*os_task_create(spi_video_open_close_test, NULL, 1, 1024, 0, "spi_video_open_close_test"); */

    return 0;
}

late_initcall(c_main);
#endif




