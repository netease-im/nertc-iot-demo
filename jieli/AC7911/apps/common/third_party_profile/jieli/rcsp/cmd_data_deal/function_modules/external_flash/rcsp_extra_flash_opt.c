#include "rcsp_config.h"

#include "ble_rcsp_module.h"
#include "rcsp_update.h"

#include "rcsp_extra_flash_opt.h"
#include "rcsp_extra_flash_cmd.h"
/* #include "norflash.h" *////RCSP TODO
#include "syscfg_id.h"
#include "update.h"

#include "rcsp_task.h"

#if (RCSP_MODE && JL_RCSP_EXTRA_FLASH_OPT)

#include "ui/ui_api.h"
#include "ui/res_config.h"

#define RCSP_DEBUG_EN
#ifdef RCSP_DEBUG_EN
#define rcsp_putchar(x)                putchar(x)
#define rcsp_printf                    printf
#define rcsp_printf_buf(x,len)         put_buf(x,len)
#else
#define rcsp_putchar(...)
#define rcsp_printf(...)
#define rcsp_printf_buf(...)
#endif

#define EXTRA_FLASH_OPT_WRITE_VERIFY		1

// 一次传输256个字节
#define EXTRA_FLASH_RESP_MTU			(256)
#define EXTRA_FLASH_REVC_MTU			(256)
#define EXTRA_FLASH_PROTOCOL_VERSION	(0)

// 系统类型：
enum SYSTEM_TYPE {
    SYSTEM_TYPE_FAT = 0,
    SYSTEM_TYPE_RCSP = 1,
};

// 当前系统状态：
enum CURR_SYSTEM_STATE {
    CURR_SYSTEM_STATE_NORMAL,
    CURR_SYSTEM_STATE_WRITE_ERR,
};

// 读/写操作
enum CURR_SYSTEM_OPT {
    CURR_SYSTEM_OPT_WRITE = 0,
    CURR_SYSTEM_OPT_READ = 1,
    CURR_SYSTEM_OPT_INSERT = 2,
    CURR_SYSTEM_OPT_DIAL = 3,
    CURR_SYSTEM_OPT_ERASE = 4,
    CURR_SYSTEM_OPT_DELETE = 5,
    CURR_SYSTEM_OPT_UPDATE_FAT = 6,
    CURR_SYSTEM_OPT_UPDATE_UI = 7,
    CURR_SYSTEM_OPT_TRAN_REPLY = 8,
    CURR_SYSTEM_OPT_UPDATE_FLAG = 9,
    CURR_SYSTEM_OPT_RESTORE = 0xA,
    CURR_SYSTEM_OPT_FILE_INFO_GET = 0xB,
    CURR_SYSTEM_OPT_REMAIN_SPACE = 0xC,
};

// 第一个byte用来存放操作结果
static u32 g_cluster_size = 0;
static u8 g_eflash_state_flag = 0;
static u16 g_curr_data_crc16 = 0;

static void *g_eflash_hdl = NULL;
extern int _norflash_read(u32 addr, u8 *buf, u32 len, u8 cache);
extern int _norflash_write(u32 addr, void *buf, u32 len, u8 cache);
extern u32 virfat_flash_read_watch(void *buf, u32 addr_sec, u32 len);
extern u32 virfat_flash_write_watch(void *buf, u32 addr_sec, u32 len);
extern u32 virfat_flash_erase_watch(int cmd, u32 arg);
extern u32 virfat_flash_capacity(); //fat上容量
extern u32 virfat_flash_get_real_capacity(); //获取实际flash容量
extern char *watch_get_cur_path();
extern int watch_get_cur_path_len();
extern char *watch_get_root_path();
extern int watch_set_style_by_name(char *name);
extern int watch_add_item(char *s);
extern int watch_del_item(char *s);
extern void watch_update_finish();
extern int watch_set_init();
extern int watch_get_update_status();
extern const char *WATCH_VERSION_LIST;
extern int watch_get_version(char *watch_item, char *version);
extern int watch_get_uuid(char *watch_item, char *version);
extern int watch_set_background(char *bg_pic);
extern char *watch_get_background();
extern char *watch_get_item(int style);

extern int watch_bgp_set_related(char *bgp, u8 cur_watch, u8 del);
extern int watch_get_style();

extern int watch_bgp_related_del_all(char *bgp);

extern int watch_get_style_by_name(char *name);
extern char *watch_bgp_get_related(u8 cur_watch);
extern void watch_add_item_deal(void);
extern int get_cur_srreen_width_and_height(u16 *screen_width, u16 *screen_height);
#if RCSP_UPDATE_EN
extern u8 get_update_ex_flash_flag(void);
extern void set_update_ex_flash_flag(u8 update_flag);
#endif

#if 0
static int test(void *p)
{
    static int step = 0;
    printf(">>%d \n", step);


    if (step == 1) {
        UI_MSG_POST("upgrade:start=%4", 1);    //开始
    }

    if (step == 2) {
        UI_MSG_POST("upgrade:result=%4", 1);    //成功
    }

    if (step == 3) {
        UI_MSG_POST("upgrade:result=%4", 0);    //失败
    }

    if (step == 4) {
        UI_MSG_POST("upgrade:process=%4", 55);    //进度
    }

    if (step == 5) {
        UI_MSG_POST("upgrade:process=%4", 65);
    }

    step++;

    if (step >= 6) {
        step = 0;
    }
    return 0;
}
#endif
//升级UI表盘开始
static int watch_ui_update_open()
{
    /* ui_auto_shut_down_disable(); */
    /* ui_touch_timer_delete(); */
    /* sys_key_event_disable(); */
#if UI_UPGRADE_RES_ENABLE   //升级界面功能
    UI_WINDOW_PREEMPTION_POSH(ID_WINDOW_UPGRADE, NULL, NULL, UI_WINDOW_PREEMPTION_TYPE_UPGRADE);
    UI_MSG_POST("upgrade:wait=%4", 4);
#endif
    return 0;
}

//升级UI表盘结束
static int watch_ui_update_close()
{
    /* ui_touch_timer_start(); */
    /* ui_auto_shut_down_enable(); */
    /* sys_key_event_enable(); */
#if UI_UPGRADE_RES_ENABLE   //升级界面功能
    UI_HIDE_CURR_WINDOW();
#endif
    return 0;
}


static void watch_ui_update_out()
{
    if (watch_get_update_status() == 2) {
#if RCSP_UPDATE_EN
        if (get_jl_update_flag()) {
            return;
        }
#endif
        rcsp_printf("\n\n\nbegin cpu reset...\n\n\n\n");
        cpu_reset();
    } else {
    }
}

//升级UI表盘中，暂时不显示进度条，让手机APP显示，小机
//只显示升级中...
//flag : 0表示升级操作中 1表示升级操作完成 （升级操作包括增加文件和删除文件）
static int watch_ui_update(u8 flag)
{
    return 0;
}


//*----------------------------------------------------------------------------*/
/**@brief    表盘升级任务
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void rcsp_eflash_change_sys_clock(u8 flag)
{
    extern int clk_get(const char *name);
    extern int clk_set(const char *name, int clk);
    static u32 last_sys_hz = 0;
    last_sys_hz = clk_get("sys");
    if (flag) {
        clk_set("sys", 192 * 1000000);
    } else {
        clk_set("sys", last_sys_hz);
    }
}

void bt_direct_init();
int bt_update_event_handler(struct sys_event *event);
void app_watch_ui_updata_task()
{
    int msg[32];

    rcsp_printf("\n\n\nwatch ui update task start\n\n\n\n");

    if (watch_get_update_status() == 2) {
        bt_direct_init();
        rcsp_bt_ble_init();
    }

    rcsp_eflash_change_sys_clock(1);
    watch_ui_update_open();

    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);

        switch (msg[0]) {
        case APP_MSG_SYS_EVENT:
            if (watch_get_update_status() == 2) {
                // 可以相应ota升级的事件
                if (bt_update_event_handler((struct sys_event *)(msg + 1)) == false) {
                    app_default_event_deal((struct sys_event *)(&msg[1]));
                }
            } else {
                app_default_event_deal((struct sys_event *)(&msg[1]));
            }
            break;
        default:
            break;
        }

        if (app_task_exitting()) {
            rcsp_printf("\n\n\nwatch ui update task exit\n\n\n\n");
            rcsp_eflash_change_sys_clock(0);
            watch_ui_update_close();
            return;
        }
    }
}

void rcsp_extra_flash_init(void)
{
    rcsp_printf("\n\n\nwatch ui update task start\n\n\n\n");

    if (watch_get_update_status() == 2) {
        bt_direct_init();
        rcsp_bt_ble_init();
    }

    watch_ui_update_open();
    rcsp_eflash_change_sys_clock(1);
}

int rcsp_extra_flash_event_deal(struct sys_event *event)
{
    if (watch_get_update_status() == 2) {
        if (bt_update_event_handler(event) == false) {
            app_default_event_deal(event);
        }
    } else {
        app_default_event_deal(event);
    }
    return true;
}

void rcsp_extra_flash_close(void)
{
    rcsp_printf("\n\n\nwatch ui update task exit\n\n\n\n");
    watch_ui_update_close();
    rcsp_eflash_change_sys_clock(0);
}

//*----------------------------------------------------------------------------*/
/**@brief    结束时还原界面操作
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
u8 get_bt_init_status(void);
u8 get_rcsp_connect_status(void);
static void rcsp_extra_flash_update_end(void)
{
    if (!rcsp_eflash_update_flag_get()) {
        watch_add_item_deal();
    }

    if (get_bt_init_status()) {
        app_task_switch_back();
    } else {
        rcsp_printf("exflash opt after poweroff and entry exception mode\n");
    }
    if (!get_rcsp_connect_status()) {
        watch_ui_update_out();
        watch_update_finish();
    }
}

//*----------------------------------------------------------------------------*/
/**@brief    第一次交互时app获取的外部flash的大小
   @param
   @return   flash大小
   @note
*/
/*----------------------------------------------------------------------------*/
static u32 rcsp_extra_flash_size_get(void)
{
    return virfat_flash_get_real_capacity(); //获取实际flash容量
}

//
//*----------------------------------------------------------------------------*/
/**@brief    第一次交互时app获取的扇区大小
   @param
   @return   扇区大小
   @note
*/
/*----------------------------------------------------------------------------*/
static u32 rcsp_extra_flash_sector_get(void)
{
    return 4096;
}

//*----------------------------------------------------------------------------*/
/**@brief    第一次交互时app获取的fat系统实际大小
   @param
   @return   fat容量
   @note
*/
/*----------------------------------------------------------------------------*/
static u32 rcsp_extra_fat_size(void)
{
    return virfat_flash_capacity(); //fat上容量
}

//*----------------------------------------------------------------------------*/
/**@brief    第一次交互时app获取表盘列表字符串信息的大小
   @param
   @return   版本号信息长度
   @note
*/
/*----------------------------------------------------------------------------*/
static u16 rcsp_eflash_version_info_size(void)
{
    return strlen(WATCH_VERSION_LIST);
}

//*----------------------------------------------------------------------------*/
/**@brief    第一次交互是app获取表盘列表字符串信息
   @param    ver_info:版本号获取(出参)，ver_info_size:版本号信息长度
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void rcsp_eflash_version_info_get(u8 *ver_info, u16 ver_info_size)
{
    // 字符串以逗号作为分隔符，例如: w0,w1,w2,w3,w4,w5
    memcpy(ver_info, WATCH_VERSION_LIST, ver_info_size);
}

//*----------------------------------------------------------------------------*/
/**@brief    读操作
   @param    buffer:数据，buffer_size:数据长度，offset:偏移地址，opt_state:当前操作状态
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_read(u8 *buffer, u16 buffer_size, u32 offset, u8 opt_state)
{
    rcsp_printf("rcsp_extra_flash_opt_read, buffer_size = %x, offset = %x\n", buffer_size, offset);
    if (2 == opt_state) {
        // 开始读
    } else if (1 == opt_state) {
        // 停止读
    }

    rcsp_printf("---efread begin %x, %x\n", offset, buffer_size);
    virfat_flash_read_watch(buffer, offset, buffer_size);

    rcsp_printf("---efread\n");
    rcsp_printf_buf(buffer, buffer_size);

    return 0;
}

static char watch_name[16];
static u32 g_watch_file_size = 0;

#define FLASH_WRITE_RESP		0
#if FLASH_WRITE_RESP
static u8 *g_fbuf = NULL;
static u16 g_foffset = 0;
static u32 g_faddr = 0;
#endif

//*----------------------------------------------------------------------------*/
/**@brief    擦除数据操作
   @param    erase_addr:起始地址，erase_unit_size:擦除大小
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_erase(u32 erase_addr, u32 erase_unit_size)
{
    rcsp_printf("erase_addr %x, erase_unit_size %x\n", erase_addr, erase_unit_size);
    int result = 0;
    u32 cmd = IOCTL_ERASE_SECTOR;

    if (erase_unit_size == 4096) {
        cmd = IOCTL_ERASE_SECTOR;
    }

    result = virfat_flash_erase_watch(cmd, erase_addr);
    if (result != 0) {
        printf("virfat erase err\n");
    }

    return result;
}

#if FLASH_WRITE_RESP
//*----------------------------------------------------------------------------*/
/**@brief    写操作
   @param    buffer:数据，buffer_size:数据长度，offset:偏移地址，opt_state:当前操作状态
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_write(u8 *buffer, u16 buffer_size, u32 offset, u8 opt_state)
{
    u32 ret;
    rcsp_printf("rcsp_extra_flash_opt_write, buffer_size = %x, offset = %x, g_foffset = %x\n", buffer_size, offset, g_foffset);

    ASSERT((g_foffset + buffer_size <= 4096), "overflow 4k cluster, g_foffset %x\n", g_foffset);
    if (2 == opt_state) {
        // 开始写
        rcsp_printf("\n\nwbgin\n\n");
        ASSERT(((offset % 4096) == 0), "not aligned 4k addr, offset %x\n", offset);
        g_foffset = 0;
        g_faddr = offset;
        rcsp_extra_flash_opt_erase(offset, g_cluster_size);
    }

    memcpy(g_fbuf + g_foffset, buffer, buffer_size);
    g_foffset += buffer_size;

    if (1 == opt_state) {
        // 停止写
        rcsp_printf("\n\nwend\n\n");
        ret = virfat_flash_write_watch(g_fbuf, g_faddr, g_foffset);

        ASSERT((ret == g_foffset), "virfat werr, ret %x, g_foffset %x\n", ret, g_foffset);
#if EXTRA_FLASH_OPT_WRITE_VERIFY
        // 把写进去的数据读出来校验
        u16 write_crc16 = CRC16(g_fbuf, g_foffset);
        virfat_flash_read_watch(g_fbuf, g_faddr, g_foffset);
        u16 read_crc16 = CRC16(g_fbuf, g_foffset);
        if (write_crc16 != read_crc16) {
            rcsp_printf("write_err : g_faddr %x, g_foffset %x\n", g_faddr, g_foffset);
            rcsp_printf_buf(g_fbuf, g_foffset);
            // 告诉app当前传输错误
            return 1;
        }
#endif
        rcsp_printf("bend\n");
        g_foffset = 0;
    }

    return 0;
}
#endif

//*----------------------------------------------------------------------------*/
/**@brief    校验写入flash数据是否正确
   @param    buffer:数据，buffer_size:数据长度，offset:偏移地址
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_write_check(u8 *buffer, u16 buffer_size, u32 offset)
{
    int ret = 0;
    if (0 == buffer_size) {
        return ret;
    }
    u16 crc_flash_data = 0;
    u16 crc_write_data = CRC16(buffer, buffer_size);
    virfat_flash_read_watch(buffer, offset, buffer_size);
    crc_flash_data = CRC16(buffer, buffer_size);
    if (crc_write_data != crc_flash_data) {
        printf("rcsp_extra_flash_write_check err, crc_write_data : %x, crc_flash_data : %x\n", crc_write_data, crc_flash_data);
        ret = -1;
    }
    return 0;
}

//*----------------------------------------------------------------------------*/
/**@brief    写操作，不回复
   @param    buffer:数据，buffer_size:数据长度，offset:偏移地址
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_write_no_resp(u8 *buffer, u16 buffer_size, u32 offset)
{
    rcsp_printf("rcsp_extra_flash_write_no_resp, buffer_size = %x, offset = %x\n", buffer_size, offset);
    static int result = 0;
    if (!buffer_size) {
        int ret = result;
        result = 0;
        return ret;
    }
    if (result) {
        return result;
    }
    if (0 == (offset % g_cluster_size)) {
        result = rcsp_extra_flash_opt_erase(offset, g_cluster_size);
    }

    if (buffer_size != virfat_flash_write_watch(buffer, offset, buffer_size)) {
        result = -1;
    } else if (rcsp_extra_flash_write_check(buffer, buffer_size, offset)) {
        result = -1;
    } else {
        printf("===================%s======%d=========yuring==\n\r", __func__, __LINE__);
#if 0 ///RCSP TODO
        g_curr_data_crc16 = CRC16_with_initval(buffer, buffer_size, g_curr_data_crc16);
#endif
    }
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    插入文件开始操作
   @param    file_name:文件名，file_name_len:文件名长度，file_size:当前文件总大小
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_insert_start(u8 *file_name, u16 file_name_len, u32 file_size)
{
    rcsp_printf("file_size %x, file_name %s, file_name_len %d", file_size, file_name, file_name_len);
    int result = 0;
#if FLASH_WRITE_RESP
    if (g_fbuf == NULL) { // 防止内存泄漏
        g_fbuf = zalloc(g_cluster_size); // g_cluster_size 暂时为4096, 可根据rcsp_extra_flash_sector_get函数进行改变

        ASSERT((g_fbuf != NULL), "opt_flash malloc err, g_fbuf is NULL, line %d\n", __LINE__);
    }
    g_foffset = 0;
    g_faddr = 0;
#endif


    ASSERT((file_name_len < sizeof(watch_name)), "err name0 , file_name_len %x, sizeof(watch_name) %x\n", file_name_len, sizeof(watch_name));

    strcpy(watch_name, file_name);
    watch_name[strlen(file_name)] = '\0';
    g_watch_file_size = file_size;
    watch_ui_update(0);

    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    插入文件结束操作
   @param    erase_addr:起始地址，erase_unit_size:擦除大小
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_insert_stop(void)
{
    rcsp_printf("extra_flash_opt_insert_stop\n");
    int result = 0;

#if FLASH_WRITE_RESP
    if (g_fbuf) {
        free(g_fbuf);
        g_fbuf = NULL;
    }
    g_foffset = 0;
    g_faddr = 0;
#endif

    watch_ui_update(1);

    char *watch_ptr = watch_name;
    if ('/' == watch_ptr[0]) {
        watch_ptr++;
    }
    if (0 == strncmp(watch_ptr, "watch", strlen("watch")) ||
        0 == strncmp(watch_ptr, "WATCH", strlen("WATCH"))) {
        watch_add_item(watch_name);
    }


    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    表盘设置操作
   @param    file_path:文件路径(也用作出参)，file_path_len:路径长度
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_dial_set(u8 *file_path, u16 *data_len)
{
    rcsp_printf("file_path %s, file_path_len %d\n", file_path, *data_len);
    int result = 0;
    char *root_path = watch_get_root_path();
    char watch[64];

    ASSERT(((strlen(root_path) + strlen(file_path) + 2) < sizeof(watch)), "err name2, (strlen(root_path) + strlen(file_path) + 2) : %d, sizeof(watch) : %d\n\n", (strlen(root_path) + strlen(file_path) + 2), sizeof(watch));

    ASCII_ToLower(file_path, strlen(file_path));
    strcpy(watch, root_path);
    strcpy(&watch[strlen(root_path)], &file_path[1]);
    if (watch[strlen(watch) - 1] != '/') {
        strcat(&watch[strlen(root_path)], "/");//解决下面表盘匹配，watch 和 watch1半匹配撞车
    }

    /* rcsp_printf("%s\n", watch); */

    result = watch_set_style_by_name(watch);


#if  (TCFG_UI_ENABLE )
    if (UI_GET_WINDOW_ID() == PAGE_0) {
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(PAGE_0);
    } else {
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_UPGRADE);
    }
#endif

    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    表盘背景操作
   @param    file_path:文件路径(也用作出参)，file_path_len:路径长度
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_dial_backgroud_set(u8 *file_path, u16 *data_len)
{
    rcsp_printf("rcsp_extra_flash_opt_dial_backgroud_set, %s\n", file_path);
    int result = 0;
    int style = watch_get_style();
    if (0 == strcmp(file_path, "/null")) {
        watch_bgp_set_related(NULL, style, 0);
    } else {
        watch_bgp_set_related(file_path + 1, style, 0);
    }


#if  (TCFG_UI_ENABLE )
    if (UI_GET_WINDOW_ID() == PAGE_0) {
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(PAGE_0);
    } else {
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_UPGRADE);
    }
#endif
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    通过路径获取表盘背景
   @param    file_path:文件路径(也用作出参)，file_path_len:路径长度
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_dial_backgroud_get(u8 *data, u8 *file_path, u16 *data_len)
{
    rcsp_printf("rcsp_extra_flash_opt_dial_backgroud_get, %s\n", file_path);
    int result = 0;
    char *root_path = watch_get_root_path();
    char watch[64] = {0};
    ASSERT(((strlen(root_path) + strlen(file_path) + 2) < sizeof(watch)), "err name2, (strlen(root_path) + strlen(file_path) + 2) : %d, sizeof(watch) : %d\n\n", (strlen(root_path) + strlen(file_path) + 2), sizeof(watch));

    ASCII_ToLower(file_path, strlen(file_path));
    strcpy(watch, root_path);
    strcpy(&watch[strlen(root_path)], &file_path[1]);

    int watch_item = watch_get_style_by_name(watch);
    if (watch_item < 0) {
        // 找不到
        memcpy(data, "null", sizeof("null"));
    } else {
        char *bgp = watch_bgp_get_related(watch_item);
        if (bgp) {
            memcpy(data, bgp, strlen(bgp) + 1);
        } else {
            memcpy(data, "null", sizeof("null"));
        }
    }
    *data_len = strlen(data) + 1;

    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    获取表盘版本号操作
   @param    file_path:文件路径(也用作出参)，file_path_len:路径长度
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_dial_version_get(u8 *data, u8 *file_path, u16 *data_len)
{
    int ret = 0;
    rcsp_printf("rcsp_extra_flash_opt_dial_version_get, %s\n", file_path);

    char *root_path = watch_get_root_path();
    char watch[64] = {0};
    ASSERT(((strlen(root_path) + strlen(file_path) + 2) < sizeof(watch)), "err name2, (strlen(root_path) + strlen(file_path) + 2) : %d, sizeof(watch) : %d\n\n", (strlen(root_path) + strlen(file_path) + 2), sizeof(watch));
    ASCII_ToLower(file_path, strlen(file_path));
    strcpy(watch, root_path);
    /* strcpy(watch + strlen(watch), "watch"); */
    strcpy(watch + strlen(watch), file_path + 1);
    strcpy(watch + strlen(watch), file_path);
    strcpy(watch + strlen(watch), ".sty");

    u32 offset = 0;

    // 获取version
    ret = watch_get_version(watch, data + offset);
    if (!ret) {
        offset += strlen(data + offset) + 1;
    }

    // 获取uuid
    ret = watch_get_uuid(watch, data + offset);
    if (!ret) {
        offset += strlen(data + offset) + 1;
    }

    // 填充分隔符
    for (u8 i = 0; i < offset; i++) {
        if ((i + 1 != offset) && 0 == data[i]) {
            data[i] = ',';
        }
    }
    *data_len = offset;
    return 0;
}

//*----------------------------------------------------------------------------*/
/**@brief    表盘读取操作，获取表盘文件路径的大小
   @param
   @return   路径的大小
   @note
*/
/*----------------------------------------------------------------------------*/
static u16 rcsp_extra_flash_opt_dial_get_file_path_size(void)
{
    return watch_get_cur_path_len();
}

//*----------------------------------------------------------------------------*/
/**@brief    表盘读取操作，获取表盘文件路径
   @param    file_path:文件路径，file_path_len:路径长度
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_dial_get_file_path(u8 *file_path, u16 file_path_size)
{
    int result = 0;
    // 获取file_path并填充到file_path中
    /* memcpy(file_path, watch_get_cur_path(), file_path_size - 1); */
    /* file_path[file_path_size - 1] = '\0'; */

    memcpy(file_path, watch_get_cur_path(), file_path_size);

    rcsp_printf("test get pathname %d, %s\n\n\n\n", file_path_size, file_path);

    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    删除文件开始操作
   @param    file_path:文件路径，file_path_len:路径长度
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_delete_start(u8 *file_path, u16 file_path_len)
{
    rcsp_printf("file_path %s, file_path_len %d\n", file_path, file_path_len);
    /* app_task_switch_to(APP_WATCH_UPDATE_TASK); */
    int result = 0;

#if FLASH_WRITE_RESP
    if (g_fbuf == NULL) {	// 防止内存泄漏
        g_fbuf = zalloc(g_cluster_size); // g_cluster_size 暂时为4096,可根据rcsp_extra_flash_sector_get函数进行改变

        ASSERT((g_fbuf != NULL), "opt_flash malloc err, g_fbuf is NULL, line %d\n", __LINE__);
        /* if (g_fbuf == NULL) { */
        /* rcsp_printf("opt_flash malloc err, g_fbuf is NULL, line %d\n", __LINE__); */
        /* while (1); */
        /* } */
    }
    g_foffset = 0;
    g_faddr = 0;
#endif


    ASSERT((file_path_len < sizeof(watch_name)), "err name1 file_path_len : %x, sizeof(watch_name) : %x, line : %d\n", file_path_len, sizeof(watch_name), __LINE__);
    /* if (file_path_len >= sizeof(watch_name)) { */
    /* rcsp_printf("err name1 file_path_len : %x, sizeof(watch_name) : %x, line : %d\n", file_path_len, sizeof(watch_name), __LINE__); */
    /* while (1); */
    /* } */
    strcpy(watch_name, file_path);
    watch_name[strlen(file_path)] = '\0';

    watch_ui_update(0);

    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    删除文件结束操作
   @param
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int samrtbox_extra_flash_opt_delete_end(void)
{
    rcsp_printf("samrtbox_extra_flash_opt_delete_end\n");
    int result = 0;
#if FLASH_WRITE_RESP
    if (g_fbuf) {
        free(g_fbuf);
        g_fbuf = NULL;
    }
    g_foffset = 0;
    g_faddr = 0;
#endif

    watch_del_item(watch_name);

    watch_ui_update(1);

    if (0 == strncmp(watch_name + 1, "bgp_w", strlen("bgp_w")) ||
        0 == strncmp(watch_name + 1, "BGP_W", strlen("BGP_W"))) {
        watch_bgp_related_del_all(watch_name + 1);
    }

    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    数据填充操作，包含大小端转换
   @param    desc:获取结果存放的buffer，src:数据源，len:数据长度
   @return   数据长度
   @note
*/
/*----------------------------------------------------------------------------*/
static u16 rcsp_extra_flash_opt_data_fill(u8 *desc, u8 *src, u8 len)
{
    u16 offset = 0;
    for (; offset < len; offset++) {
        desc[offset] = src[len - offset - 1];
    }
    return offset;
}

//*----------------------------------------------------------------------------*/
/**@brief    获取当前表盘操作
   @param    buffer:获取结果存放的buffer，data_len:获取结果的长度(出参)
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_dial_get(u8 *buffer, u16 *data_len)
{
    rcsp_printf("rcsp_extra_flash_opt_dial_get\n");
    int result = 0;
    u16 offset = 0;
    u8 *file_path = NULL;
    *data_len = 0;
    // 表盘读操作回复app需要的内容：file_path_size(2) + file_path(n)

    // file_path_size
    u16 file_path_size = rcsp_extra_flash_opt_dial_get_file_path_size();

    // file_path
    if (file_path_size) {
        file_path = zalloc(file_path_size);
        result = rcsp_extra_flash_opt_dial_get_file_path(file_path, file_path_size);
        if (result) {
            goto __end;
        }
        memcpy(buffer + offset, file_path, file_path_size);
        offset += file_path_size;
    }

    //
    *data_len = offset;
__end:
    if (file_path) {
        free(file_path);
    }
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    fat表更新开始操作
   @param
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_update_fat_begin(void)
{
    rcsp_printf("rcsp_extra_flash_opt_update_fat_begin\n");
    int result = 0;
    rcsp_eflash_flag_set(1);
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    fat表更新结束操作
   @param
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_update_fat_end(void)
{
    rcsp_printf("rcsp_extra_flash_opt_update_fat_end\n");
    int result = 0;
    rcsp_eflash_flag_set(0);
    if (g_eflash_state_flag) {
        // 异常回复完后会触发
        g_eflash_state_flag	= 0;
        watch_set_init();
    }
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    ui操作
   @param    state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_opt_update_ui_opt(u8 state)
{
    rcsp_printf("rcsp_extra_flash_opt_update_ui_opt\n");
    int result = 0;
    // 结束时还原界面
    switch (state) {
    case 1: // 开始时进入等待升级
        /* app_task_switch_to(APP_WATCH_UPDATE_TASK); */
        app_rcsp_task_prepare(0, RCSP_TASK_ACTION_WATCH_TRANSFER, 0);
        break;
    case 0: // 结束时还原界面
        rcsp_extra_flash_update_end();
        break;
    }
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    获取当前写flash的状态，用于写操作
   @param
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_resp_state_to_app(void)
{
    rcsp_printf("rcsp_extra_resp_state_to_app\n");
    return rcsp_extra_flash_write_no_resp(NULL, 0, 0);
}

//*----------------------------------------------------------------------------*/
/**@brief    获取当前一包传输数据的大小
   @param    发送/接收一包期望大小
   @return   传输数据的大小
   @note
*/
/*----------------------------------------------------------------------------*/
static u16 get_communication_mtu(u16 expect_mtu)
{
    u16 curr_mtu = JL_packet_get_tx_max_mtu();
    curr_mtu = (expect_mtu + 16) > curr_mtu ? (curr_mtu - 16) : expect_mtu;
    return curr_mtu;
}

//*----------------------------------------------------------------------------*/
/**@brief    填充表盘操作第一条命令的数据回复包
   @param    resp_data:数据回复包
   @return   负数-失败，正数-数据长度
   @note
*/
/*----------------------------------------------------------------------------*/
int get_extra_flash_info(void *priv, u8 *resp_data)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    int data_len = 0;

    // 获取填充的元素
    u32 flash_size = rcsp_extra_flash_size_get(); // 调用接口获取外挂flash大小，暂时使用512k
    u32 fat_size = rcsp_extra_fat_size();
    u8 sys_type = SYSTEM_TYPE_FAT; // 暂时也只有0
    if (rcspModel->file_transfer_mode) {
        sys_type = SYSTEM_TYPE_RCSP;
    }
    u16 version = EXTRA_FLASH_PROTOCOL_VERSION;
    u16 resp_mtu = get_communication_mtu(EXTRA_FLASH_RESP_MTU);
    g_cluster_size = rcsp_extra_flash_sector_get();
    u16 sector = (u16)(g_cluster_size / 256);
    u16 ver_len = rcsp_eflash_version_info_size();
    u16 recv_mtu = get_communication_mtu(EXTRA_FLASH_REVC_MTU);
    u16 screen_width = 0;
    u16 screen_height = 0;
    get_cur_srreen_width_and_height(&screen_width, &screen_height);

    // 填充
    data_len = 0; // 用作计数
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&flash_size, sizeof(flash_size));
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&fat_size, sizeof(fat_size));
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&sys_type, sizeof(sys_type));

    g_eflash_state_flag = rcsp_eflash_update_flag_get() ? 0 : rcsp_eflash_flag_get();
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&g_eflash_state_flag, sizeof(g_eflash_state_flag));
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&version, sizeof(version));
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&resp_mtu, sizeof(resp_mtu));
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&sector, sizeof(sector));
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&ver_len, sizeof(ver_len));
    rcsp_eflash_version_info_get(resp_data + data_len, ver_len);
    data_len += ver_len;
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&recv_mtu, sizeof(recv_mtu));
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&screen_width, sizeof(screen_width));
    data_len += rcsp_extra_flash_opt_data_fill(resp_data + data_len, (u8 *)&screen_height, sizeof(screen_height));
    return data_len;
}

//*----------------------------------------------------------------------------*/
/**@brief    数据回复包的填充
   @param    resp_data:数据回复包，len:数据长度，opt_flag:操作命令，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
u16 rcsp_extra_flash_opt_resp_data_get(u8 *resp_data, u16 len, u8 opt_flag, u8 state)
{
    u16 rlen = 0;
    switch (opt_flag) {
    case CURR_SYSTEM_OPT_READ:
    case CURR_SYSTEM_OPT_FILE_INFO_GET:
    case CURR_SYSTEM_OPT_REMAIN_SPACE:
        rlen += len;
        break;
    case CURR_SYSTEM_OPT_DIAL:
        switch (state) {
        case 0:
        case 3:
        case 5:
            rlen += len;
            break;
        case 2:
            rlen += len;
            break;
        }
        break;
    case CURR_SYSTEM_OPT_TRAN_REPLY:
        switch (state) {
        case 1:
            rlen += len;
            break;
        }
        break;
    }
    return rlen;
}

//*----------------------------------------------------------------------------*/
/**@brief    外部flash开始操作
   @param
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
int rcsp_extra_flash_opt_start(void)
{
    log_i("\n\n\n\n\napp update ui start\n\n\n\n\n\n");
#if FLASH_WRITE_RESP
    if (g_eflash_state_flag && g_fbuf == NULL) {
        g_fbuf = zalloc(g_cluster_size); // g_cluster_size 暂时为4096, 可根据rcsp_extra_flash_sector_get函数进行改变
        ASSERT((g_fbuf != NULL), "opt_flash malloc err, g_fbuf is NULL, line : %d\n", __LINE__);
    }
#endif
    g_curr_data_crc16 = 0;

    return 0;
}

//*----------------------------------------------------------------------------*/
/**@brief    外部flash结束操作
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void rcsp_extra_flash_opt_stop(void)
{
#if FLASH_WRITE_RESP
    g_foffset = 0;
    g_faddr = 0;
#endif

    log_i("\n\n\n\n\napp update ui stop\n\n\n\n\n\n");
    //如果上次表盘升级异常，直接进入表盘升级模式等待升级完成
    // 加上新标志位的判断，用于升级升到一般时候的情况
    if (rcsp_eflash_flag_get() != 0 ||
        rcsp_eflash_update_flag_get() != 0) {
        rcsp_printf("\n\ngoto watch update mode again\n\n");
        /* app_task_switch_to(APP_WATCH_UPDATE_TASK); */
    } else if (APP_WATCH_UPDATE_TASK == app_get_curr_task() ||
               APP_RCSP_ACTION_TASK == app_get_curr_task()) {
        rcsp_extra_flash_update_end();
    }

}

//*----------------------------------------------------------------------------*/
/**@brief    设置异常状态标志位
   @param    eflash_state_type:标志位
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void rcsp_eflash_flag_set(u8 eflash_state_type)
{
    u8 eflash_flag = -1;
    syscfg_read(VM_EXTRA_FLASH_UPDATE_FLAG, &eflash_flag, sizeof(eflash_flag));
    if (eflash_state_type != eflash_flag) {
        eflash_flag = eflash_state_type;
        syscfg_write(VM_EXTRA_FLASH_UPDATE_FLAG, &eflash_flag, sizeof(eflash_flag));
    }
}

//*----------------------------------------------------------------------------*/
/**@brief    获取异常状态标志位
   @param
   @return   1-处于异常，0-正常
   @note
*/
/*----------------------------------------------------------------------------*/
u8 rcsp_eflash_flag_get(void)
{
    u8 eflash_flag = 0;

    // 如果备份区是全f，那么需要清除异常标志位
    virfat_flash_read_watch(&eflash_flag, 4096, 1);
    if (0xFF == eflash_flag) {
#if RCSP_UPDATE_EN
        if (0 == get_update_ex_flash_flag() && 0 == get_jl_update_flag())
#endif
        {
            rcsp_eflash_flag_set(0);
            return 0;
        }
    }

    if (sizeof(eflash_flag) ==
        syscfg_read(VM_EXTRA_FLASH_UPDATE_FLAG, &eflash_flag, sizeof(eflash_flag))) {
        return eflash_flag;
    }
    return 0;
}

//*----------------------------------------------------------------------------*/
/**@brief    检查当前是否处于表盘升级状态
   @param    param:状态标志
   @return   true-处于升级状态，false-其他状态
   @note
*/
/*----------------------------------------------------------------------------*/
bool rcsp_exflash_flash_opt_check(void *param)
{
    u8 *app = (u8 *) param;
    if (rcsp_eflash_flag_get() ||
        rcsp_eflash_update_flag_get()) {
        *app = 0xff;
        return true;
    }
    return false;
}

//*----------------------------------------------------------------------------*/
/**@brief    获取当前操作状态，用于读/写操作
   @param    state:当前操作状态
   @return   0-读/写结束，2-读/写开始，1-其他
   @note
*/
/*----------------------------------------------------------------------------*/
static u8 rcsp_extra_flash_op_state_get(u8 state)
{
    static u8 opt_state_record = 0;
    if (opt_state_record != state) {
        opt_state_record = state;
    } else {
        return 0;
    }
    return opt_state_record + 1;
}

//*----------------------------------------------------------------------------*/
/**@brief    读数据操作
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_read_opt(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    int result = 0;

    u32 start_addr = (data[offset++] << (8 * 3)) | (data[offset++] << (8 * 2)) | (data[offset++] << (8 * 1)) | data[offset++];
    *data_len = (data[offset++] << (8 * 1)) | data[offset++];

    state = rcsp_extra_flash_op_state_get(state);
    result = rcsp_extra_flash_opt_read(data + 1, *data_len, start_addr, state);
    data[0] = result;

    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    写数据操作
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型，resp_flag:是否需要回复
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_write_opt(u8 *data, u16 *data_len, u8 offset, u8 state, u8 resp_flag)
{
    int result = 0;
    u32 start_addr = (data[offset++] << (8 * 3)) | (data[offset++] << (8 * 2)) | (data[offset++] << (8 * 1)) | data[offset++];
    *data_len -= offset;

    if (resp_flag) {
#if FLASH_WRITE_RESP
        state = rcsp_extra_flash_op_state_get(state);
        result = rcsp_extra_flash_opt_write(data + offset, *data_len, start_addr, state);
#endif
    } else {
        result = rcsp_extra_flash_write_no_resp(data + offset, *data_len, start_addr);
    }
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    表盘插入操作
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_insert_opt(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    int result = 0;
    if (state) {
        u32 start_addr = (data[offset++] << (8 * 3)) | (data[offset++] << (8 * 2)) | (data[offset++] << (8 * 1)) | data[offset++];
        *data_len -= offset;
        data[offset + *data_len] = 0;
        result = rcsp_extra_flash_opt_insert_start(data + offset, *data_len, start_addr);
    } else {
        result = rcsp_extra_flash_opt_insert_stop();
    }
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    表盘操作
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_dial_opt(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    int result = 0;
    if (state) {
        *data_len -= offset;
        data[*data_len + offset] = 0;
    } else {
        *data_len = 0;
    }

    switch (state) {
    case 0:
        result = rcsp_extra_flash_opt_dial_get(data + 1, data_len);
        break;
    case 1:
        result = rcsp_extra_flash_opt_dial_set(data + offset, data_len);
        break;
    case 3:
        result = rcsp_extra_flash_opt_dial_version_get(data + 1, data + offset, data_len);
        break;
    case 4:
        result = rcsp_extra_flash_opt_dial_backgroud_set(data + offset, data_len);
        break;
    case 5:
        result = rcsp_extra_flash_opt_dial_backgroud_get(data + 1, data + offset, data_len);
        break;
    }
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    擦除数据操作
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_erase_opt(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    int result = 0;
    u32 start_addr = (data[offset++] << (8 * 3)) | (data[offset++] << (8 * 2)) | (data[offset++] << (8 * 1)) | data[offset++];
    *data_len = (data[offset++] << (8 * 1)) | data[offset++];
    result = rcsp_extra_flash_opt_erase(start_addr, (*data_len) * 256);
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    删除表盘文件操作
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_delete_opt(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    int result = 0;
    if (state) {
        *data_len -= offset;
        data[offset + *data_len] = 0;
        result = rcsp_extra_flash_opt_delete_start(data + offset, *data_len);
    } else {
        result = samrtbox_extra_flash_opt_delete_end();
    }
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    更新fat表操作
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_update_fat_opt(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    int result = 0;
    switch (state) {
    case 0:
        result = rcsp_extra_flash_opt_update_fat_end();
        break;
    case 1:
        result = rcsp_extra_flash_opt_update_fat_begin();
        break;
    }
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    ui显示操作，升级(插入/删除/覆盖)操作(开始/结束)触发
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_update_ui_opt(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    int result = 0;
    result = rcsp_extra_flash_opt_update_ui_opt(state);
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    app获取状态指令
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_update_state_to_app(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    rcsp_printf("rcsp_extra_flash_update_state_to_app\n");
    int result = 0;
    result = rcsp_extra_resp_state_to_app();
    if (*data_len > offset) {
        u16 remote_data_crc16 = data[offset++] << 8 | data[offset];
        rcsp_printf("remote_data_crc16 %x, g_curr_data_crc16 %x\n", remote_data_crc16, g_curr_data_crc16);
        if (remote_data_crc16 != g_curr_data_crc16) {
            result = -1;
        }
        g_curr_data_crc16 = 0;
        // 回复buffer剩余大小
        if (state) {
            u16 recieve_max = rcsp_packet_write_alloc_len() - 13;
            data[1] = ((u8 *)&recieve_max)[1];
            data[2] = ((u8 *)&recieve_max)[0];
            *data_len = sizeof(recieve_max);
        }
    }
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    app获取ota升级状态命令
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int rcsp_extra_flash_ota_update_state(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    rcsp_printf("rcsp_extra_flash_ota_update_state\n");
    int result = 0;
    rcsp_eflash_update_flag_set(state);
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    还原系统命令
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int samrtbox_extra_flash_restore(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    return 0;
}

//*----------------------------------------------------------------------------*/
/**@brief    获取文件信息命令
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int samrtbox_extra_flash_file_info_get(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    rcsp_printf("samrtbox_extra_flash_file_info_get, file_len %d, file_name :", *data_len);
    put_buf(data + offset, *data_len - offset);
    int result = 0;
    FILE *file = NULL;

    char *root_path = watch_get_root_path();
    char watch[64] = {0};

    u8 *data_ptr = data + 1;

    ASCII_ToUpper(data + offset, *data_len - offset);
    strcpy(watch, root_path);
    strncpy(&watch[strlen(root_path)], data + offset + 1, *data_len - offset - 1);
    file = fopen(watch, "r");
    if (NULL == file) {
        result = -1;
        goto __samrtbox_extra_flash_file_info_get_err;
    }

    u32 file_len = flen(file);
    u16 file_crc = 0;
    for (u32 crc_offset = 0; crc_offset < file_len;) {
        wdt_clear();
        u32 crc_len = (file_len - crc_offset) > 256 ? 256 : (file_len - crc_offset);
        fseek(file, crc_offset, SEEK_SET);
        if (crc_len != fread(file, data, crc_len)) {
            printf("err : read fail, %s, %d\n", watch, crc_offset);
            result = -1;
            goto __samrtbox_extra_flash_file_info_get_err;
        }
        printf("===================%s======%d=========yuring==\n\r", __func__, __LINE__);
#if 0 ///RCSP TODO
        file_crc = CRC16_with_initval(data, crc_len, file_crc);
#endif
        crc_offset += crc_len;
    }

    *data_len = 0;

    // 文件长度
    for (u8 i = 0; i < sizeof(file_len); i++, data_ptr++, (*data_len)++) {
        *data_ptr = ((u8 *)&file_len)[sizeof(file_len) - i - 1];
    }

    // 还需要获取文件crc
    for (u8 i = 0; i < sizeof(file_crc); i++, data_ptr++, (*data_len)++) {
        *data_ptr = ((u8 *)&file_crc)[sizeof(file_crc) - i - 1];
    }

__samrtbox_extra_flash_file_info_get_err:
    if (file) {
        fclose(file);
    }
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    获取FLASH剩余空间命令
   @param    data:数据, data_len:数据长度(也用作出参)，offset:遍历数据时的偏移量，state:操作类型
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int samrtbox_extra_flash_remain_space(u8 *data, u16 *data_len, u8 offset, u8 state)
{
    rcsp_printf("samrtbox_extra_flash_remain_space");
    int result = 0;
    char *root_path = watch_get_root_path();
    u32 space = 0;
    *data_len = 0;
    u8 *data_ptr = data + 1;
    if (fget_free_space(root_path, &space)) {
        printf("err : get flash space fail\n");
        result = -1;
    } else {
        for (u8 i = 0; i < sizeof(space); i++, data_ptr++, (*data_len)++) {
            *data_ptr = ((u8 *)&space)[sizeof(space) - i - 1];
        }
    }
    data[0] = result;
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    外部flash操作函数
   @param    param:数据, len:数据长度，OpCode:命令号，OpCode_SN:数据包序列号
   @return   0-成功，其他-失败
   @note
*/
/*----------------------------------------------------------------------------*/
int rcsp_extra_flash_opt(u8 *param, u16 len, u8 OpCode, u8 OpCode_SN)
{
    int result = 0;

    u8 offset = 0;
    u8 opt_flag = param[offset++];
    u8 state = param[offset++];

    switch (opt_flag) {
    case CURR_SYSTEM_OPT_READ:
        result = rcsp_extra_flash_read_opt(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_WRITE:
        result = rcsp_extra_flash_write_opt(param, &len, offset, state, OpCode);
        break;
    case CURR_SYSTEM_OPT_INSERT:
        result = rcsp_extra_flash_insert_opt(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_DIAL:
        result = rcsp_extra_flash_dial_opt(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_ERASE:
        result = rcsp_extra_flash_erase_opt(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_DELETE:
        result = rcsp_extra_flash_delete_opt(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_UPDATE_FAT:
        result = rcsp_extra_flash_update_fat_opt(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_UPDATE_UI:
        result = rcsp_extra_flash_update_ui_opt(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_TRAN_REPLY:
        result = rcsp_extra_flash_update_state_to_app(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_UPDATE_FLAG:
        result = rcsp_extra_flash_ota_update_state(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_RESTORE:
        result = samrtbox_extra_flash_restore(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_FILE_INFO_GET:
        result = samrtbox_extra_flash_file_info_get(param, &len, offset, state);
        break;
    case CURR_SYSTEM_OPT_REMAIN_SPACE:
        result = samrtbox_extra_flash_remain_space(param, &len, offset, state);
        break;
    }

    u16 resp_len = rcsp_extra_flash_opt_resp_data_get(param + 1, len, opt_flag, state) + 1;
    rcsp_extra_flash_opt_resp(0, OpCode, OpCode_SN, param, resp_len);
    return result;
}

//*----------------------------------------------------------------------------*/
/**@brief    主动通知app当前表盘路径，用于表盘操作
   @param    file_path:路径名，file_path_size:路名名长度
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void rcsp_extra_flash_opt_dial_nodify(void)
{
    rcsp_printf("rcsp_extra_flash_opt_dial_nodify\n");
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL) {
        return;
    }
    if (!get_rcsp_connect_status()) {
        return;
    }
    u8 *data = watch_get_root_path();
    u8 *file_path = watch_get_item(watch_get_style()) + strlen(data) - 1;
    data = rindex(file_path, '/');
    u8 file_path_size = data - file_path;

    u8 OpCode = 0x1A;

    u8 opt_flag = CURR_SYSTEM_OPT_DIAL;
    u8 state = 2;
    u8 len = file_path_size + sizeof(opt_flag) + sizeof(state) + 1;

    data = (u8 *) zalloc(len);
    len = 0;
    data[len++] = opt_flag;
    data[len++] = state;
    memcpy(data + len, file_path, file_path_size);
    ASCII_ToUpper(data + len, file_path_size);
    len += file_path_size;
    data[len++] = 0;
    rcsp_extra_flash_opt_resp(1, OpCode, 0, data, len);
    if (data) {
        free(data);
    }
}

//*----------------------------------------------------------------------------*/
/**@brief    获取升级异常状态标志位
   @param
   @return   1-处于异常，0-正常
   @note
*/
/*----------------------------------------------------------------------------*/
u8 rcsp_eflash_update_flag_get(void)
{
    u8 eflash_update_flag = 0;
    if (sizeof(eflash_update_flag) ==
        syscfg_read(VM_EXTRA_FLASH_ALL_UPDATE_FLAG, &eflash_update_flag, sizeof(eflash_update_flag))) {
        return eflash_update_flag;
    }
    return eflash_update_flag;
}

//*----------------------------------------------------------------------------*/
/**@brief    获取升级异常状态标志位
   @param
   @return   1-处于异常，0-正常
   @note
*/
/*----------------------------------------------------------------------------*/
void rcsp_eflash_update_flag_set(u8 eflash_state_type)
{
    u8 eflash_update_flag = -1;
    syscfg_read(VM_EXTRA_FLASH_ALL_UPDATE_FLAG, &eflash_update_flag, sizeof(eflash_update_flag));
    if (eflash_state_type != eflash_update_flag) {
        eflash_update_flag = eflash_state_type;
        syscfg_write(VM_EXTRA_FLASH_ALL_UPDATE_FLAG, &eflash_update_flag, sizeof(eflash_update_flag));
    }
}

//*----------------------------------------------------------------------------*/
/**@brief    小机恢复fat表操作
   @param
   @return   1-处于异常，0-正常
   @note
*/
/*----------------------------------------------------------------------------*/
int rcsp_eflash_resume_opt(void)
{
    int ret = 0;
    if (0 == rcsp_eflash_flag_get()) {
        return ret;
    }
    u8 backup_data[256] = {0};
    u16 offset = 0;
    // 小机进行回复动作
    virfat_flash_read_watch(backup_data, 4096, sizeof(backup_data));
    // 假如备份区不是全F就开始进行回复操作
    if (0xFF == backup_data[0]) {
        return ret;
    }
    for (offset = 0; offset < 4096 * 2; offset += sizeof(backup_data)) {
        virfat_flash_read_watch(backup_data, 4096 + offset, sizeof(backup_data));
        if (0 == (offset % 4096)) {
            rcsp_extra_flash_opt_erase(4096 + 4096 * 2 + offset, 4096);
        }
        virfat_flash_write_watch(backup_data, 4096 + 4096 * 2 + offset, sizeof(backup_data));
    }
    // 清除标志位标志位
    rcsp_eflash_flag_set(0);
    return ret;
}

static void rcsp_extra_flash_disconnect_tips_opt(void *priv)
{
    // 无线断开、触发定时器后的操作
}

void rcsp_extra_flash_disconnect_tips(u32 sec)
{
    if (!rcsp_eflash_flag_get() || rcsp_eflash_update_flag_get()) {
        return;
    }
    static u16 tips_timer = 0;
    if (sec && (0 == tips_timer)) {
        tips_timer = sys_timeout_add(NULL, rcsp_extra_flash_disconnect_tips_opt, sec);
    } else if (tips_timer) {
        sys_timeout_del(tips_timer);
        tips_timer = 0;
    }
}

//*----------------------------------------------------------------------------*/
/**@brief    大文件传输表盘开始/结束操作
   @param    0 大文件续传
             1 新文件（不做任何操作）
			 2 插入结束
			 3 更新表盘与背景关系
			 4 删除表盘
			-1 重命名前删除重名文件
   @return   0 成功
			 1 写失败
			 2 数据超出范围
			 3 crc校验失败
			 4 内存不足
   @note
*/
/*----------------------------------------------------------------------------*/
int rcsp_file_transfer_watch_opt(u8 flag, char *root_path)
{
    int ret = 0;
    FILE *file = NULL;
    u32 len = sizeof("*.tmp") > strlen(watch_name) ? sizeof("*.tmp") : strlen(watch_name);
    char *path = zalloc(strlen(root_path) + len);
    if (NULL == path) {
        printf("path is null\n");
        ret = 1;
        goto __rcsp_file_transfer_watch_opt_end;
    }

    switch (flag) {
    case 1:
        // 1、删除所有tmp文件
        if (NULL == root_path) {
            printf("root_path is null\n");
            ret = 1;
            goto __rcsp_file_transfer_watch_opt_end;
        }
        strcat(path, root_path);
        strcat(path, "*.tmp");
        while ((file = fopen(path, "r"))) {
            if (fdelete(file)) {
                printf("err : delete tmp file fail\n");
                ret = 1;
                goto __rcsp_file_transfer_watch_opt_end;
            }
            file = NULL;
        }

        // 2、假如空间刚刚足够，就尝试删除需要添加的文件
        //    假如空间不够，返回错误
        u32 space = 0;
        if (fget_free_space(root_path, &space)) {
            printf("err : get flash space fail\n");
            ret = 1;
            goto __rcsp_file_transfer_watch_opt_end;
        }
        space *= 1024;
        memset(path, 0, strlen(path));
        strcat(path, root_path);
        if ('/' == watch_name[0]) {
            strcat(path, watch_name + 1);
        } else {
            strcat(path, watch_name);
        }
        file = fopen(path, "r");

        if (file) {
            if ((space + flen(file)) < g_watch_file_size) {
                // 空间不足
                printf("err : flash no enough space\n");
                ret = 4;
                goto __rcsp_file_transfer_watch_opt_end;
            } else if (space < g_watch_file_size) {
                // 删除后空间足够
                printf("delelte repeat file : %s\n", watch_name);
                if (fdelete(file)) {
                    printf("err(%d) : delete file fail\n", flag);
                    ret = 1;
                    goto __rcsp_file_transfer_watch_opt_end;
                }
                file = NULL;
            }
        } else if (space < g_watch_file_size) {
            printf("err : flash no enough space\n");
            ret = 4;
            goto __rcsp_file_transfer_watch_opt_end;
        }

        break;
    case 2:
        rcsp_extra_flash_opt_insert_stop();
        break;
    case 3:
        if (!rcsp_eflash_update_flag_get()) {
            watch_add_item_deal();
        }
        break;
    case 4:
        samrtbox_extra_flash_opt_delete_end();
        break;
    case (u8)-1:
        // 尝试删除需要添加的文件
        if (NULL == root_path) {
            printf("root_path is null\n");
            ret = 1;
            goto __rcsp_file_transfer_watch_opt_end;
        }
        strcat(path, root_path);
        if ('/' == watch_name[0]) {
            strcat(path, watch_name + 1);
        } else {
            strcat(path, watch_name);
        }
        if ((file = fopen(path, "r"))) {
            if (fdelete(file)) {
                printf("err(%d) : delete file fail\n", flag);
                ret = 1;
                goto __rcsp_file_transfer_watch_opt_end;
            }
            file = NULL;
        }
        break;
    }

__rcsp_file_transfer_watch_opt_end:
    if (file) {
        fclose(file);
    }

    if (path) {
        free(path);
    }
    return ret;
}

void rcsp_update_flag_clear(void)
{
#if RCSP_UPDATE_EN
    if (update_success_boot_check()) {
        rcsp_eflash_update_flag_set(0);
        rcsp_eflash_flag_set(0);
        set_update_ex_flash_flag(0);
    }
#endif
}

#else

void rcsp_extra_flash_opt_dial_nodify(void)
{

}

void rcsp_eflash_flag_set(u8 eflash_state_type)
{

}

u8 rcsp_eflash_flag_get(void)
{
    return 0;
}

void rcsp_eflash_update_flag_set(u8 eflash_state_type)
{

}

u8 rcsp_eflash_update_flag_get(void)
{
    return 0;
}

void app_watch_ui_updata_task()
{

}

bool rcsp_exflash_flash_opt_check(void *param)
{
    return false;
}

void rcsp_extra_flash_init(void)
{

}

int rcsp_extra_flash_event_deal(struct sys_event *event)
{
    return true;
}

void rcsp_extra_flash_close(void)
{

}

int rcsp_eflash_resume_opt(void)
{
    return 0;
}

void rcsp_extra_flash_disconnect_tips(u32 sec)
{

}

int rcsp_file_transfer_watch_opt(u8 flag)
{
    return 0;
}

int rcsp_extra_flash_opt(u8 *param, u16 len, u8 OpCode, u8 OpCode_SN)
{
    return 0;
}

void rcsp_update_flag_clear(void)
{

}
#endif
