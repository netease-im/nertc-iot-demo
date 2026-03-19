#include "typedef.h"
void cpp_run_init(void)
{
    typedef void (*pfunc)();
    extern pfunc ctors_begin[];
    extern pfunc ctors_end[];
    pfunc *p;

    // 调用全局构造函数（注意是要逆序）
    for (p = ctors_end; p > ctors_begin;) {
        (*--p)();    // 注意这里是倒序调用的
    }
}
