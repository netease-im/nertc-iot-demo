#include "w5500.h"
#include "includes.h"
#include "asm/spi.h"
#include "generic/ioctl.h"


//注意：作为服务器需要设置网关和IP在有效(路由器分配的IP)的同一个网段下，局域网客户端才能连接
//#error "err notece !!!"//务必注意：配置的网关和IP地址需要在路由器分配的IP同一段网络，否则无法通信
static	const unsigned char gateway_ip[4] 	= {192, 168, 0, 1};				//网关参数,默认192.168.31.1
static	const unsigned char ip_addr[4] 		= {192, 168, 0, 199};				//本机IP地址,默认192.168.31.199
static	const unsigned char sub_mask[4] 	= {255, 255, 255, 0};				//子网掩码,默认255.255.255.0
static	const unsigned char phy_addr[6] 	= {0x0c, 0x29, 0xab, 0x7c, 0x00, 0x01}; //网卡物理地址mac地址(第一个字节必须为偶数),默认0x0c29ab7c0001


#if SPI_USE_DEV
static void *spi_handler = NULL;
#else
#define SPI_DLY_US		0	//使用IO模拟翻转时候的时钟跳变延时
#endif

/*******************************************************************************
* 函数名  : SPI_Configuration
* 描述    : W5500 SPI初始化配置(STM32 SPI1)
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
static void *SPI_Configuration(void)
{
#if SPI_USE_DEV
    if (!spi_handler) {
        spi_handler = dev_open((unsigned char *)SPI_DEV_NAME, NULL);
    }
    W5500_SCLK_SET_HD(1);
    return &spi_handler;
#else
    W5500_SCLK_SET_HD(1);
    return NULL;
#endif
}
/********************************************************************
*函数名：W5500_io_init
*功  能：W5500 IO 初始化
*参  数：interrupt_callback_func中断回调函数
*返回值：无
*********************************************************************/
void W5500_io_init(void (*interrupt_callback_func)(void))
{
    W5500_IO_INIT();
    SPI_Configuration();
    port_wakeup_reg(W5500_INT_EVENT, W5500_INT_IO, W5500_INT_EDGE, interrupt_callback_func);
}
/********************************************************************
*函数名：SPI_Read_Byte
*功  能：SPI读取一个字节数据
*参  数：无
*返回值：读取的数据
*********************************************************************/
static unsigned char SPI_Read_Byte(void)
{
    unsigned char i = 0;

#if SPI_USE_DEV
    if (spi_handler) {
        dev_ioctl(spi_handler, IOCTL_SPI_READ_BYTE, &i);
        /*dev_read(spi_handler,&i,1);*/
    }
    return i;
#else
    unsigned char rByte = 0;
    W5500_SCLK_SET(0);
    w5500_delay_us(SPI_DLY_US);
    for (i = 0; i < 8; i++) {
        W5500_SCLK_SET(1);
        w5500_delay_us(SPI_DLY_US);
        rByte <<= 1;
        rByte |= W5500_MISO_GET();
        w5500_delay_us(SPI_DLY_US);
        W5500_SCLK_SET(0);
        w5500_delay_us(SPI_DLY_US);
    }
    return rByte;
#endif
}

/********************************************************************
*函数名：SPI_SendByte
*功  能：SPI发送一个字节数据
*参  数：dt:待发送的数据
*返回值：无
*********************************************************************/
static void SPI_Send_Byte(unsigned char dt)
{
#if SPI_USE_DEV
    if (spi_handler) {
        dev_ioctl(spi_handler, IOCTL_SPI_SEND_BYTE, dt);
        /*dev_write(spi_handler,(unsigned char*)&dt,1);*/
    }
#else
    unsigned char i = 0;
    W5500_SCLK_SET(0);
    for (i = 0; i < 8; i++) {
        w5500_delay_us(SPI_DLY_US);
        W5500_SCLK_SET(0);
        w5500_delay_us(SPI_DLY_US);
        if ((dt << i) & 0x80) {
            W5500_MOSI_SET(1);
        } else {
            W5500_MOSI_SET(0);
        }
        w5500_delay_us(SPI_DLY_US);
        W5500_SCLK_SET(1);
        w5500_delay_us(SPI_DLY_US);
    }
    W5500_SCLK_SET(0);
#endif
}
/********************************************************************
*函数名：SPI_Send_Short
*功  能：SPI发送2个字节数据
*参  数：dt:待发送的数据
*返回值：无
*********************************************************************/
void SPI_Send_Short(unsigned short dt)
{
    SPI_Send_Byte((unsigned char)(dt / 256));	//写数据高位
    SPI_Send_Byte(dt);						//写数据低位
}
/********************************************************************
*函数名：Write_W5500_1Byte
*功  能：W5500写寄存器
*参  数：reg寄存器，dat写的值
*返回值：无
*********************************************************************/
void Write_W5500_1Byte(unsigned short reg, unsigned char dat)
{
    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(reg);//通过SPI写16位寄存器地址
    SPI_Send_Byte(FDM1 | RWB_WRITE | COMMON_R); //通过SPI写控制字节,1个字节数据长度,写数据,选择通用寄存器
    SPI_Send_Byte(dat);//写1个字节数据

    W5500_CS_SET(1); //置W5500的SCS为高电平
}
/********************************************************************
*函数名：Write_W5500_2Byte
*功  能：通过SPI向指定地址寄存器写2个字节数据
*参  数：reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
*返回值：无
*********************************************************************/
void Write_W5500_2Byte(unsigned short reg, unsigned short dat)
{
    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(reg);//通过SPI写16位寄存器地址
    SPI_Send_Byte(FDM2 | RWB_WRITE | COMMON_R); //通过SPI写控制字节,2个字节数据长度,写数据,选择通用寄存器
    SPI_Send_Short(dat);//写16位数据

    W5500_CS_SET(1); //置W5500的SCS为高电平
}
/********************************************************************
*函数名：Write_W5500_nByte
*功  能：通过SPI向指定地址寄存器写n个字节数据
*参  数：reg:16位寄存器地址,*dat_ptr:待写入数据缓冲区指针,
*       size:待写入的数据长度
*返回值：无
*********************************************************************/
void Write_W5500_nByte(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
    unsigned short i;

    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(reg);//通过SPI写16位寄存器地址
    SPI_Send_Byte(VDM | RWB_WRITE | COMMON_R); //通过SPI写控制字节,N个字节数据长度,写数据,选择通用寄存器

    for (i = 0; i < size; i++) { //循环将缓冲区的size个字节数据写入W5500
        SPI_Send_Byte(*dat_ptr++);//写一个字节数据
    }

    W5500_CS_SET(1); //置W5500的SCS为高电平
}
/********************************************************************
*函数名：Write_W5500_SOCK_1Byte
*功  能：通过SPI向指定端口寄存器写1个字节数据
*参  数：s:端口号,reg:16位寄存器地址,dat:待写入的数据
*返回值：无
*********************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(reg);//通过SPI写16位寄存器地址
    SPI_Send_Byte(FDM1 | RWB_WRITE | (s * 0x20 + 0x08)); //通过SPI写控制字节,1个字节数据长度,写数据,选择端口s的寄存器
    SPI_Send_Byte(dat);//写1个字节数据

    W5500_CS_SET(1); //置W5500的SCS为高电平
}
/********************************************************************
*函数名：Write_W5500_SOCK_2Byte
*功  能：通过SPI向指定端口寄存器写2个字节数据
*参  数：s:端口号,reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
*返回值：无
*********************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(reg);//通过SPI写16位寄存器地址
    SPI_Send_Byte(FDM2 | RWB_WRITE | (s * 0x20 + 0x08)); //通过SPI写控制字节,2个字节数据长度,写数据,选择端口s的寄存器
    SPI_Send_Short(dat);//写16位数据

    W5500_CS_SET(1); //置W5500的SCS为高电平
}
/********************************************************************
*函数名：Write_W5500_SOCK_4Byte
*功  能：通过SPI向指定端口寄存器写4个字节数据
*参  数：s:端口号,reg:16位寄存器地址,*dat_ptr:待写入的4个字节缓冲区指针
*返回值：无
*********************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(reg);//通过SPI写16位寄存器地址
    SPI_Send_Byte(FDM4 | RWB_WRITE | (s * 0x20 + 0x08)); //通过SPI写控制字节,4个字节数据长度,写数据,选择端口s的寄存器

    SPI_Send_Byte(*dat_ptr++);//写第1个字节数据
    SPI_Send_Byte(*dat_ptr++);//写第2个字节数据
    SPI_Send_Byte(*dat_ptr++);//写第3个字节数据
    SPI_Send_Byte(*dat_ptr++);//写第4个字节数据

    W5500_CS_SET(1); //置W5500的SCS为高电平
}
/********************************************************************
*函数名：Read_W5500_1Byte
*功  能：读W5500指定地址寄存器的1个字节数据
*参  数：reg:16位寄存器地址
*返回值：读取到寄存器的1个字节数据
*********************************************************************/
unsigned char Read_W5500_1Byte(unsigned short reg)
{
    unsigned char i;

    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(reg);//通过SPI写16位寄存器地址
    SPI_Send_Byte(FDM1 | RWB_READ | COMMON_R); //通过SPI写控制字节,1个字节数据长度,读数据,选择通用寄存器
    i = SPI_Read_Byte();

    W5500_CS_SET(1);//置W5500的SCS为高电平
    return i;//返回读取到的寄存器数据
}
/********************************************************************
*函数名：Read_W5500_SOCK_1Byte
*功  能：读W5500指定端口寄存器的1个字节数据
*参  数：s:端口号,reg:16位寄存器地址
*返回值：读取到寄存器的1个字节数据
*********************************************************************/
unsigned char Read_W5500_SOCK_1Byte(SOCKET s, unsigned short reg)
{
    unsigned char i;

    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(reg);//通过SPI写16位寄存器地址
    SPI_Send_Byte(FDM1 | RWB_READ | (s * 0x20 + 0x08)); //通过SPI写控制字节,1个字节数据长度,读数据,选择端口s的寄存器
    i = SPI_Read_Byte();

    W5500_CS_SET(1);//置W5500的SCS为高电平
    return i;//返回读取到的寄存器数据
}
/********************************************************************
*函数名：Read_W5500_SOCK_2Byte
*功  能：读W5500指定端口寄存器的2个字节数据
*参  数：s:端口号,reg:16位寄存器地址
*返回值：读取到寄存器的2个字节数据(16位)
*********************************************************************/
unsigned short Read_W5500_SOCK_2Byte(SOCKET s, unsigned short reg)
{
    unsigned short i;

    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(reg);//通过SPI写16位寄存器地址
    SPI_Send_Byte(FDM2 | RWB_READ | (s * 0x20 + 0x08)); //通过SPI写控制字节,2个字节数据长度,读数据,选择端口s的寄存器

    i = SPI_Read_Byte();
    i *= 256;
    i += SPI_Read_Byte(); //读取低位数据

    W5500_CS_SET(1);//置W5500的SCS为高电平
    return i;//返回读取到的寄存器数据
}
/********************************************************************
*函数名：Read_SOCK_Data_Buffer
*功  能：从W5500接收数据缓冲区中读取数据
*参  数：s:端口号,*dat_ptr:数据保存缓冲区指针
*返回值：读取到的数据长度,rx_size个字节
*********************************************************************/
unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
    unsigned short rx_size;
    unsigned short offset, offset1;
    unsigned short i;
    unsigned char j;

    rx_size = Read_W5500_SOCK_2Byte(s, Sn_RX_RSR);
    if (rx_size == 0) {
        return 0;    //没接收到数据则返回
    }
    if (rx_size > SOCKET_MAX_SIZE) {
        rx_size = SOCKET_MAX_SIZE;
    }

    offset = Read_W5500_SOCK_2Byte(s, Sn_RX_RD);
    offset1 = offset;
    offset &= (S_RX_SIZE - 1); //计算实际的物理地址

    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(offset);//写16位地址
    SPI_Send_Byte(VDM | RWB_READ | (s * 0x20 + 0x18)); //写控制字节,N个字节数据长度,读数据,选择端口s的寄存器

    if ((offset + rx_size) < S_RX_SIZE) { //如果最大地址未超过W5500接收缓冲区寄存器的最大地址
        for (i = 0; i < rx_size; i++) { //循环读取rx_size个字节数据
            j = SPI_Read_Byte(); //读取1个字节数据
            *dat_ptr = j; //将读取到的数据保存到数据保存缓冲区
            dat_ptr++;//数据保存缓冲区指针地址自增1
        }
    } else { //如果最大地址超过W5500接收缓冲区寄存器的最大地址
        offset = S_RX_SIZE - offset;
        for (i = 0; i < offset; i++) { //循环读取出前offset个字节数据
            j = SPI_Read_Byte(); //读取1个字节数据
            *dat_ptr = j; //将读取到的数据保存到数据保存缓冲区
            dat_ptr++;//数据保存缓冲区指针地址自增1
        }
        W5500_CS_SET(1); //置W5500的SCS为高电平

        W5500_CS_SET(0);//置W5500的SCS为低电平

        SPI_Send_Short(0x00);//写16位地址
        SPI_Send_Byte(VDM | RWB_READ | (s * 0x20 + 0x18)); //写控制字节,N个字节数据长度,读数据,选择端口s的寄存器

        for (; i < rx_size; i++) { //循环读取后rx_size-offset个字节数据
            j = SPI_Read_Byte(); //读取1个字节数据
            *dat_ptr = j; //将读取到的数据保存到数据保存缓冲区
            dat_ptr++;//数据保存缓冲区指针地址自增1
        }
    }
    W5500_CS_SET(1); //置W5500的SCS为高电平

    offset1 += rx_size; //更新实际物理地址,即下次读取接收到的数据的起始地址
    Write_W5500_SOCK_2Byte(s, Sn_RX_RD, offset1);
    Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);//发送启动接收命令
    return rx_size;//返回接收到数据的长度
}
/********************************************************************
*函数名：Write_SOCK_Data_Buffer
*功  能：将数据写入W5500的数据发送缓冲区
*参  数：s:端口号,*dat_ptr:数据保存缓冲区指针,size:待写入数据的长度
*返回值：无
*********************************************************************/
void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
    unsigned short offset, offset1;
    unsigned short i = 0;

    offset = Read_W5500_SOCK_2Byte(s, Sn_TX_WR);
    offset1 = offset;
    offset &= (S_TX_SIZE - 1); //计算实际的物理地址

    W5500_CS_SET(0);//置W5500的SCS为低电平

    SPI_Send_Short(offset);//写16位地址
    SPI_Send_Byte(VDM | RWB_WRITE | (s * 0x20 + 0x10)); //写控制字节,N个字节数据长度,写数据,选择端口s的寄存器

    if ((offset + size) < S_TX_SIZE) { //如果最大地址未超过W5500发送缓冲区寄存器的最大地址
        for (i = 0; i < size; i++) { //循环写入size个字节数据
            SPI_Send_Byte(*dat_ptr++);//写入一个字节的数据
        }
    } else { //如果最大地址超过W5500发送缓冲区寄存器的最大地址
        offset = S_TX_SIZE - offset;
        for (i = 0; i < offset; i++) { //循环写入前offset个字节数据
            SPI_Send_Byte(*dat_ptr++);//写入一个字节的数据
        }
        W5500_CS_SET(1); //置W5500的SCS为高电平

        W5500_CS_SET(0);//置W5500的SCS为低电平

        SPI_Send_Short(0x00);//写16位地址
        SPI_Send_Byte(VDM | RWB_WRITE | (s * 0x20 + 0x10)); //写控制字节,N个字节数据长度,写数据,选择端口s的寄存器
        for (; i < size; i++) { //循环写入size-offset个字节数据
            SPI_Send_Byte(*dat_ptr++);//写入一个字节的数据
        }
    }
    W5500_CS_SET(1); //置W5500的SCS为高电平
    offset1 += size; //更新实际物理地址,即下次写待发送数据到发送数据缓冲区的起始地址
    Write_W5500_SOCK_2Byte(s, Sn_TX_WR, offset1);
    Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);//发送启动发送命令
}
/********************************************************************
*函数名：W5500_Hardware_Reset
*功  能：硬件复位W5500
*参  数：无
*返回值：无
*********************************************************************/
void W5500_Hardware_Reset(void)
{
    u32 cnt = 0;
    W5500_RST_SET(0);//复位引脚拉低
    w5500_delay_ms(200);
    W5500_RST_SET(1);//复位引脚拉高
    w5500_delay_ms(200);
    while ((Read_W5500_1Byte(PHYCFGR)&LINK) == 0) { //等待以太网连接完成
        cnt++;
        w5500_delay_ms(10);
        if (cnt > 1000) {
            w5500_log("%s %d err \n", __FUNCTION__, __LINE__);
            break;
        }
    }
}
/********************************************************************
*函数名：W5500_ip_addr_get
*功  能：获取注册到W5500的IP地址
*参  数：无
*返回值：IP地址
*********************************************************************/
unsigned int W5500_ip_addr_get(void)
{
    return ((ip_addr[0] << 24) | (ip_addr[1] << 16) | (ip_addr[2] << 8) | (ip_addr[3]));
}
/********************************************************************
*函数名：W5500_Init
*功  能：初始化W5500寄存器
*参  数：无
*返回值：无
*********************************************************************/
void W5500_Init(void)
{
    unsigned char i = 0;

    W5500_io_init(socket_interrupt);//初始化IO，注册socket中断函数
    W5500_Hardware_Reset();//硬件复位W5500
    Write_W5500_1Byte(MR, RST);//软件复位W5500,置1有效,复位后自动清0
    w5500_delay_ms(10);//延时10ms,自己定义该函数

    //设置网关(Gateway)的IP地址,Gateway_IP为4字节unsigned char数组,自己定义
    //使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet
    Write_W5500_nByte(GAR, (unsigned char *)gateway_ip, 4);

    //设置子网掩码(MASK)值,SUB_MASK为4字节unsigned char数组,自己定义
    //子网掩码用于子网运算
    Write_W5500_nByte(SUBR, (unsigned char *)sub_mask, 4);

    //设置物理地址,PHY_ADDR为6字节unsigned char数组,自己定义,用于唯一标识网络设备的物理地址值
    //该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
    //如果自己定义物理地址，注意第一个字节必须为偶数
    Write_W5500_nByte(SHAR, (unsigned char *)phy_addr, 6);

    //设置本机的IP地址,IP_ADDR为4字节unsigned char数组,自己定义
    //注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关
    Write_W5500_nByte(SIPR, (unsigned char *)ip_addr, 4);

    //设置发送缓冲区和接收缓冲区的大小，参考W5500数据手册
    for (i = 0; i < 8; i++) {
        Write_W5500_SOCK_1Byte(i, Sn_RXBUF_SIZE, 0x02); //Socket Rx memory size=2k
        Write_W5500_SOCK_1Byte(i, Sn_TXBUF_SIZE, 0x02); //Socket Tx mempry size=2k
    }

    //设置重试时间，默认为2000(200ms)
    //每一单位数值为100微秒
    Write_W5500_2Byte(RTR, 200 * 10);

    //设置重试次数，默认为255次
    //如果重发的次数超过设定值,则产生超时中断(相关的端口中断寄存器中的Sn_IR 超时位(TIMEOUT)置“1”)
    Write_W5500_1Byte(RCR, 255);
}

/********************************************************************
*函数名：Socket_Connect
*功  能：TCP模式socket连接
*参  数：s:端口号
*返回值：无
*********************************************************************/
unsigned char Socket_Connect(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s, Sn_MR, MR_TCP); //设置socket为TCP模式
    Write_W5500_SOCK_1Byte(s, Sn_CR, OPEN); //打开Socket
    w5500_delay_ms(5);//延时5ms
    if (Read_W5500_SOCK_1Byte(s, Sn_SR) != SOCK_INIT) { //如果socket打开失败
        Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE); //打开不成功,关闭Socket
        w5500_log("%s %d err \n", __FUNCTION__, __LINE__);
        return FALSE;//返回FALSE(0x00)
    }
    Write_W5500_SOCK_1Byte(s, Sn_CR, CONNECT); //设置Socket为Connect模式
    return TRUE;//返回TRUE,设置成功
}
/********************************************************************
*函数名：Socket_Listen
*功  能：TCP模式设置指定Socket(0~7)作为服务器等待远程主机的连接
*参  数：s:端口号
*返回值：成功返回TRUE(0xFF),失败返回FALSE(0x00)
*说  明：当本机Socket工作在服务器模式时,引用该程序,等等远程主机的连接
*		该程序只调用一次,就使W5500设置为服务器模式
*********************************************************************/
unsigned char Socket_Listen(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s, Sn_MR, MR_TCP); //设置socket为TCP模式
    Write_W5500_SOCK_1Byte(s, Sn_CR, OPEN); //打开Socket
    w5500_delay_ms(5);//延时5ms
    if (Read_W5500_SOCK_1Byte(s, Sn_SR) != SOCK_INIT) { //如果socket打开失败
        Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE); //打开不成功,关闭Socket
        w5500_log("%s %d err \n", __FUNCTION__, __LINE__);
        return FALSE;//返回FALSE(0x00)
    }
    Write_W5500_SOCK_1Byte(s, Sn_CR, LISTEN); //设置Socket为侦听模式
    w5500_delay_ms(5);//延时5ms
    if (Read_W5500_SOCK_1Byte(s, Sn_SR) != SOCK_LISTEN) { //如果socket设置失败
        Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE); //设置不成功,关闭Socket
        w5500_log("%s %d err \n", __FUNCTION__, __LINE__);
        return FALSE;//返回FALSE(0x00)
    }

    return TRUE;

    //至此完成了Socket的打开和设置侦听工作,至于远程客户端是否与它建立连接,则需要等待Socket中断，
    //以判断Socket的连接是否成功。参考W5500数据手册的Socket中断状态
    //在服务器侦听模式不需要设置目的IP和目的端口号
}
/********************************************************************
*函数名：Socket_UDP
*功  能：设置指定Socket(0~7)为UDP模式
*参  数：s:端口号
*返回值：成功返回TRUE(0xFF),失败返回FALSE(0x00)
*说  明：如果Socket工作在UDP模式,引用该程序,在UDP模式下,Socket通信不需
*		要建立连接，该程序只调用一次，就使W5500设置为UDP模式
*********************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s, Sn_MR, MR_UDP); //设置Socket为UDP模式*/
    Write_W5500_SOCK_1Byte(s, Sn_CR, OPEN); //打开Socket*/
    w5500_delay_ms(5);//延时5ms
    if (Read_W5500_SOCK_1Byte(s, Sn_SR) != SOCK_UDP) { //如果Socket打开失败
        Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE); //打开不成功,关闭Socket
        w5500_log("%s %d err \n", __FUNCTION__, __LINE__);
        return FALSE;//返回FALSE(0x00)
    } else {
        return TRUE;
    }
    //至此完成了Socket的打开和UDP模式设置,在这种模式下它不需要与远程主机建立连接
    //因为Socket不需要建立连接,所以在发送数据前都可以设置目的主机IP和目的Socket的端口号
    //如果目的主机IP和目的Socket的端口号是固定的,在运行过程中没有改变,那么也可以在这里设置
}


