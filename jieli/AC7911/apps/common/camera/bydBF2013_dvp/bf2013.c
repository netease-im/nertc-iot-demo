#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "bf2013.h"
#include "gpio.h"
#include "generic/jiffies.h"
#include "device/iic.h"
#include "device/device.h"
#include "app_config.h"

#ifdef CONFIG_VIDEO_ENABLE


static void *iic = NULL;
static u8 BF2013_reset_io[2] = {-1, -1};
static u8 BF2013_power_io[2] = {-1, -1};

#if (CONFIG_VIDEO_IMAGE_W > 640)
#define BF2013_DEVP_INPUT_W 	640
#define BF2013_DEVP_INPUT_H		480
#else
#define BF2013_DEVP_INPUT_W 	CONFIG_VIDEO_IMAGE_W
#define BF2013_DEVP_INPUT_H		CONFIG_VIDEO_IMAGE_H
#endif

#define BF2013_WRCMD  0xdc
#define BF2013_RDCMD  0xdd

#define CONFIG_INPUT_FPS	20 //0自动帧率 5/10/15/20/25固定帧率, 25帧需要关闭ppbuf, 加大实时流内存(cpu_config.h)

#define DELAY_TIME	10
static s32 BF2013_set_output_size(u16 *width, u16 *height, u8 *freq);
static unsigned char wrBF2013Reg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, BF2013_WRCMD)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(DELAY_TIME);
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(DELAY_TIME);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(DELAY_TIME);

exit:
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

static unsigned char rdBF2013Reg(unsigned char regID, unsigned char *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, BF2013_WRCMD)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(DELAY_TIME);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(DELAY_TIME);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, BF2013_RDCMD)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(DELAY_TIME);
    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);

exit:
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

static void BF2013_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    wrBF2013Reg(0x09, 0x02);
    wrBF2013Reg(0x15, 0x20);
    wrBF2013Reg(0x3a, 0x00);
    wrBF2013Reg(0x12, 0x00);
    wrBF2013Reg(0x1e, 0x00);
    wrBF2013Reg(0x13, 0x00);
    wrBF2013Reg(0x01, 0x14);
    wrBF2013Reg(0x02, 0x21);
    wrBF2013Reg(0x8c, 0x01);
    wrBF2013Reg(0x8d, 0xcb);
    wrBF2013Reg(0x87, 0x20);
    wrBF2013Reg(0x1b, 0x80);
    wrBF2013Reg(0x11, 0x80);
    wrBF2013Reg(0x2b, 0x20);
    wrBF2013Reg(0x92, 0x40);
    wrBF2013Reg(0x06, 0xe0);
    wrBF2013Reg(0x29, 0x54);
    wrBF2013Reg(0xeb, 0x30);
    wrBF2013Reg(0xbb, 0x20);
    wrBF2013Reg(0xf5, 0x21);
    wrBF2013Reg(0xe1, 0x3c);
    wrBF2013Reg(0x16, 0x01);
    wrBF2013Reg(0xe0, 0x0b);
    wrBF2013Reg(0x2f, 0xf6);
    wrBF2013Reg(0x1f, 0x20);
    wrBF2013Reg(0x22, 0x20);
    wrBF2013Reg(0x26, 0x20);
    wrBF2013Reg(0x33, 0x20);
    wrBF2013Reg(0x34, 0x08);
    wrBF2013Reg(0x35, 0x50);
    wrBF2013Reg(0x65, 0x4a);
    wrBF2013Reg(0x66, 0x48);
    wrBF2013Reg(0x36, 0x05);
    wrBF2013Reg(0x37, 0xf6);
    wrBF2013Reg(0x38, 0x46);
    wrBF2013Reg(0x9b, 0xf6);
    wrBF2013Reg(0x9c, 0x46);
    wrBF2013Reg(0xbc, 0x01);
    wrBF2013Reg(0xbd, 0xf6);
    wrBF2013Reg(0xbe, 0x46);
    wrBF2013Reg(0x70, 0x6f);
    wrBF2013Reg(0x72, 0x3f);
    wrBF2013Reg(0x73, 0x3f);
    wrBF2013Reg(0x74, 0x27);
    wrBF2013Reg(0x77, 0x90);
    wrBF2013Reg(0x79, 0x48);
    wrBF2013Reg(0x7a, 0x1e);
    wrBF2013Reg(0x7b, 0x30);
    wrBF2013Reg(0x24, 0x70);
    wrBF2013Reg(0x25, 0x80);
    wrBF2013Reg(0x80, 0x55);
    wrBF2013Reg(0x81, 0x02);
    wrBF2013Reg(0x82, 0x14);
    wrBF2013Reg(0x83, 0x23);
    wrBF2013Reg(0x9a, 0x23);
    wrBF2013Reg(0x84, 0x1a);
    wrBF2013Reg(0x85, 0x20);
    wrBF2013Reg(0x86, 0x30);
    wrBF2013Reg(0x89, 0x02);
    wrBF2013Reg(0x8a, 0x64);
    wrBF2013Reg(0x8b, 0x02);

    /*wrBF2013Reg(0x8e,0x07);*/ //11fps
    /*wrBF2013Reg(0x8f,0x79);*/

    wrBF2013Reg(0x8e, 0x02); // 20 fps
    wrBF2013Reg(0x8f, 0xdf); // 20 fps

    wrBF2013Reg(0x94, 0x0a);
    wrBF2013Reg(0x96, 0xa6);
    wrBF2013Reg(0x97, 0x0c);
    wrBF2013Reg(0x98, 0x18);
    wrBF2013Reg(0x9d, 0x93);
    wrBF2013Reg(0x9e, 0x7a);
    wrBF2013Reg(0x3b, 0x60);
    wrBF2013Reg(0x3c, 0x20);
    wrBF2013Reg(0x39, 0x80);
    wrBF2013Reg(0x3f, 0xb0);
    wrBF2013Reg(0x40, 0x9b);
    wrBF2013Reg(0x41, 0x88);
    wrBF2013Reg(0x42, 0x6e);
    wrBF2013Reg(0x43, 0x59);
    wrBF2013Reg(0x44, 0x4d);
    wrBF2013Reg(0x45, 0x45);
    wrBF2013Reg(0x46, 0x3e);
    wrBF2013Reg(0x47, 0x39);
    wrBF2013Reg(0x48, 0x35);
    wrBF2013Reg(0x49, 0x31);
    wrBF2013Reg(0x4b, 0x2e);
    wrBF2013Reg(0x4c, 0x2b);
    wrBF2013Reg(0x4e, 0x26);
    wrBF2013Reg(0x4f, 0x22);
    wrBF2013Reg(0x50, 0x1f);
    wrBF2013Reg(0x51, 0x05);
    wrBF2013Reg(0x52, 0x10);
    wrBF2013Reg(0x53, 0x0b);
    wrBF2013Reg(0x54, 0x15);
    wrBF2013Reg(0x57, 0x87);
    wrBF2013Reg(0x58, 0x72);
    wrBF2013Reg(0x59, 0x5f);
    wrBF2013Reg(0x5a, 0x7e);
    wrBF2013Reg(0x5b, 0x1f);
    wrBF2013Reg(0x5c, 0x0e);
    wrBF2013Reg(0x5d, 0x95);
    wrBF2013Reg(0x60, 0x24);
    wrBF2013Reg(0x6a, 0x01);
    wrBF2013Reg(0x23, 0x66);
    wrBF2013Reg(0xa0, 0x03);
    wrBF2013Reg(0xa1, 0x31);
    wrBF2013Reg(0xa2, 0x0e);
    wrBF2013Reg(0xa3, 0x27);
    wrBF2013Reg(0xa4, 0x08);
    wrBF2013Reg(0xa5, 0x25);
    wrBF2013Reg(0xa6, 0x06);
    wrBF2013Reg(0xa7, 0x80);
    wrBF2013Reg(0xa8, 0x7e);
    wrBF2013Reg(0xa9, 0x20);
    wrBF2013Reg(0xaa, 0x20);
    wrBF2013Reg(0xab, 0x20);
    wrBF2013Reg(0xac, 0x3c);
    wrBF2013Reg(0xad, 0xf0);
    wrBF2013Reg(0xae, 0x80);
    wrBF2013Reg(0xaf, 0x00);
    wrBF2013Reg(0xc5, 0x18);
    wrBF2013Reg(0xc6, 0x00);
    wrBF2013Reg(0xc7, 0x20);
    wrBF2013Reg(0xc8, 0x18);
    wrBF2013Reg(0xc9, 0x20);
    wrBF2013Reg(0xca, 0x17);
    wrBF2013Reg(0xcb, 0x1f);
    wrBF2013Reg(0xcc, 0x40);
    wrBF2013Reg(0xcd, 0x58);
    wrBF2013Reg(0xee, 0x4c);
    wrBF2013Reg(0xb0, 0xe0);
    wrBF2013Reg(0xb1, 0xc0);
    wrBF2013Reg(0xb2, 0xb0);
    wrBF2013Reg(0xb3, 0x88);
    wrBF2013Reg(0x56, 0x40);
    wrBF2013Reg(0x13, 0x07);

    /*wrBF2013Reg(0x17,0x01);*/
    /*wrBF2013Reg(0x18,0xA1);*/
    /*wrBF2013Reg(0x03,0xF0);*/
    /*wrBF2013Reg(0x19,0x00);*/
    /*wrBF2013Reg(0x1A,0x78);*/

}

static s32 BF2013_set_output_size(u16 *width, u16 *height, u8 *freq)
{
    u16 liv_width = *width + 8;
    u16 liv_height = *height + 8;

    wrBF2013Reg(0x09, liv_height >> 8);
    wrBF2013Reg(0x0a, liv_height & 0xff);
    wrBF2013Reg(0x0b, liv_width >> 8);
    wrBF2013Reg(0x0c, liv_width & 0xff);

    printf("BF2013 : %d , %d \n", *width, *height);
    return 0;
}

static s32 BF2013_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

static s32 BF2013_ID_check(void)
{
    u8 pid = 0x00;

    rdBF2013Reg(0xfc, &pid);
    if (pid != 0x37) {
        return -1;
    }
    rdBF2013Reg(0xfd, &pid);
    if (pid != 0x3) {
        return -1;
    }
    printf("BF2013 check ok");

    return 0;
}

static void BF2013_powerio_ctl(u32 _power_gpio, u32 on_off)
{
    gpio_direction_output(_power_gpio, on_off);
}
static void BF2013_reset(u8 isp_dev)
{
    u8 res_io;
    u8 powd_io;
    u8 id = 0;
    puts("BF2013 reset\n");

    if (isp_dev == ISP_DEV_0) {
        res_io = (u8)BF2013_reset_io[0];
        powd_io = (u8)BF2013_power_io[0];
    } else {
        res_io = (u8)BF2013_reset_io[1];
        powd_io = (u8)BF2013_power_io[1];
    }

    if (powd_io != (u8) - 1) {
        BF2013_powerio_ctl((u32)powd_io, 0);
    }
    if (res_io != (u8) - 1) {
        gpio_direction_output(res_io, 1);
        gpio_direction_output(res_io, 0);
        os_time_dly(1);
        gpio_direction_output(res_io, 1);
    }
}


static s32 BF2013_check(u8 isp_dev, u32 _reset_gpio, u32 _power_gpio)
{
    if (!iic) {
        /*if (isp_dev == ISP_DEV_0) {*/
        iic = dev_open("iic0", 0);
        /*} else {*/
        /*iic = dev_open("iic1", 0);*/
        /*}*/
        BF2013_reset_io[isp_dev] = (u8)_reset_gpio;
        BF2013_power_io[isp_dev] = (u8)_power_gpio;
    }
    if (iic == NULL) {
        printf("BF2013 iic open err!!!\n\n");
        return -1;
    }
    BF2013_reset(isp_dev);

    if (0 != BF2013_ID_check()) {
        printf("-------not BF2013------\n\n");
        dev_close(iic);
        iic = NULL;
        return -1;
    }
    printf("-------hello BF2013------\n\n");
    return 0;
}


static s32 BF2013_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n BF2013_init \n\n");

    gpio_direction_output(IO_PORTB_06, 1);
    gpio_direction_output(IO_PORTB_06, 0);
    gpio_direction_output(IO_PORTB_06, 1);
    gpio_direction_output(IO_PORTB_06, 0);
    if (0 != BF2013_check(isp_dev, 0, 0)) {
        return -1;
    }
    BF2013_config_SENSOR(width, height, format, frame_freq);

    return 0;
}
void set_rev_sensor_BF2013(u16 rev_flag)
{
    if (!rev_flag) {
        wrBF2013Reg(0x14, 0x13);
    } else {
        wrBF2013Reg(0x14, 0x10);
    }
}

u16 BF2013_dvp_rd_reg(u16 addr)
{
    u8 val;
    rdBF2013Reg((u8)addr, &val);
    return val;
}

void BF2013_dvp_wr_reg(u16 addr, u16 val)
{
    wrBF2013Reg((u8)addr, (u8)val);
}

// *INDENT-OFF*
REGISTER_CAMERA(BF2013) = {
    .logo 				= 	"BF2013",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_YUYV,
    .mbus_type          =   SEN_MBUS_PARALLEL,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_8B  | SEN_MBUS_HSYNC_ACTIVE_HIGH | \
    						SEN_MBUS_PCLK_SAMPLE_FALLING | SEN_MBUS_VSYNC_ACTIVE_HIGH,
#if CONFIG_CAMERA_H_V_EXCHANGE
    .sync_config		=   SEN_MBUS_SYNC0_VSYNC_SYNC1_HSYNC,//WL82/AC791才可以H-V SYNC互换，请留意
#endif
    .fps         		= 	CONFIG_INPUT_FPS,
    .out_fps			=   CONFIG_INPUT_FPS,
    .sen_size 			= 	{BF2013_DEVP_INPUT_W, BF2013_DEVP_INPUT_H},
    .cap_fps         	= 	CONFIG_INPUT_FPS,
    .sen_cap_size 		= 	{BF2013_DEVP_INPUT_W, BF2013_DEVP_INPUT_H},


    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	BF2013_check,
        .init 		        = 	BF2013_init,
        .set_size_fps 		=	BF2013_set_output_size,
        .power_ctrl         =   BF2013_power_ctl,


        .sleep 		        =	NULL,
        .wakeup 		    =	NULL,
        .write_reg 		    =	BF2013_dvp_wr_reg,
        .read_reg 		    =	BF2013_dvp_rd_reg,
        .set_sensor_reverse =   set_rev_sensor_BF2013,
    }
};

#endif
