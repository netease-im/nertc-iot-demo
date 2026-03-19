#include "app_config.h"
#include "system/includes.h"
#include "asm/wdt.h"

extern void coremark_main(int argc, char *argv[]);
extern void dhry_main(unsigned int);
extern void linpack_main(int arsize);
extern int whetstone_main(int argc, char *argv[]);
extern int ipi_fftcs_main(int argc, char *argv[]);

static void BenchMark_test_task(void *p)
{
    int argc;
    char *argv[4] = {"", 0};

    argc = 0;
    coremark_main(argc, argv);

    /*dhry_main(10000000);*/

    /*linpack_main(100);*/

    /*argc = 0;*/
    /*argv[++argc] = "1000";*/
    /*whetstone_main(++argc, argv);*/

    /*argc = 0;*/
    /*argv[++argc] = "10000"; //Number of digits of pi to calculate,注意精度越大消耗越大堆和任务的栈*/
    /*ipi_fftcs_main(++argc, argv);*/

    puts("\r\n\r\n BenchMarkTest RUN END\r\n");

    while (1) {
        os_time_dly(10000);
    }
}

static int c_main(void)
{
    printf("\r\n\r\n\r\n\r\n\r\n -----------BenchMark run %s -------------\r\n\r\n\r\n\r\n\r\n", __TIME__);

    wdt_close();    //关闭看门狗,防止占据CPU太猛导致复位

    os_task_create(BenchMark_test_task, NULL, 28, 3000, 0, "BenchMark_test_task");

    while (1) {
        os_time_dly(10000);
    }

    return 0;
}

late_initcall(c_main);

