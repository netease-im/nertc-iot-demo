#include "system/includes.h"
#include "os/os_api.h"
#include "asm/crc16.h"
#include "http/http_cli.h"
#include "fs/fs.h"
#include "app_config.h"
#include "system/init.h"
#include "device/device.h"

#define DMA_COPY_TEST 1 //打开DMAcopy测试
#define DMA_COPY_TEST_SDRAM_TO_SDRAM_EN 		1 //sdram --> sdram
#define DMA_COPY_TEST_SDRAM_TO_SDRAM_NOCACH_EN 	1 //sdram nocach --> sdram nocach
#define DMA_COPY_TEST_SDRAM_TO_SRAM_EN 			1 //sdram --> sram
#define DMA_COPY_TEST_SRAM_TO_SDRAM_EN 			1 //sram --> sdram
#define DMA_COPY_TEST_SRAM_TO_SDRAM_NOCACH_EN 	1 //sram --> sdram nocach
#define DMA_COPY_TEST_SRAM_TO_SRAM_EN 			1 //sram --> sram

/*#define SRAM_DMA_TEST_BUF_SIZE	(120 * 1024)*/
#define SDRAM_DMA_TEST_BUF_SIZE	(3 * 1024 * 1024)
#define SRAM_DMA_TEST_BUF_SIZE	(60 * 1024)
/*#define SDRAM_DMA_TEST_BUF_SIZE	(SRAM_DMA_TEST_BUF_SIZE * 6)*/

/********************DMA COPY TEST*****************************/

#if DMA_COPY_TEST
static u8 dma_tx_buf_sdram[SDRAM_DMA_TEST_BUF_SIZE] ALIGNE(32);
static u8 dma_rx_buf_sdram[SDRAM_DMA_TEST_BUF_SIZE] ALIGNE(32);
static u8 dma_tx_buf_ram[SRAM_DMA_TEST_BUF_SIZE] SEC_USED(.sram) ALIGNE(32);
static u8 dma_rx_buf_ram[SRAM_DMA_TEST_BUF_SIZE] SEC_USED(.sram) ALIGNE(32);

static void dma_sdram_to_sdram(void)
{
    int ret = 0;

    for (int i = 0; i < sizeof(dma_tx_buf_sdram) - 2; i++) {
        dma_tx_buf_sdram[i] = (u8)JL_RAND->R64H;
    }
    ret = CRC16(dma_tx_buf_sdram, sizeof(dma_tx_buf_sdram) - 2);
    dma_tx_buf_sdram[sizeof(dma_tx_buf_sdram) - 2] = (ret >> 8 & 0x00ff);
    dma_tx_buf_sdram[sizeof(dma_tx_buf_sdram) - 1] = (ret & 0x00ff);

    memset(dma_rx_buf_sdram, 0, sizeof(dma_rx_buf_sdram));
    dma_copy(dma_rx_buf_sdram, dma_tx_buf_sdram, sizeof(dma_tx_buf_sdram));
    ret = CRC16(dma_rx_buf_sdram, sizeof(dma_tx_buf_sdram));
    if (ret) {
        printf("dma_sdram_to_sdram\n");
        printf("dma_tx_buf_sdram = 0x%x, ret = 0x%x\n", (u32)dma_tx_buf_sdram, ret);
        printf("dma_rx_buf_sdram = 0x%x, ret = 0x%x\n", (u32)dma_rx_buf_sdram, ret);
        ASSERT(0);
    }
}
static void dma_ram0_to_ram0(void)
{
    int ret = 0;

    for (int i = 0; i < sizeof(dma_tx_buf_ram) - 2; i++) {
        dma_tx_buf_ram[i] = (u8)JL_RAND->R64L;
    }
    ret = CRC16(dma_tx_buf_ram, sizeof(dma_tx_buf_ram) - 2);
    dma_tx_buf_ram[sizeof(dma_tx_buf_ram) - 2] = (ret >> 8 & 0x00ff);
    dma_tx_buf_ram[sizeof(dma_tx_buf_ram) - 1] = (ret & 0x00ff);
    memset(dma_rx_buf_ram, 0, sizeof(dma_rx_buf_ram));
    dma_copy(dma_rx_buf_ram, dma_tx_buf_ram, sizeof(dma_tx_buf_ram));
    ret = CRC16(dma_rx_buf_ram, sizeof(dma_tx_buf_ram));
    if (ret) {
        printf("dma_ram0_to_ram0 ret = 0x%x  \n", ret);
        ASSERT(0);
    }
}

static void dma_ram0_to_sdram(void)
{
    int ret = 0;
    int size  = MIN(sizeof(dma_tx_buf_ram), sizeof(dma_rx_buf_sdram));
    for (int i = 0; i < size - 2; i++) {
        dma_tx_buf_ram[i] = (u8)JL_RAND->R64L;
    }
    ret = CRC16(dma_tx_buf_ram, size - 2);
    dma_tx_buf_ram[size - 2] = (ret >> 8 & 0x00ff);
    dma_tx_buf_ram[size - 1] = (ret & 0x00ff);

    memset(dma_rx_buf_sdram, 0, size);
    dma_copy(dma_rx_buf_sdram, dma_tx_buf_ram, size);
    ret = CRC16(dma_rx_buf_sdram, size);
    if (ret) {
        printf("dma_ram0_to_sdram ret = 0x%x  \n", ret);
    }
}

static void dma_sdram_to_ram0(void)
{
    int ret = 0;
    int size  = MIN(sizeof(dma_tx_buf_sdram), sizeof(dma_rx_buf_ram));
    for (int i = 0; i < size - 2; i++) {
        dma_tx_buf_sdram[i] = (u8)JL_RAND->R64H;
    }
    ret = CRC16(dma_tx_buf_sdram, size - 2);
    dma_tx_buf_sdram[size - 2] = (ret >> 8 & 0x00ff);
    dma_tx_buf_sdram[size - 1] = (ret & 0x00ff);

    memset(dma_rx_buf_ram, 0, size);
    dma_copy(dma_rx_buf_ram, dma_tx_buf_sdram, size);
    ret = CRC16(dma_rx_buf_ram, size);
    if (ret) {
        printf("dma_sdram_to_ram0 ret = 0x%x  \n", ret);
        ASSERT(0);
    }
}

static void dma_sdram_to_sdram_nocache(void)
{
    int ret = 0;

    for (int i = 0; i < sizeof(dma_tx_buf_sdram) - 2; i++) {
        dma_tx_buf_sdram[i] = (u8)JL_RAND->R64H;
    }
    ret = CRC16(dma_tx_buf_sdram, sizeof(dma_tx_buf_sdram) - 2);
    dma_tx_buf_sdram[sizeof(dma_tx_buf_sdram) - 2] = (ret >> 8 & 0x00ff);
    dma_tx_buf_sdram[sizeof(dma_tx_buf_sdram) - 1] = (ret & 0x00ff);

    memset(dma_rx_buf_sdram, 0, sizeof(dma_rx_buf_sdram));

    flush_dcache(dma_tx_buf_sdram, sizeof(dma_tx_buf_sdram));
    flushinv_dcache(dma_rx_buf_sdram, sizeof(dma_rx_buf_sdram));
    dma_copy((void *)NO_CACHE_ADDR(dma_rx_buf_sdram), (void *)NO_CACHE_ADDR(dma_tx_buf_sdram), sizeof(dma_tx_buf_sdram));

    ret = CRC16(dma_rx_buf_sdram, sizeof(dma_rx_buf_sdram));
    if (ret) {
        printf("dma_sdram_to_sdram_nocache\n");
        printf("dma_tx_buf_sdram = 0x%x, ret = 0x%x\n", (u32)dma_tx_buf_sdram, ret);
        printf("dma_rx_buf_sdram = 0x%x, ret = 0x%x\n", (u32)dma_rx_buf_sdram, ret);
        put_buf(dma_tx_buf_sdram, sizeof(dma_tx_buf_sdram));
        put_buf(dma_rx_buf_sdram, sizeof(dma_rx_buf_sdram));
        ASSERT(0);
    }
}

static void dma_ram0_to_sdram_nocache(void)
{
    int ret = 0;
    int size  = MIN(sizeof(dma_tx_buf_ram), sizeof(dma_rx_buf_sdram));

    for (int i = 0; i < size - 2; i++) {
        dma_tx_buf_ram[i] = (u8)JL_RAND->R64L;
    }
    ret = CRC16(dma_tx_buf_ram, size - 2);
    dma_tx_buf_ram[size - 2] = (ret >> 8 & 0x00ff);
    dma_tx_buf_ram[size - 1] = (ret & 0x00ff);

    memset(dma_rx_buf_sdram, 0, size);

    flushinv_dcache(dma_rx_buf_sdram, size);
    dma_copy((void *)NO_CACHE_ADDR(dma_rx_buf_sdram), dma_tx_buf_ram, size);

    ret = CRC16(dma_rx_buf_sdram, size);
    if (ret) {
        printf("dma_ram0_to_sdram_nocache ret = 0x%x  \n", ret);
    }
}

static void dma_sdram_to_ram0_nocache(void)
{
    int ret = 0;
    int size  = MIN(sizeof(dma_tx_buf_sdram), sizeof(dma_rx_buf_ram));

    for (int i = 0; i < size - 2; i++) {
        dma_tx_buf_sdram[i] = (u8)JL_RAND->R64H;
    }
    ret = CRC16(dma_tx_buf_sdram, size - 2);
    dma_tx_buf_sdram[size - 2] = (ret >> 8 & 0x00ff);
    dma_tx_buf_sdram[size - 1] = (ret & 0x00ff);

    memset(dma_rx_buf_ram, 0, size);

    flush_dcache(dma_tx_buf_sdram, size);
    dma_copy(dma_rx_buf_ram, (void *)NO_CACHE_ADDR(dma_tx_buf_sdram), size);

    ret = CRC16(dma_rx_buf_ram, size);
    if (ret) {
        printf("dma_sdram_to_ram0_nocache ret = 0x%x  \n", ret);
        ASSERT(0);
    }
}
#endif

static void sdram_test_task(void *p)
{
    u32 cnt = 0;
    int res;
    int msg[32];
    struct intent it = {0};

    os_time_dly(500);

    /* 1 DMA COPY test */
    printf("........................ DMA COPY test ........................ \n\n");
    while (1) {
        printf("DMA COPY test cnt:%d\n", cnt++);
#if DMA_COPY_TEST
#if DMA_COPY_TEST_SDRAM_TO_SDRAM_EN
        dma_sdram_to_sdram();
#endif
#if DMA_COPY_TEST_SDRAM_TO_SDRAM_NOCACH_EN
        dma_sdram_to_sdram_nocache();
#endif
#if DMA_COPY_TEST_SDRAM_TO_SRAM_EN
        dma_sdram_to_ram0();
        dma_sdram_to_ram0_nocache();
#endif
#if DMA_COPY_TEST_SRAM_TO_SRAM_EN
        dma_ram0_to_ram0();
#endif
#if DMA_COPY_TEST_SRAM_TO_SDRAM_EN
        dma_ram0_to_sdram();
#endif
#if DMA_COPY_TEST_SRAM_TO_SDRAM_NOCACH_EN
        dma_ram0_to_sdram_nocache();
#endif
#endif
        os_time_dly(1);
    }
}

static int sdram_test_init()
{
    return thread_fork("sdram_test", 15, 1500, 64, 0, sdram_test_task, NULL);
}
#if DMA_COPY_TEST
late_initcall(sdram_test_init);
#endif

