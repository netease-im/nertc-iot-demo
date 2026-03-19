#include "asm/gpio.h"
#include "system/includes.h"
#include "app_config.h"
#include "asm/port_waked_up.h"
#include "device/gpio.h"
#include "demo_config.h"

#ifdef USE_GPIO_TEST_DEMO

static const u32 gpio_regs[] = {
    (u32) JL_PORTA,
    (u32) JL_PORTB,
    (u32) JL_PORTC,
    (u32) JL_PORTD,
    (u32) JL_PORTE,
    (u32) JL_PORTF,
    (u32) JL_PORTG,
    (u32) JL_PORTH,
};
static spinlock_t locks[ARRAY_SIZE(gpio_regs)] = {0};

static void *PA0IntHandler(void)   //外部中断处理

{
    printf("hello,jl");
}

static void c_main(void *priv)
{

    __gpio_direction_input(IO_PORTH_12);       //GPIO输入   同一组引脚之间没有互斥, 针对用户有高速度要求, 并且不需要考虑同一组引脚之间操作互斥才允许使用
    __gpio_set_pull_up(IO_PORTH_12, 1);        //上拉配置

    gpio_direction_output(IO_PORTC_01, 1);  //GPIO输出
    gpio_set_hd(IO_PORTC_01, 1);            //强驱配置

    //gpio_direction_input(IO_PORTA_01);         //GPIO输入
    //gpio_set_pull_down(IO_PORTA_01,1);
    os_time_dly(300);


    port_wakeup_reg(EVENT_IO_0, IO_PORTA_01, EDGE_POSITIVE, PA0IntHandler);//gpio中断模式使用（i/o口作外部中断，外部中断引脚，上升沿触发，中断响应函数）event_io_0可以映射到任意引脚



    gpio_direction_output(IO_PORT_USB_DPA, 1);
    gpio_direction_output(IO_PORT_USB_DMA, 1);  //特殊引脚如USB/配置为GPIO使用方法

    gpio_direction_input(IO_PORTH_04);         //GPIO输入,引脚PH4和PH6是双重绑定IO
    gpio_set_pull_down(IO_PORTH_04, 1);        //下拉配置

    os_time_dly(300);
    gpio_direction_output(IO_PORTH_06, 1);

    //gpio_output_channle(IO_PORTE_00,00);
    //gpio_read(IO_PORTE_00);
    //ret=gpio_read(IO_PORTE_00);
    //printf("ret:",ret);

    /********************************以下为GPIO直接操作寄存器示例*********************************/

    /* gpio_direction_output(IO_PORTC_03, 1);  //GPIO输出 */
    spin_lock(&locks[2]);					//对PORTC引脚上锁，0、1、2，分别代表A、B、C 组引脚,以此类推下去
    JL_PORTC->OUT |= BIT(3);				//将引脚PC03设置为高电平,0、1、2对应引脚号，对应bit位置1则为高电平
    JL_PORTC->DIR &= ~BIT(3);				//将引脚PCO3设置为输出模式,0、1、2对应引脚号，对应bit位置0为输出模式
    spin_unlock(&locks[2]);                //对PORTC引脚解锁，0、1、2，分别代表A、B、C 组引脚,以此类推下去

    os_time_dly(300);

    /* gpio_set_hd(IO_PORTC_03, 1);            //强驱配置 */
    spin_lock(&locks[2]);
    JL_PORTC->HD0 |= BIT(3);				//对应bit为置1为有效
    spin_unlock(&locks[2]);

    os_time_dly(300);

    /* gpio_set_hd1(IO_PORTC_03, 1);            //强驱配置 */
    spin_lock(&locks[2]);
    JL_PORTC->HD |= BIT(3); 				//对应bit为置1为有效
    spin_unlock(&locks[2]);

    os_time_dly(300);

    /* gpio_direction_output(IO_PORTC_03, 0);  //GPIO输出 */
    spin_lock(&locks[2]);
    JL_PORTC->OUT &= ~BIT(3);
    JL_PORTC->DIR &= ~BIT(3);
    spin_unlock(&locks[2]);

    os_time_dly(300);

    /* gpio_direction_intput(IO_PORTC_02, 1);  //GPIO输入 */
    spin_lock(&locks[2]);
    JL_PORTC->DIR |= BIT(2);
    spin_unlock(&locks[2]);

    os_time_dly(300);

    /* gpio_set_pull_up(IO_PORTC_02, 1);  //GPIO上拉 */
    spin_lock(&locks[2]);
    JL_PORTC->PU |= BIT(2);				//对应bit为置1为有效
    spin_unlock(&locks[2]);

    os_time_dly(300);

    /* gpio_set_pull_down(IO_PORTC_02, 1);  //GPIO下拉 */
    spin_lock(&locks[2]);
    JL_PORTC->PD |= BIT(2);				//对应bit为置1为有效
    spin_unlock(&locks[2]);

    os_time_dly(300);

    /* gpio_read(IO_PORTC_02); 			 //读GPIO */
    spin_lock(&locks[2]);
    int v = !!(JL_PORTC->IN & BIT(2));
    spin_unlock(&locks[2]);

    os_time_dly(300);

    /* gpio_set_die(IO_PORTC_02, 1);  //GPIO设置为数字输入 */
    spin_lock(&locks[2]);
    JL_PORTC->DIE |= BIT(2);
    spin_unlock(&locks[2]);

    os_time_dly(300);




}

late_initcall(c_main);

#endif

