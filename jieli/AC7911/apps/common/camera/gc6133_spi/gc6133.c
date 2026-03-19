#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "generic/jiffies.h"
#include "device/iic.h"
#include "device/device.h"
#include "app_config.h"

#ifdef CONFIG_VIDEO_ENABLE

#define GC6133_DEVP_MAX_W	244
#define GC6133_DEVP_MAX_H	328

#if (CONFIG_VIDEO_IMAGE_W > GC6133_DEVP_MAX_W)
#define GC6133_DEVP_INPUT_W GC6133_DEVP_MAX_W
#define GC6133_DEVP_INPUT_H GC6133_DEVP_MAX_H
#else
#define GC6133_DEVP_INPUT_W CONFIG_VIDEO_IMAGE_W
#define GC6133_DEVP_INPUT_H CONFIG_VIDEO_IMAGE_H
#endif

#ifdef CONFIG_VIDEO1_ENABLE /*开启video1则使用video1分辨率*/
#undef GC6133_DEVP_INPUT_W
#undef GC6133_DEVP_INPUT_H
#ifdef CONFIG_VIDEO1_IMAGE_W
#define GC6133_DEVP_INPUT_W CONFIG_VIDEO1_IMAGE_W
#define GC6133_DEVP_INPUT_H CONFIG_VIDEO1_IMAGE_H
#else
#define GC6133_DEVP_INPUT_W 240
#define GC6133_DEVP_INPUT_H 320
#endif
#endif

#define CONFIG_INPUT_FPS 	12

static void *iic = NULL;
static u8 GC6133_reset_io[2] = {-1, -1};
static u8 GC6133_power_io[2] = {-1, -1};

#define GC6133_WRCMD 0x80
#define GC6133_RDCMD 0x81

#define GC6133_1WIRE_MODE	1

#define DELAY_TIME	10
static s32 GC6133_set_output_size(u16 *width, u16 *height, u8 *freq);

struct reginfo {
    u8 reg;
    u8 val;
};
static const struct reginfo sensor_init_data[] = {
    /////////////////////////////////////////////////////
    //////////////////////   SYS   //////////////////////
    /////////////////////////////////////////////////////
    {0xfe, 0xa0},
    {0xfe, 0xa0},
    {0xfe, 0xa0},
    {0xf1, 0x03}, //output enable
    {0xf6, 0x00},
    {0xfa, 0x10},
    {0xfc, 0x12}, //clock enable
    {0xfe, 0x00}, //Page select
    ////////////////////////////////////////////////////
    ///////////////   ANALOG & CISCTL   ////////////////
    ////////////////////////////////////////////////////
    {0x0d, 0x30},
    {0x12, 0xc8},
    {0x14, 0x54}, //dark CFA
    {0x15, 0x32}, //1:sdark 0:ndark
    {0x16, 0x04},
    {0x17, 0x19},
    {0x1d, 0xb9},
    {0x7a, 0x00},
    {0x7b, 0x14},
    {0x7d, 0x36},
    ////////////////////////////////////////////////////
    /////////////////////   ISP   //////////////////////
    ////////////////////////////////////////////////////
    {0x20, 0x7e},
    {0x22, 0xf0},
    {0x24, 0x54}, //output_format
    {0x26, 0x87}, //[5]Y_switch [4]UV_switch [2]skin_en
    ////////////////////////////////////////////////////
    /////////////////////   BLK   //////////////////////
    ////////////////////////////////////////////////////
    {0x2a, 0x2f},
    {0x37, 0x46}, //[4:0]blk_select_row

    {0x39, ((GC6133_DEVP_MAX_H - GC6133_DEVP_INPUT_H) / 2 - 2) & 0xff},
    {0x3a, 0x80 | ((GC6133_DEVP_MAX_W - GC6133_DEVP_INPUT_W) / 2 - 2) & 0xff},

    {0x3b, (GC6133_DEVP_INPUT_H >> 8) & 0xff},
    {0x3c, GC6133_DEVP_INPUT_H & 0xff},
    {0x3e, GC6133_DEVP_INPUT_W & 0xff},
    ////////////////////////////////////////////////////
    /////////////////////   GAIN   /////////////////////
    ////////////////////////////////////////////////////
    {0x3f, 0x10}, //global gain
    ////////////////////////////////////////////////////
    /////////////////////   DNDD   /////////////////////
    ////////////////////////////////////////////////////
    {0x52, 0x4f},
    {0x53, 0x81},
    {0x54, 0x43},
    {0x56, 0x78},
    ////////////////////////////////////////////////////
    /////////////////////   ASDE   /////////////////////
    ////////////////////////////////////////////////////
    {0x5b, 0x60}, //dd&ee th
    {0x5c, 0x80}, //60/OT_th
    ////////////////////////////////////////////////////
    ////////////////////   INTPEE   ////////////////////
    ////////////////////////////////////////////////////
    {0x63, 0x14}, //edge effect
    ////////////////////////////////////////////////////
    /////////////////////   CC   ///////////////////////
    ////////////////////////////////////////////////////
    {0x65, 0x13}, //Y
    {0x66, 0x26},
    {0x67, 0x07},
    {0x68, 0xf5}, //Cb
    {0x69, 0xea},
    {0x6a, 0x21},
    {0x6b, 0x21}, //Cr
    {0x6c, 0xe4},
    {0x6d, 0xfb},
    ////////////////////////////////////////////////////
    /////////////////////   YCP   //////////////////////
    ////////////////////////////////////////////////////
    {0x81, 0x30}, //cb
    {0x82, 0x30}, //cr
    {0x83, 0x48}, //luma contrast
    {0x8d, 0x78}, //edge dec sa
    {0x8e, 0x25}, //autogray
    ////////////////////////////////////////////////////
    /////////////////////   AEC   //////////////////////
    ////////////////////////////////////////////////////
    {0x90, 0x3c},
    {0x92, 0x40}, //target
    {0x9d, 0x65}, //flicker step
    {0x9e, 0x63}, //[7:4]margin
    {0x9f, 0xf2}, //exp_level 3f2
    {0xa3, 0x30}, //pregain
    {0xa4, 0x01},
    ////////////////////////////////////////////////////
    /////////////////////   AWB   //////////////////////
    ////////////////////////////////////////////////////
    {0xb1, 0x1e}, //Y_to_C_diff
    {0xb3, 0x20}, //C_max
    {0xbd, 0x70}, //R_limit
    {0xbe, 0x58}, //G_limit
    {0xbf, 0x80}, //B_limit
    ////////////////////////////////////////////////////
    ///////////////////   Banding   ////////////////////
    ////////////////////////////////////////////////////
    {0x03, 0x01},
    {0x04, 0x2f},
    {0x01, 0x02}, //hb
    {0x02, 0x02}, //vb
    {0x0f, 0x01},
    ////////////////////////////////////////////////////
    /////////////////////   SPI   //////////////////////
    ////////////////////////////////////////////////////
    {0xfe, 0x02},
    {0x01, 0x00}, //spi disable
    {0x02, 0x80}, //ddr_mode
    {0x03, 0x20},
    {0x04, 0x30}, //[5]width x2 [3:0]frequency divider
    {0x09, 0x08},
    {0x0a, 0x00}, //yuv:00 raw8:02
    {0x13, 0x10}, //fifo_prog_full_level

    {0x20, 0xab},
    {0x21, 0x80},
    {0x22, 0x9d},
    {0x23, 0xb6},
    {0x24, 0x01}, //[1]sck_always [0]BT656
    {0x25, 0xff},
    {0x26, 0x00},
    {0x27, 0x00},
    {0x28, 0x03}, //clock_div_spi
    {0x01, 0x01}, //spi enable
    {0xfe, 0x00},
};

static unsigned char GC6133_wrReg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GC6133_WRCMD)) {
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

static unsigned char GC6133_rdReg(unsigned char regID, unsigned char *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GC6133_WRCMD)) {
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
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GC6133_RDCMD)) {
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

static void GC6133_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    int i;
    for (i = 0; i < sizeof(sensor_init_data) / sizeof(sensor_init_data[0]); i++) {
        GC6133_wrReg(sensor_init_data[i].reg, sensor_init_data[i].val);
    }
}
static s32 GC6133_set_output_size(u16 *width, u16 *height, u8 *freq)
{
    return 0;
}

static s32 GC6133_ID_check(void)
{
    u8 pid = 0;
    GC6133_rdReg(0xf0, &pid);
    delay(4000);
    GC6133_rdReg(0xf0, &pid);
    delay(4000);
    printf("GC6133 Sensor ID : 0x%x \n", pid);
    if (pid != 0xba) {
        return -1;
    }
    return 0;
}

void GC6133_powerio_ctl(u32 _power_gpio, u32 on_off)
{
    gpio_direction_output(_power_gpio, on_off);
}
static void GC6133_reset(u8 isp_dev)
{
    u8 res_io;
    u8 powd_io;
    u8 id = 0;
    puts("GC6133 reset\n");

    if (isp_dev == ISP_DEV_0) {
        res_io = GC6133_reset_io[0];
        powd_io = GC6133_power_io[0];
    } else {
        res_io = GC6133_reset_io[1];
        powd_io = GC6133_power_io[1];
    }

    if (powd_io != (u8) - 1) {
        GC6133_powerio_ctl(powd_io, 0);
    }

    if (res_io != (u8) - 1) {
        gpio_direction_output(res_io, 1);
        gpio_direction_output(res_io, 0);
        os_time_dly(1);
        gpio_direction_output(res_io, 1);
    }
}
void close_sensor2_iic()
{
    if (iic) {
        dev_close(iic);
        iic = NULL;
    }
}
s32 GC6133_check(u8 isp_dev, u32 _reset_gpio, u32 _power_gpio)
{
    if (!iic) {
        if (isp_dev == ISP_DEV_0) {
            iic = dev_open("iic0", 0);
        } else {
            iic = dev_open("iic1", 0);
        }
        GC6133_reset_io[isp_dev] = (u8)_reset_gpio;
        GC6133_power_io[isp_dev] = (u8)_power_gpio;
    }

    if (iic == NULL) {
        printf("GC6133 iic open err!!!\n\n");
        return -1;
    }
    GC6133_reset(isp_dev);
    if (0 != GC6133_ID_check()) {
        printf("-------not GC6133------\n\n");
        dev_close(iic);
        iic = NULL;
        return -1;
    }
    printf("-------hello GC6133------\n\n");
    return 0;
}


s32 GC6133_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n GC6133_init \n\n");

    if (0 != GC6133_check(isp_dev, 0, 0)) {
        return -1;
    }

    GC6133_config_SENSOR(width, height, format, frame_freq);

    return 0;
}
void set_rev_sensor_GC6133(u16 rev_flag)
{
    if (!rev_flag) {
        GC6133_wrReg(0x17, 0x17);
    } else {
        GC6133_wrReg(0x17, 0x14);
    }
}

u16 GC6133_dvp_rd_reg(u16 addr)
{
    u8 val;
    GC6133_rdReg((u8)addr, &val);
    return val;
}

void GC6133_dvp_wr_reg(u16 addr, u16 val)
{
    GC6133_wrReg((u8)addr, (u8)val);
}

// *INDENT-OFF*
REGISTER_CAMERA1(GC6133) = { //注意这里是SPI摄像头
    .logo 				= 	"GC6133",
    .isp_dev 			= 	ISP_DEV_1,//对应的IIC设备
    .in_format 			= 	SEN_IN_FORMAT_UYVY,
    .mbus_type          =   SEN_MBUS_BT656,
#if GC6133_1WIRE_MODE
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_1B | SEN_MBUS_PCLK_SAMPLE_FALLING,
#else
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_2B | SEN_MBUS_PCLK_SAMPLE_FALLING,
#endif
    .sync_config		=   0,//WL82/AC791才可以H-V SYNC互换，请留意
    .fps         		= 	CONFIG_INPUT_FPS,
    .out_fps			=   CONFIG_INPUT_FPS,
    .sen_size 			= 	{GC6133_DEVP_INPUT_W, GC6133_DEVP_INPUT_H},
    .cap_fps         	= 	CONFIG_INPUT_FPS,
    .sen_cap_size 		= 	{GC6133_DEVP_INPUT_W, GC6133_DEVP_INPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	GC6133_check,
        .init 		        = 	GC6133_init,
        .set_size_fps 		=	GC6133_set_output_size,
        .power_ctrl         =   NULL,


        .sleep 		        =	NULL,
        .wakeup 		    =	NULL,
        .write_reg 		    =	GC6133_dvp_wr_reg,
        .read_reg 		    =	GC6133_dvp_rd_reg,
        .set_sensor_reverse =   set_rev_sensor_GC6133,
    }
};

#endif

