#include "file_trans_back.h"
#include "rcsp.h"
#include "system/includes.h"
/* #include "clock_cfg.h" *////RCSP TODO
#include "file_operate/file_manager.h"
#include "rcsp_browser.h"
#include "fs/fs.h"
#include "rcsp_manage.h"

#include "rcsp_event.h"
#include "rcsp_config.h"
#include "JL_rcsp_protocol.h"
#include "JL_rcsp_packet.h"

#if !JL_EARPHONE_APP_EN
/* #include "common/dev_status.h" *////RCSP TODO
#endif

#if (RCSP_MODE && TCFG_DEV_MANAGER_ENABLE)

#define FTP_DOWNLOAD_FOLDER_NAME				"download" //下载目录

#define FILE_TRANS_BACK_TASK_NAME				"ftran_back"

enum {
    FILE_TRANS_BACK_BY_NAME,
    FILE_TRANS_BACK_BY_CLUST,
    FILE_TRANS_BACK_BY_NAME_2,
    FILE_TRANS_BACK_FINISH = 0x80,
    FILE_TRANS_BACK_CANCEL = 0x81,
};

struct __file_trans_back {
    FILE *file;
    struct __dev *dev;
    struct vfscan *fsn;
    void (*end_callback)(void);
    u8 OpCode_SN;
    u8 op;
};

///播放参数，文件扫描时用，文件后缀等
static const char scan_parm[] = "-t"
#if (WATCH_FILE_TO_FLASH)
                                "ALL"
#else // WATCH_FILE_TO_FLASH
#if (TCFG_DEC_MP3_ENABLE)
                                "MP1MP2MP3"
#endif
#if (TCFG_DEC_WMA_ENABLE)
                                "WMA"
#endif
#if ( TCFG_DEC_WAV_ENABLE || TCFG_DEC_DTS_ENABLE)
                                "WAVDTS"
#endif
#if (TCFG_DEC_FLAC_ENABLE)
                                "FLA"
#endif
#if (TCFG_DEC_APE_ENABLE)
                                "APE"
#endif
#if (TCFG_DEC_M4A_ENABLE)
                                "M4AAAC"
#endif
#if (TCFG_DEC_M4A_ENABLE || TCFG_DEC_ALAC_ENABLE)
                                "MP4"
#endif
#if (TCFG_DEC_AMR_ENABLE)
                                "AMR"
#endif
#if (TCFG_DEC_DECRYPT_ENABLE)
                                "SMP"
#endif
#if (TCFG_DEC_MIDI_ENABLE)
                                "MID"
#endif
                                "TMP"
#endif // WATCH_FILE_TO_FLASH
                                " -sn -r"
                                ;

static struct __file_trans_back *trans_back = NULL;

static u8 g_trans_back_cancel_flag = -1;

static u32 g_dev_handle = BS_SD1;

extern u8 check_le_pakcet_sent_finish_flag(void);
extern bool rcsp_send_list_is_empty(void);
extern void file_trans_idle_set(u8 file_trans_idle_flag);

static int get_file_prepare(u32 dev_handle)
{
    char *logo = rcsp_browser_dev_remap(dev_handle);
    if (NULL == logo) {
        printf("trans_back logo is null");
        return -1;
    }

    struct __dev *dev  = dev_manager_find_spec(logo, 0);
    if (NULL == dev) {
        printf("trans_back dev is null");
        return -1;
    }

    if (NULL == trans_back) {
        trans_back = zalloc(sizeof(struct __file_trans_back));
        if (NULL == trans_back) {
            printf("trans_back zalloc err\n");
            return -1;
        }
        trans_back->dev = dev;
    }
    return 0;
}

static void creat_file_path(char *path, char *root_path, const char *folder, u8 *name, u16 name_len)
{
    strcat(path, root_path);
    if (folder) {
        strcat(path, folder);
        strcat(path, "/");
    }
    //strcat(path, name);
    /* memcpy(path + strlen(root_path) + strlen(folder) + 1, name, name_len); */
    memcpy(path + strlen(path), name, name_len);
    //printf("path = %s\n", path);
}

static u32 creat_path_len(char *root_path, const char *folder, u8 *name, u16 name_len)
{
    u32 len = (strlen(root_path) + strlen(folder) + name_len + 1);
    if (folder) {
        len += strlen("/");
    }
    return len;
}

static u8 file_trans_back_response_send(u8 *data, u16 len, u8 dire, u8 OpCode_SN)
{
    u8 ret = 0;
    switch (dire) {
    case 0:
        ret = JL_CMD_response_send(JL_OPCODE_ONE_FILE_TRANS_BACK, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, len);
        break;
    case 1:
        ret = JL_DATA_send(JL_OPCODE_DATA, JL_OPCODE_ONE_FILE_TRANS_BACK, data, len, JL_NOT_NEED_RESPOND);
        break;
    case 2:
        ret = JL_CMD_send(JL_OPCODE_ONE_FILE_TRANS_BACK, data, len, JL_NEED_RESPOND);
        break;
    case 0xFF:
        ret = JL_CMD_response_send(JL_OPCODE_ONE_FILE_TRANS_BACK, JL_PRO_STATUS_FAIL, OpCode_SN, data, len);
        break;
    }
    return ret;
}

static void file_trans_back_task(void *p)
{
    struct RcspModel *rcspModel = (struct RcspModel *)p;
    // 从文件句柄中获取当前文件的大小
    struct vfs_attr attr;
    fget_attrs(trans_back->file, &attr);
    // 获取文件数据
    u32 file_size = attr.fsize;

    // 预留 开始头(3byte) + opCode(2byte) + param_len(2byte) + state(1byte) + opCode_SN(1byte) + 结束(1byte)
    u16 resp_data_len = JL_packet_get_tx_max_mtu() - 10;
    u8 *resp_data = zalloc(resp_data_len);
    if (NULL == resp_data) {
        // 错误
        file_trans_back_response_send(&trans_back->op, sizeof(trans_back->op), (u8) - 1, trans_back->OpCode_SN);
        goto __file_trans_back_task_err;
    }

    // 把大小发送给app
    resp_data[0] = trans_back->op;
    resp_data[1] = ((u8 *)&file_size)[3];
    resp_data[2] = ((u8 *)&file_size)[2];
    resp_data[3] = ((u8 *)&file_size)[1];
    resp_data[4] = ((u8 *)&file_size)[0];
    file_trans_back_response_send(resp_data, sizeof(file_size) + 1, 0, trans_back->OpCode_SN);
    // 退出sniff
    file_trans_idle_set(0);

    u16 crc = 0;
    resp_data_len -= 4;
    if (FILE_TRANS_BACK_BY_NAME != trans_back->op) {
        resp_data_len -= 2;
    }
    for (u32 offset = 0, data_len = 0, ret = 0; offset < file_size;) {
        wdt_clear();
        // 假如当前spp或ble断开连接
        if (0 == get_rcsp_connect_status()) {
            goto __file_trans_back_task_err;
        }
        if ((u8) - 1 != g_trans_back_cancel_flag) {
            break;
        }

        data_len = (file_size - offset) > resp_data_len ? resp_data_len : file_size - offset;
        fseek(trans_back->file, offset, SEEK_SET);
        resp_data[0] = ((u8 *)&offset)[3];
        resp_data[1] = ((u8 *)&offset)[2];
        resp_data[2] = ((u8 *)&offset)[1];
        resp_data[3] = ((u8 *)&offset)[0];

        if (FILE_TRANS_BACK_BY_NAME == trans_back->op) {
            /* fread(trans_back->file, resp_data + 4, data_len); */

            fread(resp_data + 4, data_len, 1, trans_back->file);
        } else {
            /* fread(trans_back->file, resp_data + 6, data_len); */

            fread(resp_data + 6, data_len, 1, trans_back->file);

            // 填充crc
            if (JL_ERR_NONE == ret) {
                printf("===================%s======%d=========yuring==\n\r", __func__, __LINE__);
                crc = CRC16_with_initval(resp_data + 6, data_len, crc);
            }
            data_len += 2;
            resp_data[4] = crc >> 8;
            resp_data[5] = crc & 0xFF;
        }

        // 发送文件数据
        ret = file_trans_back_response_send(resp_data, data_len + 4, 1, trans_back->OpCode_SN);
        if (JL_ERR_SEND_BUSY == ret) {
            continue;
        }
        offset += resp_data_len;
    }

    while (!(rcsp_send_list_is_empty() && check_le_pakcet_sent_finish_flag())) {
        os_time_dly(10);
    }

    memset(resp_data, 0, resp_data_len + 1);
    if ((u8) - 1 == g_trans_back_cancel_flag) {
        // 如果传输完成发送结束命令
        resp_data[0] = FILE_TRANS_BACK_FINISH;
        file_trans_back_response_send(resp_data, 1 + 4, 2, trans_back->OpCode_SN);
    } else {
        // 取消
        resp_data[0] = FILE_TRANS_BACK_CANCEL;
        file_trans_back_response_send(resp_data, 1, 0, trans_back->OpCode_SN);
    }

__file_trans_back_task_err:
    if (resp_data) {
        free(resp_data);
    }

    file_trans_idle_set(1);
    rcsp_msg_post(USER_MSG_RCSP_FILE_TRANS_BACK, 1, (int)p);
    while (1) {
        os_time_dly(10);
    }
}

static int get_file_by_name(void *priv, u8 *data, u16 len)
{
    u8 offset = 0;
    u32 file_offset = data[offset++] << 24 | data[offset++] << 16 << data[offset++] << 8 | data[offset++];

    u32 param_len = 0;
    if (FILE_TRANS_BACK_BY_NAME != trans_back->op) {
        param_len = data[offset++];
    }
    u8 *file_name = data + offset;
    // 拼凑绝对路径
    char *root_path = dev_manager_get_root_path(trans_back->dev);
    char *folder = NULL;
    if (BS_FLASH == g_dev_handle ||
        BS_FLASH_2 == g_dev_handle) {
        if ('/' == file_name[0]) {
            file_name++;
            offset++;
        }
    } else {
        folder = FTP_DOWNLOAD_FOLDER_NAME;
    }
    char *path = zalloc(creat_path_len(root_path, folder, file_name, len - offset));
    creat_file_path(path, root_path, folder, file_name, len - offset);

    if (NULL == trans_back->file) {
        trans_back->file = fopen(path, "r");
    }

    if (path) {
        free(path);
    }

    if (NULL == trans_back->file) {
        // 文件不存在
        goto __get_file_file_by_name_err;
    }

    if (task_create(file_trans_back_task, priv, FILE_TRANS_BACK_TASK_NAME)) {
        goto __get_file_file_by_name_err;
    }
    return 0;

__get_file_file_by_name_err:
    file_trans_back_close();
    return -1;
}

static int cancel_file_trans_back(u8 OpCode_SN, u8 *data, u16 len)
{
    int ret = 0;
    if (trans_back) {
        trans_back->OpCode_SN = OpCode_SN;
        trans_back->op = FILE_TRANS_BACK_CANCEL;
        g_trans_back_cancel_flag = data[0];
    } else {
        ret = -1;
    }
    return ret;
}

void file_trans_back_close(void)
{
    task_kill(FILE_TRANS_BACK_TASK_NAME);
    g_trans_back_cancel_flag = -1;

    if (trans_back->fsn) {
        dev_manager_scan_disk_release(trans_back->fsn);
        trans_back->fsn = NULL;
    }

    // 关闭文件
    if (trans_back->file) {
        fclose(trans_back->file);
        trans_back->file = NULL;
    }

    if (trans_back) {
        free(trans_back);
        trans_back = NULL;
    }
}

static int get_file_by_clust(void *priv, u8 *data, u16 len)
{
    u8 offset = 0;
    u32 file_offset = data[offset++] << 24 | data[offset++] << 16 | data[offset++] << 8 | data[offset++];
    u32 cluster = data[offset++] << 24 | data[offset++] << 16 | data[offset++] << 8 | data[offset++];
    // find name by cluster
    if (NULL == trans_back->file) {
        if (NULL == trans_back->fsn) {
            trans_back->fsn = dev_manager_scan_disk(trans_back->dev, NULL, scan_parm, 0, NULL);
        }
        trans_back->file = file_manager_select(trans_back->dev, trans_back->fsn, FSEL_BY_SCLUST, cluster, NULL);
    }

    if (NULL == trans_back->fsn) {
        goto __get_file_by_clust_err;
    }

    if (NULL == trans_back->file) {
        goto __get_file_by_clust_err;
    }

    if (task_create(file_trans_back_task, priv, FILE_TRANS_BACK_TASK_NAME)) {
        goto __get_file_by_clust_err;
    }
    return 0;

__get_file_by_clust_err:
    file_trans_back_close();
    return -1;
}

void file_trans_back_opt(void *priv, u8 OpCode_SN, u8 *data, u16 len)
{
    int ret = 0;
    u8 offset = 0;
    u8 op = data[offset++];
    switch (op) {
    case FILE_TRANS_BACK_BY_NAME_2:
        g_dev_handle = data[offset++] << 24 | data[offset++] << 16 | data[offset++] << 8 | data[offset++];
    case FILE_TRANS_BACK_BY_NAME:
        ret = get_file_prepare(g_dev_handle);
        if (ret) {
            break;
        }
        trans_back->OpCode_SN = OpCode_SN;
        trans_back->op = op;
        ret = get_file_by_name(priv, data + offset, len - offset);
        break;
    case FILE_TRANS_BACK_BY_CLUST:
        g_dev_handle = data[offset++] << 24 | data[offset++] << 16 | data[offset++] << 8 | data[offset++];
        ret = get_file_prepare(g_dev_handle);
        if (ret) {
            break;
        }
        trans_back->OpCode_SN = OpCode_SN;
        trans_back->op = op;
        ret = get_file_by_clust(priv, data + offset, len - offset);
        break;
    case FILE_TRANS_BACK_CANCEL:
        ret = cancel_file_trans_back(OpCode_SN, data + 1, len - 1);
        break;
    }
    if (ret) {
        file_trans_back_response_send(&op, sizeof(op), (u8) - 1, OpCode_SN);
    }
}

#else

void file_trans_back_opt(void *priv, u8 OpCode_SN, u8 *data, u16 len)
{

}

void file_trans_back_close(void)
{

}

#endif
