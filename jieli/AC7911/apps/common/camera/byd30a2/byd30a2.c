#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "byd30a2.h"
#include "gpio.h"
#include "generic/jiffies.h"
#include "device/iic.h"
#include "device/device.h"
#include "app_config.h"

#ifdef CONFIG_VIDEO_ENABLE

/*
 * #if (CONFIG_VIDEO_IMAGE_W > 240)
 * #define BYD30A2_DEVP_INPUT_W     128
 * #define BYD30A2_DEVP_INPUT_H     160
 * #else
 * #define BYD30A2_DEVP_INPUT_W     CONFIG_VIDEO_IMAGE_W
 * #define BYD30A2_DEVP_INPUT_H     CONFIG_VIDEO_IMAGE_H
 * #endif
 */

#define BYD30A2_DEVP_INPUT_W        240
#define BYD30A2_DEVP_INPUT_H	    320

#define BYD30A2_WRCMD 0xdc
#define BYD30A2_RDCMD 0xdd

#define CONFIG_INPUT_FPS	15//30

#define DELAY_TIME	10


static void *iic = NULL;
static u8 BYD30A2_reset_io[2] = {-1, -1};
static u8 BYD30A2_power_io[2] = {-1, -1};

struct reginfo {
    u8 reg;
    u8 val;
};

static const struct reginfo sensor_init_data[] = {
    0xf2, 0x01, ///< 复位所有寄存器
    0x15, 0x80,
    0x6b, 0x73,
    0x04, 0x00,
    0x06, 0x26,
    0x08, 0x07,
    0x1c, 0x12,
    0x1e, 0x26,
    0x1f, 0x01,
    0x20, 0x20,
    0x21, 0x20,
    0x34, 0x02,
    0x35, 0x02,
    0x36, 0x21,
    0x37, 0x13,
#if (CONFIG_INPUT_FPS == 15)
    0xca, 0x02, ///< 15ps
#else
    0xca, 0x03, ///< 30ps
#endif

// 分辨率设置。注意：END_ADDRW <= 248 并且 END_ADDRH <= 328
#define START_ADDRW 0
#define START_ADDRH 0
#define END_ADDRW (BYD30A2_DEVP_INPUT_W + START_ADDRW)
#define END_ADDRH (BYD30A2_DEVP_INPUT_H + START_ADDRH)

    0x17, START_ADDRW,
    0x18, END_ADDRW,
    0x19, START_ADDRH,
    0x1a, (END_ADDRH >> 1) & 0xFF,
    /* 0x12, 0x13 | ((END_ADDRH & 0x1) << 7), ///< output mode: only Y  */
    0x12, 0x10 | ((END_ADDRH & 0x1) << 7), ///< output mode: YUV422

    0xcb, 0x22,
    0xcc, 0x89,
    0xcd, 0x4c,
    0xce, 0x6b,
    0xcf, 0x90,
    0xa0, 0x8e,
    0x01, 0x1b,
    0x02, 0x1d,
    0x13, 0x08,
    0x87, 0x13,
    0x8a, 0x33,
    0x8b, 0x08,
    0x70, 0x1f,
    0x71, 0x40,
    0x72, 0x0a,
    0x73, 0x62,
    0x74, 0xa2,
    0x75, 0xbf,
    0x76, 0x02,
    0x77, 0xcc,
    0x40, 0x32,
    0x41, 0x28,
    0x42, 0x26,
    0x43, 0x1d,
    0x44, 0x1a,
    0x45, 0x14,
    0x46, 0x11,
    0x47, 0x0f,
    0x48, 0x0e,
    0x49, 0x0d,
    0x4B, 0x0c,
    0x4C, 0x0b,
    0x4E, 0x0a,
    0x4F, 0x09,
    0x50, 0x09,
    0x24, 0x50,
    0x25, 0x36,
    0x80, 0x00,
    0x81, 0x20,
    0x82, 0x40,
    0x83, 0x30,
    0x84, 0x50,
    0x85, 0x30,
    0x86, 0xD8,
    0x89, 0x45,
    0x8f, 0x81,
    0x91, 0xff,
    0x92, 0x08,
    0x94, 0x82,
    0x95, 0xfd,
    0x9a, 0x20,
    0x9e, 0xbc,
    0xf0, 0x83,///< 动态帧率关闭 0x8f打开
    0x51, 0x06,
    0x52, 0x25,
    0x53, 0x2b,
    0x54, 0x0F,
    0x57, 0x2A,
    0x58, 0x22,
    0x59, 0x2c,
    0x23, 0x33,
    0xa0, 0x8f,
    0xa1, 0x93,
    0xa2, 0x0f,
    0xa3, 0x2a,
    0xa4, 0x08,
    0xa5, 0x26,
    0xa7, 0x80,
    0xa8, 0x80,
    0xa9, 0x1e,
    0xaa, 0x19,
    0xab, 0x18,
    0xae, 0x50,
    0xaf, 0x04,
    0xc8, 0x10,
    0xc9, 0x15,
    0xd3, 0x0c,
    0xd4, 0x16,
    0xee, 0x06,
    0xef, 0x04,
    0x55, 0x34,
    0x56, 0x9c,
    0xb1, 0x98,
    0xb2, 0x98,
    0xb3, 0xc4,
    0xb4, 0x0C,
    0x00, 0x40,
    0x13, 0x07,
};

static u8 wrBYD30A2Reg(u8 regID, u8 regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, BYD30A2_WRCMD)) {
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

static u8 rdBYD30A2Reg(u8 regID, u8 *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, BYD30A2_WRCMD)) {
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
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, BYD30A2_RDCMD)) {
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

static void BYD30A2_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    int i;
    for (i = 0; i < sizeof(sensor_init_data) / sizeof(sensor_init_data[0]); i++) {
        wrBYD30A2Reg(sensor_init_data[i].reg, sensor_init_data[i].val);
    }
}

static s32 BYD30A2_set_output_size(u16 *width, u16 *height, u8 *freq)
{
    u16 liv_width = *width;
    u16 liv_height = *height;

    return 0;
}

static s32 BYD30A2_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

static s32 BYD30A2_ID_check(void)
{
    int ret;
    u16 pid = 0x00;
    u8 id0, id1;
    ret = rdBYD30A2Reg(0xfc, &id0);
    rdBYD30A2Reg(0xfd, &id1);
    pid = (u16)((id0 << 8) | id1);
    printf("BYD30A2 Sensor ID : 0x%x\n", pid);
    if (pid != 0x3b02) {
        return -1;
    }

    return 0;
}

static void BYD30A2_powerio_ctl(u32 _power_gpio, u32 on_off)
{
    gpio_direction_output(_power_gpio, on_off);
}

static void BYD30A2_reset(u8 isp_dev)
{
    u8 res_io;
    u8 powd_io;
    u8 id = 0;
    puts("BYD30A2 reset\n");

    if (isp_dev == ISP_DEV_0) {
        res_io = (u8)BYD30A2_reset_io[0];
        powd_io = (u8)BYD30A2_power_io[0];
    } else {
        res_io = (u8)BYD30A2_reset_io[1];
        powd_io = (u8)BYD30A2_power_io[1];
    }

    if (powd_io != (u8) - 1) {
        BYD30A2_powerio_ctl((u32)powd_io, 0);
    }
    if (res_io != (u8) - 1) {
        gpio_direction_output(res_io, 1);
        gpio_direction_output(res_io, 0);
        os_time_dly(1);
        gpio_direction_output(res_io, 1);
    }
}

static s32 BYD30A2_check(u8 isp_dev, u32 _reset_gpio, u32 _power_gpio)
{
    if (!iic) {
        if (isp_dev == ISP_DEV_0) {
            iic = dev_open("iic0", 0);
        } else {
            iic = dev_open("iic1", 0);
        }
        BYD30A2_reset_io[isp_dev] = (u8)_reset_gpio;
        BYD30A2_power_io[isp_dev] = (u8)_power_gpio;
    }
    if (iic == NULL) {
        printf("BYD30A2 iic open err!!!\n\n");
        return -1;
    }
    BYD30A2_reset(isp_dev);

    if (0 != BYD30A2_ID_check()) {
        dev_close(iic);
        iic = NULL;
        printf("-------not BYD30A2------\n\n");
        return -1;
    }
    printf("-------hello BYD30A2------\n\n");
    return 0;
}

static s32 BYD30A2_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n BYD30A2 \n\n");
    if (0 != BYD30A2_check(isp_dev, 0, 0)) {
        return -1;
    }
    BYD30A2_config_SENSOR(width, height, format, frame_freq);

    return 0;
}

void set_rev_sensor_BYD30A2(u16 rev_flag)
{
    if (!rev_flag) {
        wrBYD30A2Reg(0x1e, 0x30);
    } else {
        wrBYD30A2Reg(0x1e, 0x00);
    }
}

u16 BYD30A2_dvp_rd_reg(u16 addr)
{
    u8 val;
    rdBYD30A2Reg((u8)addr, &val);
    return val;
}

void BYD30A2_dvp_wr_reg(u16 addr, u16 val)
{
    wrBYD30A2Reg((u8)addr, (u8)val);
}

// *INDENT-OFF*
// BYD30A2为SPI摄像头
/* REGISTER_CAMERA1(BYD30A2) = { */
REGISTER_CAMERA(BYD30A2) = {
    .logo 				= 	"BYD30A2",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_YUYV,

    .mbus_type          =   SEN_MBUS_BT656,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_1B | \
                            SEN_MBUS_PCLK_SAMPLE_FALLING,
    .sync_config		=   0,
    .fps         		= 	CONFIG_INPUT_FPS,
    .out_fps			=   CONFIG_INPUT_FPS,
    .sen_size 			= 	{BYD30A2_DEVP_INPUT_W, BYD30A2_DEVP_INPUT_H},
    .cap_fps         	= 	CONFIG_INPUT_FPS,
    .sen_cap_size 		= 	{BYD30A2_DEVP_INPUT_W, BYD30A2_DEVP_INPUT_H},
    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	BYD30A2_check,
        .init 		        = 	BYD30A2_init,
        .set_size_fps 		=	BYD30A2_set_output_size,
        .power_ctrl         =   BYD30A2_power_ctl,
        .sleep 		        =	NULL,
        .wakeup 		    =	NULL,
        .write_reg 		    =	BYD30A2_dvp_wr_reg,
        .read_reg 		    =	BYD30A2_dvp_rd_reg,
        .set_sensor_reverse =   set_rev_sensor_BYD30A2,
    }
};

#endif

/*
 * // 参考板级配置如下
 * static const struct camera_platform_data camera0_data = {
 *     .xclk_gpio      = IO_PORTH_02,
 *     .reset_gpio     = IO_PORTH_03,
 *     .online_detect  = NULL,
 *     .pwdn_gpio      = -1,
 *     .power_value    = 0,
 *     .interface      = SEN_INTERFACE0,
 *     .dvp={
 *         .pclk_gpio   = IO_PORTA_08,
 *         .hsync_gpio  = IO_PORTA_09,
 *         .vsync_gpio  = IO_PORTA_10,
 *         .group_port  = ISC_GROUPA,
 *         .data_gpio   = {
 *                 -1,//IO_PORTA_07,
 *                 -1,//IO_PORTA_06,
 *                 -1,//IO_PORTA_05,
 *                 -1,//IO_PORTA_04,
 *                 -1,//IO_PORTA_03,
 *                 -1,//IO_PORTA_02,
 *                 IO_PORTA_01,
 *                 -1,//IO_PORTA_00,
 *                 -1,
 *                 -1,
 *         },
 *     }
 * };
 */
