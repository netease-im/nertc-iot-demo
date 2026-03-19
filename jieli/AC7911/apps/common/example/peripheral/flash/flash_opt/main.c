#include "app_config.h"
#include "system/includes.h"
#include "fs/fs.h"
#include "asm/sfc_norflash_api.h"

#ifdef USE_FLASH_OPT_TEST_DEMO

static int c_main(void)
{
    printf("norflash_otp test \n");

    int norflash_eraser_otp(void);
    int norflash_write_otp(u8 * buf, const int len, const int zone_len);
    int norflash_read_otp(u8 * buf, const int len, const int zone_len);

    u8 buf[256];
    for (int i = 0; i < 256; i++) {
        buf[i] = i;
    }
    norflash_eraser_otp();//擦除
    norflash_write_otp(buf, 256, 256);//写，读写最大字节：768
    memset(buf, 0, 256);
    norflash_read_otp(buf, 256, 256);//读，读写最大字节：768
    put_buf(buf, 256);
    return 0;
}
late_initcall(c_main);
#endif
