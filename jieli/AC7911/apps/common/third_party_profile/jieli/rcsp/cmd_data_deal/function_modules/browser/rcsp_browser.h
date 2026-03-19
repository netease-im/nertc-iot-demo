#ifndef __RCSP_BROWSER_H__
#define __RCSP_BROWSER_H__

#include "typedef.h"
#include "app_config.h"

///rcsp 对应的设备id枚举
enum {
    BS_UDISK = 0,
    BS_SD0,
    BS_SD1,
    BS_FLASH,
    BS_AUX,
    BS_FLASH_2,
    BS_DEV_MAX,
};

//rcsp浏览设备映射
char *rcsp_browser_dev_remap(u32 index);
//rcsp获取文件浏览后缀配置
char *rcsp_browser_file_ext(void);
//rcsp获取文件浏览后缀配置数据长度
u16 rcsp_browser_file_ext_size(void);
//rcsp限制文件名长度（包括文件夹,如：xxx~~~, 省略以"~~~"表示）
u16 file_name_cut(u8 *name, u16 len, u16 len_limit);
//rcsp文件浏览开始
void rcsp_browser_start(u8 *data, u16 len);
//rcsp文件浏览停止
void rcsp_browser_stop(void);
//rcsp文件浏览繁忙查询
bool rcsp_browser_busy(void);

#endif // __RCSP_BROWSER_H__

