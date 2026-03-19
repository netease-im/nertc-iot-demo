#include "includes.h"
#include "os/os_api.h"
#include "w5500_socket.h"
#include "w5500.h"

int tcp_server_test(void)
{
    int ret;
    SOCKET_INFO socket;
    u8 buff[SOCKET_MAX_SIZE];

    /*
     *1 . 初始化W5500硬件
     */
    W5500_Init();

    /*-----------------TCP server服务器模式--------------------*/
socket_init:
    /*
     *2 . 初始化网络结构协议、模式、IP地址、端口号
     */
    memset(&socket, 0, sizeof(SOCKET_INFO));
    socket.time_out = 0;//永不超时
    socket.prot = TCP_PROTOCOL;//TCP协议
    socket.mode = TCP_SERVER_MODE;//TCP服务器模式
    socket.local_port = htons(5000);//本地监听端口

    /*
     *3 . 创建socket
     */
    ret = socket_create(&socket);
    if (ret) {
        printf("socket_create err !!!\n");
        return 0;
    }

    /*
     *4 . 等待客户端accpet
     */
    ret = socket_accept(&socket, 0);
    if (ret) {
        printf("socket_accept err !!!\n");
        return 0;
    }
    while (1) {
        /*
         *5 . 接收数据
         */
        ret = socket_recv(&socket, buff, sizeof(buff) - 1);
        if (ret > 0) {
            buff[ret] = 0;//最后字节加上结束符
            printf("recv %d byte data : %s \n", ret, buff);
            /*
             *6 . 发送数据
             */
            socket_send(&socket, buff, ret);
            printf("send data : %s \n", buff);
        } else {
            if (ret == -EDISCON) {
                printf("client disconnected \n");
                socket_close(&socket);
                goto socket_init;
            }
        }
    }
    return 0;
}
int tcp_client_test(void)
{
    int ret;
    SOCKET_INFO socket;
    u8 buff[SOCKET_MAX_SIZE];

    /*
     *1 . 初始化W5500硬件
     */
    W5500_Init();

    /*-----------------TCP client客户端模式--------------------*/
socket_init:
    /*
     *2 . 初始化网络结构协议、模式、IP地址、端口号
     */
    memset(&socket, 0, sizeof(SOCKET_INFO));
    socket.time_out = 1000;//永不超时
    socket.prot = TCP_PROTOCOL;//TCP协议
    socket.mode = TCP_CLIENT_MODE;//TCP客户端模式
    socket.remote_ip = inet_addr("192.168.0.100");//服务器IP地址
    socket.remote_port = htons(9000);//服务器端口号

    /*
     *3 . 创建socket
     */
    ret = socket_create(&socket);
    if (ret) {
        printf("socket_create err !!!\n");
        return 0;
    }

    /*
     *4 . connet服务器
     */
    printf("connect ......\n");
    ret = socket_connect(&socket);
    if (ret) {
        printf("socket_connect err !!!\n");
        return 0;
    }
    u8 *str = "hello world";
    printf("connect server ok , send data %s\n", str);

    /*
     *5 . 发送数据
     */
    socket_send(&socket, str, strlen(str));
    while (1) {
        /*
         *6 . 接收数据
         */
        ret = socket_recv(&socket, buff, sizeof(buff));
        if (ret > 0) {
            buff[ret] = 0;
            printf("recv %d byte data : %s \n", ret, buff);

            /*
             *7 . 发送数据
             */
            socket_send(&socket, buff, ret);
            printf("send data : %s \n", buff);
        } else {
            if (ret == -EDISCON) {
                printf("server disconnected \n");
                socket_close(&socket);
                goto socket_init;
            } else {
                socket_send(&socket, str, strlen(str));
            }
        }
    }
    return 0;
}

int udp_client_test(void)
{
    int ret;
    SOCKET_INFO socket;
    u8 buff[SOCKET_MAX_SIZE];

    /*
     *1 . 初始化W5500硬件
     */
    W5500_Init();

socket_init:
    /* --------------------UDP客户端广播模式  ------------------*/
    /*
     *2 . 初始化网络结构协议、模式、IP地址、端口号
     */
    memset(&socket, 0, sizeof(SOCKET_INFO));
    socket.time_out = 1000;//永不超时
    socket.prot = UDP_PROTOCOL;//UDP协议
    socket.mode = UDP_BROADCAST_MODE;//UDP广播模式
    socket.remote_ip = inet_addr("192.168.0.100");//服务器IP地址
    socket.remote_port = htons(9000);//服务器端口号
    /*
     *3 . 创建socket
     */
    ret = socket_create(&socket);
    if (ret) {
        printf("socket_create err !!!\n");
        return 0;
    }

    /*
     *4 . 发送数据
     */
    printf("udp send data ...\n");
    u8 *str = "hello world udp test";
    socket_send(&socket, str, strlen(str));
    while (1) {
        /*
         *5 . 接收数据
         */
        ret = socket_recv(&socket, buff, sizeof(buff));
        if (ret > 0) {
            buff[ret] = 0;
            put_buf(buff, ret);
            printf("recv %d byte data : %s \n", ret, buff);

            /*
             *6 . 发送数据
             */
            socket_send(&socket, buff, ret);
            printf("send data : %s \n", buff);
        } else {
            if (ret == -EDISCON) {
                printf("server disconnected \n");
                socket_close(&socket);
                goto socket_init;
            } else {
                socket_send(&socket, str, strlen(str));
            }
        }
    }
    return 0;
}

int socket_task(void)
{
    tcp_server_test();//TCP服务器测试
    /*tcp_client_test();//TCP客户端测试*/
    /*udp_client_test();//UDP客户端测试*/
    while (1) {
        delay_ms(1000);
    }
}

int socket_test(void)
{
    int os_task_create(void (*func)(void *parm),
                       void *parm,
                       u8 prio,
                       u32 stk_size,
                       int q_size,
                       const char *name);

    return os_task_create(socket_task, NULL, 20, 4024, 0, "socket_task");
}



