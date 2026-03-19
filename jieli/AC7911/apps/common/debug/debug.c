#include "app_config.h"
#include "typedef.h"

#if (!defined CONFIG_DEBUG_ENABLE || defined CONFIG_SYS_DEBUG_DISABLE)
//关闭系统自带打印信息
int putchar(int a)
{
    return a;
}
int puts(const char *out)
{
    return 0;
}
int printf(const char *format, ...)
{
    return 0;
}
int vprintf(const char *restrict format, va_list arg)
{
    return 0;
}
void put_buf(const u8 *buf, int len)
{
}
void put_u8hex(u8 dat)
{
}
void put_u16hex(u16 dat)
{
}
void put_u32hex(u32 dat)
{
}
void log_print(int level, const char *tag, const char *format, ...)
{
}
#if (!defined CONFIG_DEBUG_LITE_ENABLE && !defined CONFIG_USER_DEBUG_ENABLE)
void log_putbyte(char c)
{

}
#endif /* #ifndef CONFIG_DEBUG_LITE_ENABLE */

int assert_printf(const char *format, ...)
{
    cpu_assert_debug();

    return 0;
}

#endif

#if 0 /*打开则在休眠唤醒flash检测出错则回调到该函数*/
void low_power_wakeup_sfc_cach_err(void) SEC(.volatile_ram_code)
{
    JL_PORTA->DIR &= ~BIT(0); //设置PA0为输出模式
    JL_PORTA->PU &= ~BIT(0); //关闭上拉
    JL_PORTA->PD &= ~BIT(0); //关闭下拉
    JL_PORTA->OUT &= ~BIT(0); //设置PA0输出0
    while (1) {
        JL_PORTA->OUT ^= BIT(0);//翻转PA0
        asm volatile("nop;");
    }
}
#endif

