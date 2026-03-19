#include <stdarg.h>

typedef unsigned long size_t ;
extern "C" {
    int fseek(void *file, int offset, int orig);
    int ftell(void *file);
    void *malloc(size_t);
    void *calloc(size_t __nmemb, size_t __size);
    void free(void *);
    extern int vsnprintf(char *, unsigned long, const char *, __builtin_va_list);
    extern int vsprintf(char *str, const char *format, va_list ap);
    int printf(char const *, ...);
    int vprintf(const char *format, va_list arg);
    void local_irq_enable(void);
    void local_irq_disable(void);
}

extern "C" {
    __attribute__((noreturn))
    void __cxa_pure_virtual(void)
    {
        printf("Need to make sure \"__cxa_pure_virtual\" runs OK!");
        while (1);
    }

    __attribute__((noreturn))
    void __cxa_deleted_virtual(void)
    {
        printf("Need to make sure \"__cxa_deleted_virtual\" runs OK!");
        while (1);
    }

    int __cxa_atexit(void (*destructor)(void *), void *arg, void *dso)
    {
        // printf("Need to make sure \"__cxa_atexit\" runs OK!");
        // 调用这个函数来注册全局变量的析构函数
        // 析构全局变量的时候，会调用这里注册的东西
        return 0;
    }
    void __cxa_finalize(void *f)
    {
        printf("Need to make sure \"__cxa_finalize\" runs OK!");
    }

    void *__dso_handle __attribute__((__visibility__("hidden")));

    int swprintf(wchar_t *ws, size_t len, const wchar_t *format, ...)
    {
        printf("Need to make sure \"swprintf\" runs OK!");
        return -1;
    }

    // int fputc(int c, void *stream)   // should be FILE *stream
    // {
    // printf("Need to make sure \"fputc\" runs OK!");
    // return -1;
    // }

    char *fgets(char *str, int n, void *stream)// should be FILE *stream
    {
        printf("Need to make sure \"fgets\" runs OK!");
        return 0;
    }

    int feof(void *stream)// should be FILE *stream
    {
        printf("Need to make sure \"feof\" runs OK!");
        return 0;
    }

    int fflush(void *stream)// should be FILE *stream
    {
        printf("Need to make sure \"fflush\" runs OK!");
        return 0;
    }

    int fputs(const char *s, void *stream)
    {
        printf("Need to make sure \"fputs\" runs OK!");
        return 0;
    }

    int fprintf(void *stream, const char *format, ...)// should be FILE *stream
    {
        printf("Need to make sure \"fprintf\" runs OK!");
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        return 0;
    }

    int vasprintf(char **buf, const char *fmt, va_list ap)
    {
        printf("Need to make sure \"vasprintf\" runs OK!");
        int chars;
        char *b;
        static char _T_emptybuffer = '\0';

        if (!buf) {
            return -1;
        }

        chars = vsnprintf(&_T_emptybuffer, 0, fmt, ap) + 1;
        if (chars < 0) {
            chars *= -1;    /* CAW: old glibc versions have this problem */
        }

        b = (char *)malloc(sizeof(char) * chars);
        if (!b) {
            return -1;
        }

        if ((chars = vsprintf(b, fmt, ap)) < 0) {
            free(b);
        } else {
            *buf = b;
        }

        return chars;
    }

    // void __rt_local_irq_disable(void)
    // {
    // local_irq_disable();
    // }

    // void __rt_local_irq_enable(void)
    // {
    // local_irq_enable();
    // }

    int atexit(void (*)(void))
    {
        printf("Need to make sure \"atexit\" runs OK!");
        return 0;
    }

    void *_malloc_r(size_t sz)
    {
        return malloc(sz);
    }

    void _free_r(void *p)
    {
        return free(p);
    }

    void *_calloc_r(struct _reent *ptr, size_t size, size_t len)
    {
        void *pt = calloc(size, len);
        return pt;
    }
}

__attribute__((__weak__, __visibility__("default")))
void *
operator new (size_t size)
{
    if (size == 0) {
        size = 1;
    }
    void *p = malloc(size);
    if (p == (void *)0) {
        // puts("malloc failed");
        while (1);
    }
    return p;
}

__attribute__((__weak__, __visibility__("default")))
void *
operator new[](size_t size)
{
    return ::operator new (size);
}

__attribute__((__weak__, __visibility__("default")))
void
operator delete (void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}

__attribute__((__weak__, __visibility__("default")))
void
operator delete[](void *ptr)
{
    ::operator delete (ptr);
}

//fix me
extern "C" {
    int fseeko(void *stream, int offset, int fromwhere)
    {
        return fseek(stream, offset, fromwhere);
    }

    int ftello(void *stream)
    {
        return ftell(stream);
    }

    int strerror_r(int errnum, char *buf, unsigned int n)
    {
        printf("[%s, %d]is not supported yet\n", __FUNCTION__, __LINE__);
        return -1;
    }

    void *fdopen(int fd, const char *mode)
    {
        printf("[%s, %d]is not supported yet\n", __FUNCTION__, __LINE__);
        return (void *)0;
    }
}
