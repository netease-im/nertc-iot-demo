#include "app_config.h"
#include "typedef.h"
#include "fs/fs.h"
#include "update_loader_download.h"
#include "update.h"

typedef struct _lcflash_update_info_parm {
    u32 src_file_addr;
    u32 src_file_len;
} lcflash_update_info_parm;

static lcflash_update_info_parm lc_flash_update_info;
static void *update_fp = NULL;

#define SDFILE_EXT_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/EXT_RESERVED/"  //拓展分区

//=============================================================//
//                  内置LOCAL Flash升级V2版本                     //
/* APP LOCAL Flash 升级模型(APP_LC_FLASH_UFW_UPDATA)
  ________                       ________                       ________
 |        |                     |        |                     |        |
 |        |          (1)        |        |         (2)         |        |
 |   APP  |      --------->     |LC_Flash|      -------->      |  CHIP  |
 |        | lc_flash_update.ufw |        | lc_flash_update.ufw |        |
 |________|                     |________|                     |________|

NOTE:
 关于流程(1): 该流程为从远端获取升级文件(lc_flash_update.ufw)写到内置flash预留区域中, lc_flash_update.ufw文件在内置flash中需要保证在预留足够空间且物理空间连续, 注意该流程为用户在自己定义的ota中完成, 不在本文件流程中, 需要用户自己开发;
 关于流程(2): 该流程为本文件实现的升级流程, 在用户完成流程(1)后(lc_flash_update.ufw已存在内置flash中), 可随时调用lc_flash_update_ufw_init函数启动升级;

 */
//============================================================//

#define LC_FLASH_UFW_UPDATE_VERIFY_ALL_FILE 		1 //1: 进升级前校验用户写到内置flash的lc_flash_update.ufw文件; 0: 不校验

/* extern const int support_lc_flash_ufw_update_en; */

extern void wdt_clear(void);

//=========================================================//
// lc_flash 操作接口
//=========================================================//

static u32 user_remote_base_addr = 0; //
static u32 user_remote_cur_offset = 0;
static void *dev_ptr = NULL;
static char *path = NULL;
/*----------------------------------------------------------------------------*/
/**@brief  获取升级文件在内置flash中存放位置和长度
   @param  buf, 结构为lc_flash_update_info
   @return void
   @note
*/
/*----------------------------------------------------------------------------*/
static void get_lc_flash_update_info_param(void *buf)
{
    if (buf) {
        printf("src_file_addr = 0x%x", lc_flash_update_info.src_file_addr); //存放升级文件物理地址
        printf("src_file_len = 0x%x", lc_flash_update_info.src_file_len);   //存放升级文件物理地址
        memcpy(buf, (u8 *)&lc_flash_update_info, sizeof(lc_flash_update_info));
    }
}



static u32 user_remote_device_get_cur_addr(void)
{
    return (user_remote_base_addr + user_remote_cur_offset);
}
/*----------------------------------------------------------------------------*/
/**@brief   打开升级文件所在flash的预留区域
   @param   void
   @return  0: 打开错误;
   		   非0: 打开成功
   @note
*/
/*----------------------------------------------------------------------------*/
static u16 lc_flash_ufw_update_f_open(void)
{
    if (dev_ptr) {
        fseek((FILE *)dev_ptr, 0, SEEK_SET);
        return 1;
    }
    dev_ptr = (void *)fopen(path, "r");
    if (dev_ptr) {
        struct vfs_attr attr;
        fget_attrs((FILE *)dev_ptr, &attr);
        user_remote_base_addr = sdfile_cpu_addr2flash_addr(attr.sclust);
        lc_flash_update_info.src_file_addr = user_remote_base_addr;
        printf("open %s file succ 0x%x, 0x%x", path, dev_ptr, user_remote_base_addr);
        return 1;
    } else {
        printf("open %s file fail", path);
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   读取预留区域升级文件内容
   @param   fp: NULL, 保留
   @param   buff: 读取数据buf
   @param   len: 读取数据长度
   @return  len: 读取成功
            (-1): 读取出错
   @note
*/
/*----------------------------------------------------------------------------*/
static u16 lc_flash_ufw_update_f_read(void *fp, u8 *buff, u16 len)
{
    //TODO:
    int rlen = 0;
    if (dev_ptr) {
        wdt_clear();
        /* rlen = fread((FILE *)dev_ptr, buff, len); */
        rlen = fread(buff, 1, len, (FILE *)dev_ptr);
    }
    return (u16)rlen;
}

/*----------------------------------------------------------------------------*/
/**@brief   偏移升级文件地址
   @param   fp: NULL, 保留
   @param   type: SEEK_SET, SEEK_CUR, SEEK_END
   @param   offset: 偏移长度
   @return  0: 操作成功
            非0: 操作出错
   @note
*/
/*----------------------------------------------------------------------------*/
static int lc_flash_ufw_update_f_seek(void *fp, u8 type, u32 offset)
{
    //TODO:
    int ret = 0;
    if (dev_ptr) {
        ret = fseek((FILE *)dev_ptr, (int)offset, (int)type);
    }
    return ret;
}

/*----------------------------------------------------------------------------*/
/**@brief   关闭预留区域读操作
   @param   err: 传入升级状态
   @return  0: 操作成功
            非0: 操作出错
   @note
*/
/*----------------------------------------------------------------------------*/
static u16 lc_flash_ufw_update_f_stop(u8 err)
{
    //TODO:
    printf("%s, err = 0x%x", __func__, err);
    if (dev_ptr) {
        fclose((FILE *)dev_ptr);
        dev_ptr = NULL;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   通知升级文件大小
   @param   priv: NULL, 保留
   @param   size: 待升级文件大小
   @return  0: 操作成功
            非0: 操作出错
   @note
*/
/*----------------------------------------------------------------------------*/
static int lc_flash_ufw_update_notify_update_content_size(void *priv, u32 size)
{
    //TODO:
    printf("%s: size: 0x%x", __func__, size);

    return 0;
}


static const update_op_api_t lc_flash_ufw_update_op_api = {
    .ch_init = NULL,
    .f_open = lc_flash_ufw_update_f_open,
    .f_read = lc_flash_ufw_update_f_read,
    .f_seek = lc_flash_ufw_update_f_seek,
    .f_stop = lc_flash_ufw_update_f_stop,
    .notify_update_content_size = lc_flash_ufw_update_notify_update_content_size,
};

/*----------------------------------------------------------------------------*/
/**@brief   flash升级文件校验完成, 会调用该函数
   @param   priv: 回调传入参数
   @param   type: 升级模式
   @param   cmd: 1) UPDATE_LOADER_OK flash升级文件校验成功, cpu复位, 启动固件升级流程
   				 2) UPDATE_LOADER_ERR flash升级文件校验失败
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void lc_flash_ufw_update_end_callback(void *priv, int type, u8 cmd)
{
    //TODO:
    printf("%s: type: 0x%x, cmd = 0x%x", __func__, type, cmd);

    if (type == USER_LC_FLASH_UFW_UPDATA) {
        if (cmd == UPDATE_LOADER_OK) {
            printf("soft reset to update >>>");
            cpu_reset(); //复位让主控进入升级内置flash
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   填充升级结构体私有参数
   @param   p: 升级结构体指针(UPDATA_PARM)
   @return  void
   @note
*/
/*----------------------------------------------------------------------------*/
static void lc_flash_ufw_update_private_param_fill(UPDATA_PARM *p)
{
    get_lc_flash_update_info_param(p->parm_priv);
}

/*----------------------------------------------------------------------------*/
/**@brief   固件升级校验流程完成, cpu reset跳转升级新的固件
   @param   type: 升级类型
   @return  void
   @note
*/
/*----------------------------------------------------------------------------*/
static void lc_flash_ufw_update_before_jump_handle(int type)
{
    printf("soft reset to update >>>");
    cpu_reset(); //复位让主控进入升级内置flash
}

/*----------------------------------------------------------------------------*/
/**@brief   内置flash升级流程状态处理
   @param   type: 升级类型
   @param   state: 当前升级状态
   @param   priv: 跟状态相关的私有参数指针
   @return  void
   @note
*/
/*----------------------------------------------------------------------------*/
static void lc_flash_ufw_update_state_cbk(int type, u32 state, void *priv)
{
    update_ret_code_t *ret_code = (update_ret_code_t *)priv;

    if (ret_code) {
        printf("state:%x err:%x\n", ret_code->stu, ret_code->err_code);
    }

    switch (state) {
    case UPDATE_CH_EXIT:
        if ((0 == ret_code->stu) && (0 == ret_code->err_code)) {
            update_mode_api_v2(USER_LC_FLASH_UFW_UPDATA,
                               lc_flash_ufw_update_private_param_fill,
                               lc_flash_ufw_update_before_jump_handle);
        }

        break;

    default:
        break;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  通过地址读取升级文件内容
   @param  buf, 读取数据缓存
   @param  addr, 读取预留区域数据的地址
   @param  len, 读取预留区域数据的长度
   @return u16 读取长度
   @note
*/
/*----------------------------------------------------------------------------*/
static u16 lc_flash_update_file_read(void *buf, u32 addr, u32 len)
{
    int rlen = 0;

    if (dev_ptr) {
        wdt_clear();
        fseek((FILE *)dev_ptr, (int)(addr - user_remote_base_addr), SEEK_SET);
        /* rlen = fread((FILE *)dev_ptr, buf, len); */
        rlen = fread(buf, 1, len, (FILE *)dev_ptr);
    }
    return (u16)rlen;
}

/*----------------------------------------------------------------------------*/
/**@brief  启动内置flash升级接口, 该接口为用户在保证将lc_flash_update.ufw烧写到内置flash预留区域之后, 调用该接口进行程序升级
   @param  reserved_area_name: 升级文件所在的预留区域名字
   @return  void
   @note
*/
/*----------------------------------------------------------------------------*/
//使用ufw文件格式
void lc_flash_update_ufw_init(char *reserved_area_name)
{
    /* if (support_lc_flash_ufw_update_en == 0) { */
    /* goto _ERR_RET; */
    /* } */

    if (NULL == path) {
        path = zalloc(sizeof(SDFILE_EXT_RES_ROOT_PATH) + strlen(reserved_area_name));
        if (NULL == path) {
            printf("err: alloc lc flash ufw path fail\n");
            goto _ERR_RET;
        }
        strcat(path, SDFILE_EXT_RES_ROOT_PATH);
        strcat(path, reserved_area_name);
    }

#if LC_FLASH_UFW_UPDATE_VERIFY_ALL_FILE
    if (lc_flash_ufw_update_f_open()) {
        if (update_file_verify(user_remote_base_addr, lc_flash_update_file_read) != 0) {
            //校验失败
            goto _ERR_RET;
        }
    } else {
        goto _ERR_RET;
    }
#endif

    update_mode_info_t lc_flash_ufw_update_info = {
        .type = USER_LC_FLASH_UFW_UPDATA,
        .state_cbk = lc_flash_ufw_update_state_cbk,
        .p_op_api = &lc_flash_ufw_update_op_api,
        .task_en = 0,
    };

    app_active_update_task_init(&lc_flash_ufw_update_info);

_ERR_RET:
    if (path) {
        free(path);
        path = NULL;
    }
}



