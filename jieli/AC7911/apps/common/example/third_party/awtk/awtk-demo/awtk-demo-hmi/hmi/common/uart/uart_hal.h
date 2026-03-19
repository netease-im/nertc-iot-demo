/**
 * File:   uart_hal.h
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


#ifndef TK_USART_H
#define TK_USART_H

#include "tkc/ring_buffer.h"

BEGIN_C_DECLS

/**
 * @method uart_open
 * @param {const char*} name 串口名称，如"1"，"2"，"3"，"4"，"5"，"6"，"7"，"8"。
 * @param {int} baudrate 波特率。
 * @return {int} 串口句柄。
*/
int uart_open(const char *name, int baudrate);

/**
 * @method uart_read
 * 读串口。
 * @param {int} fd 串口句柄。
 * @param {void*} buff 读取缓冲区。
 * @param {uint32_t} nbytes 读取字节数。
 * @return {int} 实际读取字节数。
*/
int uart_read(int fd, void *buff, uint32_t nbytes);

/**
 * @method uart_has_data
 * 串口是否有数据。
 * @param {int} fd 串口句柄。
 * @return {bool_t} TRUE表示有数据，FALSE表示没有数据。
*/
bool_t uart_has_data(int fd);

/**
 * @method uart_write
 * 写串口。
 * @param {int} fd 串口句柄。
 * @param {const void*} buff 写入缓冲区。
 * @param {uint32_t} nbytes 写入字节数。
 * @return {int} 实际写入字节数。
*/
int uart_write(int fd, const void *buff, uint32_t nbytes);

/**
 * @method uart_close
 * 关闭串口。
 * @param {int} fd 串口句柄。
 * @return {int} 0表示成功，-1表示失败。
*/
int uart_close(int fd);

int uart_debug(const char *format, ...);
void uart_test(const char *name);

END_C_DECLS

#endif/*TK_USART_H*/
