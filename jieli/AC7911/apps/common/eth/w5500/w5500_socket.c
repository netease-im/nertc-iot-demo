#include "includes.h"
#include "w5500_socket.h"

static u8 socket_num = 0;
static void *socket_priv[SOCKET_MAX_NUM] = {0};
/********************************************************************
*函数名：socket_malloc
*功  能：申请socket端口号:0-7
*参  数：无
*返回值：端口号
*********************************************************************/
static int socket_malloc(void *priv)
{
    u8 i;
    int ret = -1;
    for (i = 0; i < SOCKET_MAX_NUM ; i++) {
        if (!(socket_num & BIT(i)) && socket_priv[i] == NULL) {
            socket_priv[i] = priv;
            socket_num |= BIT(i);
            ret = i;
            break;
        }
    }
    return ret;
}
/********************************************************************
*函数名：socket_free
*功  能：释放socket端口号:0-7
*参  数：port 端口号
*返回值：无
*********************************************************************/
static void socket_free(u8 port)
{
    if (port >= SOCKET_MAX_NUM) {
        return;
    }
    if (port & BIT(port) && socket_priv[port] != NULL) {
        socket_priv[port] = NULL;
        socket_num &= ~BIT(port);
    }
}
/********************************************************************
*函数名：socket_err
*功  能：socket出现异常，需要确定socket或IP问题
*参  数：err出错参数：UNREACH CONFLICT
*返回值：无
*********************************************************************/
static void socket_err(u8 err)
{
    for (u8 num = 0; num < SOCKET_MAX_NUM; num ++) {
        SOCKET_INFO *socket = (SOCKET_INFO *)socket_priv[num];
        if (socket && (err == UNREACH || err == CONFLICT)) { //Socket事件处理
            socket->status |= IR_DISCON;//要求关闭
        }
#if OS_SUPPORT
        if (socket) {
            os_sem_post(&socket->irq_sem);
        }
#endif
    }
}
/********************************************************************
*函数名：socket_port_rank
*功  能：socket本地端口号生成
*参  数：无
*返回值：无
*********************************************************************/
static unsigned short socket_port_rank(void)
{
#if 1
    int i = 0, p = 0;
    while (p < 1024 || p >= 65535) {
        /*p = rand() % 65535;*/
        p = CPU_RAND() % 65535;
        i++;
        if (i > 1000) {
            p = LOCAL_DEFAULT_PORT;
            break;
        }
    }
    return (unsigned short)p;
#else
    return LOCAL_DEFAULT_PORT;
#endif
}
/********************************************************************
*函数名：detect_gateway
*功  能：检查网关及获取网关的物理地址，检测网卡是否可用
*参  数：socket 网络socket结构体
*返回值：真 可用，假 不可用
*********************************************************************/
static int detect_gateway(SOCKET_INFO *socket)//检查网关服务器
{
    u8 ip_adde[4];
    u8 j = 0;

    ip_adde[0] = ((socket->local_ip >> 24) & 0xff) + 1;
    ip_adde[1] = ((socket->local_ip >> 16) & 0xff) + 1;
    ip_adde[2] = ((socket->local_ip >> 8) & 0xff) + 1;
    ip_adde[3] = ((socket->local_ip >> 0) & 0xff) + 1;

    Write_W5500_SOCK_4Byte(socket->socket_port, Sn_DIPR, ip_adde);			//向目的地址寄存器写入与本机IP不同的IP值,检查网关及获取网关的物理地址

    if (socket->prot == TCP_PROTOCOL) { //TCP
        Write_W5500_SOCK_1Byte(socket->socket_port, Sn_MR, MR_TCP);			//设置socket为TCP模式
        Write_W5500_SOCK_1Byte(socket->socket_port, Sn_CR, OPEN);				//打开Socket
        socket_delay_ms(10);//延时10ms

        if (Read_W5500_SOCK_1Byte(socket->socket_port, Sn_SR) != SOCK_INIT) {	//如果socket打开失败
            Write_W5500_SOCK_1Byte(socket->socket_port, Sn_CR, CLOSE);		//打开不成功,关闭Socket
            socket_log("%s %d err \n", __FUNCTION__, __LINE__);
            return FALSE;													//返回FALSE(0x00)
        }

        Write_W5500_SOCK_1Byte(socket->socket_port, Sn_CR, CONNECT);			//设置Socket为Connect模式

        do {
            j = Read_W5500_SOCK_1Byte(socket->socket_port, Sn_IR);				//读取Socket中断标志寄存器
            if (j != 0) {
                Write_W5500_SOCK_1Byte(socket->socket_port, Sn_IR, j);		//清空中断标志
            }
            socket_delay_ms(10);//延时10ms
            if ((j & IR_TIMEOUT) == IR_TIMEOUT) {
                socket_log("%s %d err \n", __FUNCTION__, __LINE__);
                return FALSE;
            } else if (Read_W5500_SOCK_1Byte(socket->socket_port, Sn_DHAR) != 0xff) {
                Write_W5500_SOCK_1Byte(socket->socket_port, Sn_CR, CLOSE);	//关闭Socket
                return TRUE;
            }
        } while (1);
    } else if (socket->prot == UDP_PROTOCOL) { //UDP

    }
    return TRUE;
}
/********************************************************************
*函数名：socket_set_timer_out
*功  能：设置socket超时时间
*参  数：socket 网络socket结构体
*返回值：无
*********************************************************************/
static void socket_set_timer_out(SOCKET_INFO *socket, u32 time)
{
    socket->time_out = (u32)time;
}
/********************************************************************
*函数名：socket_interrupt
*功  能：硬件中断回调函数
*参  数：无
*返回值：无
*********************************************************************/
void socket_interrupt(void)
{
    SOCKET_INFO *socket;
    unsigned char i, j, num;
    unsigned int cnt = 0;

    i = Read_W5500_1Byte(IR);//读取中断标志寄存器
    Write_W5500_1Byte(IR, (i & 0xf0)); //回写清除中断标志
    if ((i & CONFLICT) == CONFLICT) { //IP地址冲突异常处理
        socket_log("warning ip address conflict !!!!!!!!!!!!!!!!!!\r\n\r\n");
        /*socket_err(CONFLICT);*/
    }
    if ((i & UNREACH) == UNREACH) { //地址无法到达异常处理
        socket_log("warning invalid ip address !!!!!!!!!!!!!!!!!!\r\n\r\n");//可能是数据发太猛
        /*socket_err(UNREACH);*/
    }
    if ((i = Read_W5500_1Byte(SIR)) != 0) {								//读取端口中断标志寄存器
        for (num = 0; num < SOCKET_MAX_NUM; num ++) {
            socket = (SOCKET_INFO *)socket_priv[num];
            if (socket && (i & BIT(socket->socket_port))) {								//Socket事件处理
                j = Read_W5500_SOCK_1Byte(socket->socket_port, Sn_IR);			//读取Socket中断标志寄存器
                if (!j) { //无标记则跳过
                    continue;
                }
                Write_W5500_SOCK_1Byte(socket->socket_port, Sn_IR, j);		//清空标志
                socket_lock(socket->lock);
                if (j & IR_CON) {												//在TCP模式下,Socket成功连接
                    socket->status |= IR_CON;								//网络连接状态0x02,端口完成连接，可以正常传输数据
                }
                if (j & IR_DISCON) {											//在TCP模式下Socket断开连接处理
                    Write_W5500_SOCK_1Byte(socket->socket_port, Sn_CR, CLOSE); //关闭端口,等待重新打开连接
                    socket->status |= IR_DISCON;							//网络连接状态0x00,端口连接失败
                }
                if (j & IR_SEND_OK) {											//Socket数据发送完成
                    socket->status |= IR_SEND_OK;							//端口发送一个数据包完成
                }
                if (j & IR_RECV) {												//Socket接收到数据,可以启动S_rx_process()函数
                    socket->status |= IR_RECV;								//端口接收到一个数据包
                }
                if (j & IR_TIMEOUT) {											//Socket连接或数据传输超时处理
                    Write_W5500_SOCK_1Byte(socket->socket_port, Sn_CR, CLOSE); // 关闭端口,等待重新打开连接
                    socket->status |= IR_TIMEOUT;
                }
                socket_unlock(socket->lock);
#if OS_SUPPORT
                os_sem_post(&socket->irq_sem);
#endif
            }
        }
    }
}
/********************************************************************
*函数名：socket_create
*功  能：创建一个socket
*参  数：socket 网络socket结构体
*返回值：0 成功，小于0 失败
*********************************************************************/
int socket_create(SOCKET_INFO *socket)
{
    char cnt = 0;
    int ret = socket_malloc(socket);//申请socket端口号 0-7
    if (ret < 0) {
        socket_log("err no enough port\n");
        return -EFAULT;
    }
    socket->socket_port = (u8)ret;
    if (!socket->local_ip) {
        socket->local_ip = W5500_ip_addr_get();
    }
    if (!socket->local_port && socket->mode == TCP_SERVER_MODE) {
        socket_log("err unknown port = %d\n", socket->local_port);
        return -EFAULT;
    }
    if (!socket->local_port) {
        socket->local_port = htons(socket_port_rank());
    }
redo:
    if (!detect_gateway(socket)) {														//检查网关、获取网关的物理地址，检测网卡是否可用
        cnt++;
        if (cnt < 5) {
            socket_delay_ms(100);
            goto redo;
        }
        socket_free(socket->socket_port);
        return -EFAULT;
    }
#if OS_SUPPORT
    os_sem_create(&socket->irq_sem, 0);
#endif
    if (socket->prot == TCP_PROTOCOL) { //TCP
        Write_W5500_SOCK_2Byte(socket->socket_port, Sn_MSSR, SOCKET_MAX_SIZE);						//最大分片字节数=1460,设置分片长度，参考W5500数据手册，该值可以不修改
        Write_W5500_SOCK_2Byte(socket->socket_port, Sn_PORT, socket->local_port);		//设置端口的端口号,在服务器侦听模式不需要设置目的IP和目的端口号
        if (socket->mode == TCP_CLIENT_MODE) {
            Write_W5500_SOCK_2Byte(socket->socket_port, Sn_DPORTR, socket->remote_port);//设置端口目的(远程)端口号
            Write_W5500_SOCK_4Byte(socket->socket_port, Sn_DIPR, (u8 *)&socket->remote_ip);	//设置端口目的(远程)IP地址
        }
    } else if (socket->prot == UDP_PROTOCOL) { //UDP
        Socket_UDP(socket->socket_port);
        Write_W5500_SOCK_2Byte(socket->socket_port, Sn_DPORTR, socket->remote_port);	//设置目的主机端口号
        Write_W5500_SOCK_4Byte(socket->socket_port, Sn_DIPR, (u8 *)&socket->remote_ip);		//设置目的主机IP
    }
    socket->status = 0;
    socket->lock = 0;

    //启动中断，参考W5500数据手册确定自己需要的中断类型
    //IMR_CONFLICT是IP地址冲突异常中断,IMR_UNREACH是UDP通信时，地址无法到达的异常中断
    //其它是Socket事件中断，根据需要添加
    Write_W5500_1Byte(IMR, IM_IR7 | IM_IR6);
    Write_W5500_1Byte(SIMR, BIT(socket->socket_port));
    Write_W5500_SOCK_1Byte(socket->socket_port, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
    return ENONE;
}
/********************************************************************
*函数名：socket_accept
*功  能：等待客户端连接(TCP服务器模式，注意：只能一个客户端连接)
*参  数：socket 网络socket结构体，accpet_timeout 等待超时时间(0永不超时，单位ms)
*返回值：0 成功， 小于0 失败
*********************************************************************/
int socket_accept(SOCKET_INFO *socket, u32 accpet_timeout)//TCP 服务器模式下，注意：只能一个客户端连接
{
    int ret = -EFAULT;
    u32 timeout = accpet_timeout;
    u32 cnt = 0;
    u32 retry = 0;

    if (socket->prot == TCP_PROTOCOL) { //TCP
        if (socket->mode == TCP_SERVER_MODE) { //服务器
redo:
            if (!Socket_Listen(socket->socket_port)) {
                retry++;
                if (retry < 50) {
                    socket_log("listen retry against ...\n");
                    socket_delay_ms(500);
                    goto redo;
                } else {
                    socket_log("err in function %s \n", __FUNCTION__);
                    socket_log("retry against err , please check the net line \n");
                }
            } else {
                socket_log("local_ip : %d:%d:%d:%d listen port : %d , wait connet ...\n",
                           (u8)(socket->local_ip >> 24),
                           (u8)(socket->local_ip >> 16),
                           (u8)(socket->local_ip >> 8),
                           (u8)(socket->local_ip & 0xFF),
                           socket->local_port);
                while (1) {													//阻塞等待客户端连接
                    cnt++;
#if OS_SUPPORT
                    if (cnt > 1 || os_sem_pend(&socket->irq_sem, timeout ? timeout / 10 : 0)) {
                        os_sem_set(&socket->irq_sem, 0);
                        return -ETIMOUT;
                    }
#else
                    socket_delay_ms(10);
                    if (timeout && ((cnt * 10) > timeout)) {			//accpet超时
                        return -ETIMOUT;
                    }
#endif
                    socket_lock(socket->lock);
                    if (socket->status & IR_CON) {							//网络连接状态0x01,端口完成连接，可以正常传输数据
                        socket->status &= ~IR_CON;							//清空标记
                        ret = ENONE;
                        socket_unlock(socket->lock);
                        break;
                    }
                    socket_unlock(socket->lock);
                }
                socket_set_timer_out(socket, socket->time_out);
                socket_log("client conneted \n");
            }
        }
    }
    return ret;
}
/********************************************************************
*函数名：socket_connect
*功  能：连接服务器(TCP服务器模式)
*参  数：socket 网络socket结构体
*返回值：0 成功， 小于0 失败
*********************************************************************/
int socket_connect(SOCKET_INFO *socket)//TCP 客户端模式下
{
    u32 retry = 0;
    u32 cnt = 0;
    u32 timeout = socket->time_out;
    int ret = -EFAULT;
    if (socket->prot == TCP_PROTOCOL) { //TCP
        if (socket->mode == TCP_CLIENT_MODE) {
redo:
            if (!Socket_Connect(socket->socket_port)) {
                retry++;
                if (retry < 50) {
                    socket_log("connect retry against ...\n");
                    socket_delay_ms(50);
                    goto redo;
                } else {
                    socket_log("err in function %s \n", __FUNCTION__);
                    socket_log("retry against err , please check the net line \n");
                }
            } else {
                while (1) {
                    cnt++;
#if OS_SUPPORT
                    if (cnt > 1 || os_sem_pend(&socket->irq_sem, timeout ? timeout / 10 : 0)) {
                        os_sem_set(&socket->irq_sem, 0);
                        return -ETIMOUT;
                    }
#else
                    socket_delay_ms(10);
                    if (timeout && cnt > timeout) {
                        return -ETIMOUT;//接收超时
                    }
#endif
                    socket_lock(socket->lock);
                    if (socket->status & IR_DISCON) {		//在TCP模式下Socket断开连接处理
                        socket->status &= ~IR_CON;		//清空标记
                        socket_unlock(socket->lock);
                        return -EDISCON;
                    }
                    if (socket->status & IR_TIMEOUT) {	//Socket连接或数据传输超时处理
                        socket->status &= ~IR_TIMEOUT;	//清空标记
                        socket_unlock(socket->lock);
                        return -ETIMOUT;
                    }
                    if (socket->status & IR_CON) {		//Socket连接成功
                        socket->status &= ~IR_CON;		//清空标记
                        socket_unlock(socket->lock);
                        break;
                    }
                    socket_unlock(socket->lock);
                }
                ret = ENONE;
            }
        }
    }
    return ret;
}
/********************************************************************
*函数名：socket_recv
*功  能：连接服务器(TCP服务器模式)
*参  数：socket 网络socket结构体，buf接收数据地址，size 数据地址长度
*返回值：大于0 成功接收字节数， 小于等于0 失败
*********************************************************************/
int socket_recv(SOCKET_INFO *socket, u8 *buf, u32 size)
{
    u32 cnt = 0;
    u32 timeout = socket->time_out;
    u16 recv;
    if (size > SOCKET_MAX_SIZE) {
        socket_log("err recv size %d > %d \n", size, SOCKET_MAX_SIZE);
        return -EFAULT;
    }

wait:
    while (1) {
        cnt++;
#if OS_SUPPORT
        if (cnt > 1 || os_sem_pend(&socket->irq_sem, timeout ? timeout / 10 : 0)) {
            os_sem_set(&socket->irq_sem, 0);
            return -ETIMOUT;
        }
#else
        socket_delay_ms(10);
        if (timeout && cnt > timeout) {
            return -ETIMOUT;//接收超时
        }
#endif
        socket_lock(socket->lock);
        if (socket->status & IR_DISCON) {		//在TCP模式下Socket断开连接处理
            socket->status &= ~IR_CON;		//清空标记
            socket_unlock(socket->lock);
            return -EDISCON;
        }
        if (socket->status & IR_TIMEOUT) {	//Socket连接或数据传输超时处理
            socket->status &= ~IR_TIMEOUT;	//清空标记
            socket_unlock(socket->lock);
            return -ETIMOUT;
        }
        if (socket->status & IR_RECV) {		//Socket接收到数据
            socket->status &= ~IR_RECV;		//清空标记
            socket_unlock(socket->lock);
            break;
        }
        socket_unlock(socket->lock);
    }
    recv = Read_SOCK_Data_Buffer(socket->socket_port, buf);//读取一包数据，一次最大读取1460个字节
    if (recv > size) {
        socket_log("err in recv buff , recv size %d , buff size : %d \n", recv, size);
        return -ENOMEM;
    } else if (recv == 0) {
        goto wait;
    }
    if (socket->prot == UDP_PROTOCOL) { //注意：UDP接收带有8字节头部
        recv -= 8;
        memcpy(buf, buf + 8, recv);
    }
    return recv;
}
/********************************************************************
*函数名：socket_send
*功  能：连接服务器(TCP服务器模式)
*参  数：socket 网络socket结构体，buf接收数据地址，size 数据地址长度
*返回值：大于0 成功发送字节数， 小于等于0 失败
*********************************************************************/
int socket_send(SOCKET_INFO *socket, u8 *buf, u32 size)
{
    u32 cnt = 0;
    u32 timeout = socket->time_out;

    //如果是UDP模式,可以在此设置目的主机的IP和端口号
    if ((Read_W5500_SOCK_1Byte(socket->socket_port, Sn_MR) & 0x0f) != SOCK_UDP) { //如果Socket打开失败
        Write_W5500_SOCK_2Byte(socket->socket_port, Sn_DPORTR, socket->remote_port);//设置端口目的(远程)端口号
        Write_W5500_SOCK_4Byte(socket->socket_port, Sn_DIPR, (u8 *)&socket->remote_ip);	//设置端口目的(远程)IP地址
    }
    Write_SOCK_Data_Buffer(socket->socket_port, buf, size);
    while (1) {
        cnt++;
#if OS_SUPPORT
        if (cnt > 1 || os_sem_pend(&socket->irq_sem, timeout ? timeout / 10 : 0)) {
            os_sem_set(&socket->irq_sem, 0);
            return -ETIMOUT;
        }
#else
        socket_delay_ms(10);
        if (timeout && cnt > timeout) {
            return -ETIMOUT;//发送超时
        }
#endif
        socket_lock(socket->lock);
        if (socket->status & IR_SEND_OK) {
            socket->status &= ~IR_SEND_OK; //发送成功，清空标志
            socket_unlock(socket->lock);
            break;
        }
        socket_unlock(socket->lock);
    }
    return size;
}
/********************************************************************
*函数名：socket_close
*功  能：关闭socket
*参  数：socket 网络socket结构体
*返回值：无
*********************************************************************/
void socket_close(SOCKET_INFO *socket)
{
    Write_W5500_SOCK_1Byte(socket->socket_port, Sn_CR, DISCON);
    Write_W5500_SOCK_1Byte(socket->socket_port, Sn_CR, CLOSE);
    socket_free(socket->socket_port);//释放socket端口号0-7
#if OS_SUPPORT
    os_sem_del(&socket->irq_sem, 0);
#endif
    memset(socket, 0, sizeof(SOCKET_INFO));
}

