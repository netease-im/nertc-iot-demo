#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "asm/spi.h"
#include "device/gpio.h"
#include "lcd_drive.h"

#ifdef USE_SPI_2BIT_TSET_DEMO

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
            printf("\n [MSG]fps_count = %d\n", fps_cnt *  1000 / tdiff);
            tstart = 0;
            fps_cnt = 0;
        }
    }
}

static struct spi_2bit_data spi_2data;
#define LCD_CS 		IO_PORTA_00
#define LCD_RS 		IO_PORTA_02
#define LCD_RESET   IO_PORTA_01

static u8 data[320 * 240 * 2] ALIGNE(32) ;

static void *spi_test_hdl = NULL;

static u8 lcd_reset(void)
{
    gpio_direction_output(LCD_RESET, 1);
    os_time_dly(1);
    gpio_direction_output(LCD_RESET, 0);
    os_time_dly(1);
    gpio_direction_output(LCD_RESET, 1);
    gpio_direction_output(LCD_CS, 1);
    gpio_direction_output(LCD_RS, 1);
    os_time_dly(1);
}

static u8 lcd_send_cmd(u8 cmd)
{
    gpio_direction_output(LCD_CS, 0);
    gpio_direction_output(LCD_RS, 0);
    dev_write(spi_test_hdl, &cmd, 1);//发送1bit使用该接口
    lcd_send_byte(cmd);
    gpio_direction_output(LCD_CS, 1);
}

static u8 lcd_read(void)
{
    u8 dat = 0;
    gpio_direction_output(LCD_CS, 0);
    gpio_direction_output(LCD_RS, 1);
    dev_read(spi_test_hdl, &dat, 1);//发送1bit使用该接口
    gpio_direction_output(LCD_CS, 1);
    return dat;
}

static void spi_test_task(void *p)
{

    printf("\n [SPI_2BIT_MODE_TASK] %s,  %d\n", __FUNCTION__, __LINE__);

    //1.打开spi设备
    spi_test_hdl = dev_open("spi2", NULL);
    if (!spi_test_hdl) {
        printf("spi open err \n");
        goto exit;
    }

    u32 len = 320 * 240 * 2;
    memset(data, 0xAA, 320 * 240 * 2);

#if 1
    lcd_reset();

    lcd_send_cmd(0x04);
    u8 get_id = lcd_read();
    printf(">>>>>get_id = %d", get_id);

    get_id = lcd_read();
    printf(">>>>>get_id = %d", get_id);
#endif

    while (1) {

        os_time_dly(20);
#if 1
        dev_write(spi_test_hdl, data, 1);//发送1bit使用该接口
        dev_write(spi_test_hdl, data, 1);//发送1bit使用该接口
        dev_write(spi_test_hdl, data, 1);//发送1bit使用该接口
        spi_2data.len = len;
        spi_2data.buf = data;

        dev_ioctl(spi_test_hdl, IOCTL_SPI_SEND_2BYTE, &spi_2data);
        Calculation_frame();
        os_time_dly(1);
#endif
        /* yield();//clear sys dog */
    }

exit:
    if (spi_test_hdl) { //关闭spi设备
        dev_close(spi_test_hdl);
    }
    while (1) { //等待其他任务杀此任务
        os_time_dly(100);
    }
}

static int c_main(void)
{
    os_task_create(spi_test_task, NULL, 22, 1000, 0, "spi_test_task");
    return 0;
}
late_initcall(c_main);

#endif
