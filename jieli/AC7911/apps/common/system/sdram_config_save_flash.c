#include "system/includes.h"
#include "device/ioctl_cmds.h"
#include "asm/sfc_norflash_api.h"
#include "app_config.h"
#include "boot.h"

extern SDRAM_CFG_INFO sdram_info;
static void config_save_flash(void)
{
    SDRAM_CFG_INFO *info = (struct sdram_config_info *)&sdram_info;
    if (info->sdram_config_val && info->sdram_config_addr && (JL_SDR->CON0 & BIT(11))) {
        //1.打开flash设备
        u32 flash_val;
        u32 addr = info->sdram_config_addr;
        u32 val = info->sdram_config_val;
        printf("sdram_config_save_flash addr = 0x%x , 0x%x \n", addr, val);
        norflash_open(NULL, NULL, NULL);
        norflash_protect_suspend();//解除flash写保护
        norflash_write(NULL, &val, 4, addr);
        norflash_protect_resume();
        norflash_origin_read(&flash_val, addr, 4);
        if (flash_val != val) {
            printf("err in sdram_config_save_flash 0x%x 0x%x\n", flash_val, val);
            return;
        }
        printf("sdram_config save ok\n");
    }
}
late_initcall(config_save_flash);


