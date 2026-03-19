#ifndef SOCKET_H
#define SOCKET_H
#include "string.h"
#include "stdio.h"
#include "includes.h"
#include "os/os_api.h"
#include "w5500.h"


#define socket_log			printf

#define SOCKET_MAX_NUM		8//最大socket数量
#define SOCKET_MAX_SIZE		1460//一次接收最大字节数
#define LOCAL_DEFAULT_PORT 	54321	//默认本地端口号

/* 错误类型 */
#ifndef ENONE
#define ENONE		0	//无措
#endif
#ifndef ETIMOUT
#define ETIMOUT		1	//超时
#endif
#ifndef EDISCON
#define EDISCON		2	//其他错误
#endif
#ifndef ENOMEM
#define ENOMEM		3	//内存不足
#endif
#ifndef EFAULT
#define EFAULT		4	//其他错误
#endif

typedef unsigned char u8_t, u8, sa_family_t;
typedef unsigned short u16_t, u16, in_port_t;
typedef unsigned int u32_t, u32, in_addr_t;
typedef unsigned long long u64_t, u64;

enum protocol_type {
    NONE_PROTOCOL = 0,
    TCP_PROTOCOL,//TCP协议
    UDP_PROTOCOL,//UDP协议
};

enum mode_type {
    NONE_MODE = 0,
    TCP_SERVER_MODE,//TCP服务器模式
    TCP_CLIENT_MODE,//TCP客户端模式
    UDP_BROADCAST_MODE,//UDP广播模式
};
typedef struct socket_protocol {
    /* 初始化只需要配置这3个参数即可 */
    unsigned char prot;//TCP_PROTOCOL 或者 UDP_PROTOCOL
    unsigned char mode;//TCP_SERVER_MODE 或者 TCP_CLIENT_MODE UDP_BROADCAST_MODE
    unsigned int time_out;//发送和接收超时，0永不超时，单位毫秒

    unsigned short local_port;//本地端口号
    unsigned short remote_port;//远程目标端口号
    unsigned int local_ip;//本地IP地址,可以使用inet_addr()转换赋值
    unsigned int remote_ip;//远程目标IP地址,可以使用inet_addr()转换赋值

    unsigned char socket_port;//socket_malloc返回的socket端口号
    unsigned char status;//socket状态标记
    volatile unsigned char lock;//多核情况下的锁住status
#if OS_SUPPORT
    OS_SEM irq_sem;
#endif
} SOCKET_INFO;

#define socket_lock(x) {while(x);(x) = 1;}
#define socket_unlock(x) {(x) = 0;}

extern void socket_interrupt(void);
extern int socket_create(SOCKET_INFO *socket);
extern int socket_accept(SOCKET_INFO *socket, u32 accpet_timeout);//TCP 服务器模式下
extern int socket_connect(SOCKET_INFO *socket);//TCP 客户端模式下
extern int socket_recv(SOCKET_INFO *socket, u8 *buf, u32 size);
extern int socket_send(SOCKET_INFO *socket, u8 *buf, u32 size);
extern void socket_close(SOCKET_INFO *socket);
extern int rand(void);

static inline unsigned int inet_addr(char *addr)//字符串IP转网络IP地址
{
    unsigned char ip[3] = {0};
    unsigned char value = 0;
    unsigned char comma = 0;
    unsigned int i = 0;
    unsigned int ret = 0;

    while (addr[i] != '\0') {
        if (addr[i] >= '0' && addr[i] <= '9') {
            ip[i] = addr[i] -  '0';
            i++;
        } else if (addr[i] == '.') {
            addr += i + 1;
            switch (i) {
            case 1:
                value = ip[0];
                break;
            case 2:
                value = ip[0] * 10 + ip[1];
                break;
            case 3:
                value = ip[0] * 100 + ip[1] * 10 + ip[2];
                break;
            }
            ret |= value << (comma * 8);
            comma++;
            i = 0;
            ip[0] = 0;
            ip[1] = 0;
            ip[2] = 0;
        }
    }
    switch (i) {
    case 1:
        value = ip[0];
        break;
    case 2:
        value = ip[0] * 10 + ip[1];
        break;
    case 3:
        value = ip[0] * 100 + ip[1] * 10 + ip[2];
        break;
    }
    ret |= value << (comma * 8);
    return ret;
}
static inline unsigned short htons(unsigned short port)//字符串IP转网络IP地址
{
    unsigned short ret = 0;
//	ret |= ((port>>8) & 0xff ) | ((port& 0xff)<<8);
    ret |= port;
    return ret;
}
#endif

