/**
 * File:   serial_helper.c
 * Author: AWTK Develop Team
 * Brief:  serial helper functions
 *
 * Copyright (c) 2019 - 2023  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2019-09-11 Li XianJing <xianjimli@hotmail.com> created
 *
 */

/*
 *the following code adapted from: https://github.com/wjwwood/serial/tree/master/src/impl
 *
 * Copyright 2012 William Woodall and John Harrison
 *
 * Additional Contributors: Christopher Baker @bakercp
 */
#include "tkc/mem.h"
#include "tkc/wstr.h"
#include "tkc/thread.h"
#include "tkc/time_now.h"
#include "tkc/serial_helper.h"

#ifdef TK_IS_PC
#include "tkc/socket_pair.h"
#include "tkc/socket_helper.h"
#endif/*TK_IS_PC*/

#if 0

serial_handle_t serial_open(const char *port)
{
    assert(0);
    return NULL;
}
int serial_close(serial_handle_t handle)
{
    assert(0);
    return -1;
}
ret_t serial_wait_for_data(serial_handle_t handle, uint32_t timeout_ms)
{
    assert(0);
    return RET_FAIL;
}
bytesize_t serial_bytesize_from_str(const char *str)
{
    assert(0);
    return eightbits;
}
stopbits_t serial_stopbits_from_str(const char *str)
{
    assert(0);
    return stopbits_one;
}
flowcontrol_t serial_flowcontrol_from_str(const char *str)
{
    assert(0);
    return flowcontrol_none;
}
parity_t serial_parity_from_str(const char *str)
{
    assert(0);
    return parity_none;
}
int serial_handle_get_fd(serial_handle_t handle)
{
    assert(0);
    return NULL;
}
ret_t serial_config(serial_handle_t handle, uint32_t baudrate, bytesize_t bytesize,
                    stopbits_t stopbits, flowcontrol_t flowcontrol, parity_t parity)
{
    assert(0);
    return RET_NOT_IMPL;
}
ret_t serial_oflush(serial_handle_t handle)
{
    assert(0);
    return RET_NOT_IMPL;
}
#endif
