/**
 * File:   uart_hal.c
 * Author: AWTK Develop Team
 * Brief:  uart_hal
 *
 * Copyright (c) 2023 - 2023  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2023-11-19 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "init.h"
#include "device/uart.h"

#include "tkc/utils.h"
#include "tkc/platform.h"
#include "uart_hal.h"

/*========================================================*/

static u8 loop_buf[1 * 1024] __attribute__((aligned(32))); //用于串口接收缓存数据的循环buf

int uart_open(const char *name, int baudrate)
{
    printf("AWTK HMI uart_hal uart_open %s %d \r\n", name, baudrate);

    void *uart_hdl = dev_open(name, NULL);
    return_value_if_fail(uart_hdl != NULL, -1);

    dev_ioctl(uart_hdl, UART_SET_BAUDRATE, (u32)baudrate);
    /* 设置串口接收缓存数据的循环buf地址 */
    dev_ioctl(uart_hdl, UART_SET_CIRCULAR_BUFF_ADDR, (u32)loop_buf);
    /* 设置串口接收缓存数据的循环buf长度 */
    dev_ioctl(uart_hdl, UART_SET_CIRCULAR_BUFF_LENTH, (u32)sizeof(loop_buf));
    /* 设置接收数据为阻塞方式,需要非阻塞可以去掉,建议加上超时设置 */
    /*dev_ioctl(uart_hdl, UART_SET_RECV_BLOCK, (u32)1);*/
    /* 设置接收数据超时时间，单位ms */
    /*dev_ioctl(uart_hdl, UART_SET_RECV_TIMEOUT, (u32)0);*/

    return (int)uart_hdl;
}

int uart_close(int fd)
{
    dev_close((void *)fd);
    return 0;
}

int uart_read(int fd, void *buffer, uint32_t size)
{
    return (int)dev_read((void *)fd, buffer, size);
}

bool_t uart_has_data(int fd)
{
    u32 len;
    dev_ioctl((void *)fd, UART_GET_RECV_CNT, (u32)&len);
    return len > 0 ? TRUE : FALSE;
}

int uart_write(int fd, const void *buffer, uint32_t size)
{
    return dev_write((void *)fd, (void *)buffer, size);
}

//////////////////////////////////////////////////////////////////////////////////////
static void uart_hal_test_thread(void *p)
{
    int len;
    int fd = uart_open("uart1", 115200);
    uint8_t buff[128];

    if (fd > 0) {
        while (1) {
            len = uart_read(fd, buff, sizeof(buff));
            if (len > 0) {
                uart_write(fd, buff, len);
            } else {
                sleep_ms(10);
            }
        }
        uart_close(fd);
    } else {
        puts("uart_hal_test_thread open failed\r\n");
    }
}

static int c_main(void)
{
    os_task_create(uart_hal_test_thread, NULL, 10, 1000, 0, "uart_hal_test_thread");
    return 0;
}
/*late_initcall(c_main);*/

