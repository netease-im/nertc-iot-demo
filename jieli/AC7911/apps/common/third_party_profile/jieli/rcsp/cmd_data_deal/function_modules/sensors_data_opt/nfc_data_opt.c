#include "rcsp_config.h"
#include "nfc_data_opt.h"
#include "rcsp_browser.h"
#include "rcsp_event.h"

#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT && JL_RCSP_NFC_DATA_OPT)

#define FTP_DOWNLOAD_FOLDER_NAME				"download" //下载目录

#define WATCH_NFC_MSG_TRANS_BACK				"w_nfc_back"

#define WATCH_NFC_ID_MAX						10

#pragma pack(1)
typedef struct nfc_msg_head_t {
    u16 nfc_file_id;
    u16 reserver;
    u32 dev_hanlder;
    u32 update_time;
    u8  nfc_file_name[6];
    u8  nfc_nick_name[24];
    u16 data_len; // crc + nfc_unique_id
    u16 crc;
} nfc_msg_head;
#pragma pack()

enum {
    NFC_DATA_INFO_SYNC_BEGIN,
    NFC_DATA_INFO_SYNC_END,
    NFC_DATA_INFO_MODIFY,
    NFC_DATA_INFO_DELETE,
    NFC_DATA_INFO_UPDATE,
    NFC_DATA_INFO_DEFAULT_ID,
    NFC_DATA_INFO_INSERT,
    NFC_DATA_INFO_INSERT_FINISH,
};

enum {
    NFC_DATA_INFO_MODIFY_TIME,
    NFC_DATA_INFO_MODIFY_NICE_NAME,
};

enum {
    NFC_DATA_INFO_DEFAULT_ID_GET,
    NFC_DATA_INFO_DEFAULT_ID_SET,
    NFC_DATA_INFO_DEFAULT_ID_NOTIFY,
};

struct __nfc_file_msg {
    struct __dev *dev;
    u32 handler;
    u8 OpCode_SN;
    u8 op;
};

static struct __nfc_file_msg *g_nfc_file_msg = NULL;
static u32 g_nfc_file_index_record = 0;

static int get_nfc_file_prepare(u32 dev_handle)
{
    char *logo = rcsp_browser_dev_remap(dev_handle);
    if (NULL == logo) {
        printf("g_nfc_file_msg logo is null");
        return -1;
    }

    struct __dev *dev  = dev_manager_find_spec(logo, 0);
    if (NULL == dev) {
        printf("g_nfc_file_msg dev is null");
        return -1;
    }

    if (NULL == g_nfc_file_msg) {
        g_nfc_file_msg = zalloc(sizeof(struct __nfc_file_msg));
        if (NULL == g_nfc_file_msg) {
            printf("g_nfc_file_msg zalloc err\n");
            return -1;
        }
        g_nfc_file_msg->dev = dev;
    }
    return 0;
}

static int nfc_id_default_get_from_vm(u8 *data, u16 data_len)
{
    if (data_len != syscfg_read(VM_WATCH_DEFAULT_NFC_ID, data, data_len)) {
        printf("%s, read vm fail\n", __func__);
        return -1;
    }
    return 0;
}

static int nfc_id_default_set_to_vm(u8 *data, u16 data_len)
{
    u8 *tmp_data = zalloc(data_len);
    if (NULL == tmp_data) {
        printf("%s, no enough memory!!\n", __func__);
        goto __nfc_id_default_set_to_vm_end;
    }
    if (nfc_id_default_get_from_vm(tmp_data, data_len)) {
        memset(tmp_data, 0xff, data_len);
    }
    if (0 == memcmp(data, tmp_data, data_len)) {
        goto __nfc_id_default_set_to_vm_end;
    }
    syscfg_write(VM_WATCH_DEFAULT_NFC_ID, data, data_len);

__nfc_id_default_set_to_vm_end:
    if (tmp_data) {
        free(tmp_data);
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
    if (name) {
        memcpy(path + strlen(path), name, name_len);
    }
}

static u32 creat_path_len(char *root_path, const char *folder, u8 *name, u16 name_len)
{
    u32 len = (strlen(root_path) + strlen(folder) + name_len + 1);
    if (folder) {
        len += strlen("/");
    }
    return len;
}

static int get_nfc_file_path(u8 *path[], u8 file_index, u8 *file_name)
{
    if (NULL == g_nfc_file_msg) {
        printf("%s, g_nfc_file_msg is null\n", __func__);
        goto __get_nfc_file_path_err;
    }

    char *root_path = dev_manager_get_root_path(g_nfc_file_msg->dev);
    char *folder = NULL;
    if (BS_SD1 == g_nfc_file_msg->handler) {
        folder = FTP_DOWNLOAD_FOLDER_NAME;
    }
    char tmp_name[] = "00.nfc";

    if (*path) {
        free(*path);
        *path = NULL;
    }

    if (NULL == *path) {
        if (file_name) {
            *path = zalloc(creat_path_len(root_path, folder, file_name, strlen(file_name) + 1));
        } else {
            *path = zalloc(creat_path_len(root_path, folder, tmp_name, strlen(tmp_name) + 1));
        }
    }

    if (NULL == *path) {
        printf("%s, no enough memory!!\n", __func__);
        goto __get_nfc_file_path_err;
    }
    if (file_index) {
        tmp_name[0] = (file_index / 10) + '0';
        tmp_name[1] = (file_index % 10) + '0';
    }

    if (file_name) {
        creat_file_path(*path, root_path, folder, file_name, strlen(file_name));
    } else {
        creat_file_path(*path, root_path, folder, tmp_name, strlen(tmp_name));
    }

    return 0;

__get_nfc_file_path_err:
    if (*path) {
        free(*path);
        *path = NULL;
    }
    return -1;
}

static u8 file_trans_back_response_send(u8 *data, u16 len, u8 dire, u8 OpCode_SN)
{
    u8 ret = 0;
    switch (dire) {
    case 0:
        ret = JL_CMD_response_send(JL_OPCODE_SENSOR_NFC_FUNCTION_OPT, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, len);
        break;
    case 1:
        ret = JL_DATA_send(JL_OPCODE_DATA, JL_OPCODE_SENSOR_NFC_FUNCTION_OPT, data, len, JL_NOT_NEED_RESPOND);
        break;
    case 2:
        ret = JL_CMD_send(JL_OPCODE_SENSOR_NFC_FUNCTION_OPT, data, len, JL_NOT_NEED_RESPOND);
        break;
    case 0xFF:
        ret = JL_CMD_response_send(JL_OPCODE_SENSOR_NFC_FUNCTION_OPT, JL_PRO_STATUS_FAIL, OpCode_SN, data, len);
        break;
    }
    return ret;
}

static int nfc_file_read(char *path, u8 *buffer, u16 buffer_size, u8 offset, u8 *reason, u8 *file_index_record)
{
    u8 ret = 0;
    FILE *file = fopen(path, "r");
    if (NULL == file) {
        printf("%s, file open fail\n", __func__);
        goto __nfc_file_read_end;
    }

    u32 nfc_data_len = sizeof(nfc_msg_head) - 2;

    if (nfc_data_len > buffer_size - offset) {
        ret = file_trans_back_response_send(buffer, offset, 1, g_nfc_file_msg->op);
        offset = 0;
        if (ret) {
            // 错误
            printf("%s, send data err : %d, %d", __func__, ret, offset);
            *reason = ret;
            goto __nfc_file_read_end;
        }
    }

    nfc_msg_head *nfc_msg = (nfc_msg_head *)(buffer + offset);
    offset += fread(file, buffer + offset, nfc_data_len);

    u8 crc_uuid_len = ((u8 *)&nfc_msg->data_len)[0] << 8 | ((u8 *)&nfc_msg->data_len)[1];

    if (crc_uuid_len > buffer_size - offset) {
        ret = file_trans_back_response_send(buffer, offset, 1, g_nfc_file_msg->op);
        offset = 0;
        if (ret) {
            // 错误
            printf("%s, send data err : %d, %d", __func__, ret, offset);
            *reason = ret;
            goto __nfc_file_read_end;
        }
    }
    fseek(file, nfc_data_len, SEEK_SET);
    offset += fread(file, buffer + offset, crc_uuid_len);
    *file_index_record = 1;

__nfc_file_read_end:
    if (file) {
        fclose(file);
        file = NULL;
    }
    return offset;
}

static void nfc_file_trans_back_close(void)
{
    task_kill(WATCH_NFC_MSG_TRANS_BACK);

    if (g_nfc_file_msg) {
        free(g_nfc_file_msg);
        g_nfc_file_msg = NULL;
    }
}

// 返回大于0是有默认值，等于0是没有默认值，-1是错误
static u16 nfc_file_exist_check(u8 *file_index_record, u16 record_len)
{
    // 读出nfc_id，如果读取失败则返回
    u16 tmp_nfc_id = 0;
    if (nfc_id_default_get_from_vm(&tmp_nfc_id, sizeof(tmp_nfc_id))) {
        return -1;
    }
    // 这种情况不可能也不应该出现
    if (tmp_nfc_id >= record_len) {
        printf("err : %s vaild !!\n");
        return -1;
    }
    // 判断file_index_record对用nfc_id的位置有没有值，有则返回
    if (file_index_record[tmp_nfc_id]) {
        return tmp_nfc_id + 1;
    }
    // 如果没有值，取最接近的上一个
    for (u16 i = tmp_nfc_id; i > 0; i--) {
        u16 tmp_index = i - 1;
        if (file_index_record[tmp_index]) {
            nfc_id_default_set_to_vm(&tmp_index, sizeof(tmp_index));
            return tmp_index + 1;
        }
    }
    // 如果还是没有，取最接近的下一个
    for (u16 i = tmp_nfc_id + 1; i < record_len; i++) {
        if (file_index_record[i]) {
            nfc_id_default_set_to_vm(&i, sizeof(i));
            return i + 1;
        }
    }
    // 如果file_index_record里的值是全0，则把nfc_id对应的vm值设置为0
    tmp_nfc_id = 0;
    nfc_id_default_set_to_vm(&tmp_nfc_id, sizeof(tmp_nfc_id));
    return 0;
}

static void nfc_file_trans_back_task(void *p)
{
    u8 ret = 0;
    u8 reason = 0;
    u8 offset = 0;
    u8 *path = NULL;
    // 回复当前命令
    u16 resp_data_len = JL_packet_get_tx_max_mtu() - 6;
    u8 *resp_data = zalloc(resp_data_len);
    u8 *file_index_record = zalloc(WATCH_NFC_ID_MAX);

    if (NULL == resp_data || NULL == file_index_record) {
        reason = -1;
        file_trans_back_response_send(NULL, 0, (u8) - 1, g_nfc_file_msg->OpCode_SN);
        goto __nfc_file_trans_back_task_err;
    }

    file_trans_back_response_send(&ret, sizeof(ret), 0, g_nfc_file_msg->OpCode_SN);

    g_nfc_file_index_record = 0;
    for (u8 i = 0; i < WATCH_NFC_ID_MAX; i++) {
        // 读取文件:01~09.nfc
        if (get_nfc_file_path(&path, i, NULL)) {
            reason = -1;
            break;
        }
        offset = nfc_file_read(path, resp_data, resp_data_len, offset, &reason, file_index_record + i);
        if (reason) {
            break;
        }
        if (file_index_record[i]) {
            g_nfc_file_index_record |= BIT(i);
        }
    }

    if (offset) {
        // 发送
        if (file_trans_back_response_send(resp_data, offset, 1, g_nfc_file_msg->op)) {
            printf("%s, send data err : %d, %d", __func__, ret, offset);
            reason = -1;
        }
    }

    nfc_file_exist_check(file_index_record, WATCH_NFC_ID_MAX);

__nfc_file_trans_back_task_err:
    if (path) {
        free(path);
    }

    if (resp_data) {
        free(resp_data);
    }

    if (file_index_record) {
        free(file_index_record);
    }

    // 发送事件结束
    rcsp_msg_post(
        USER_MSG_RCSP_NFC_FILE_TRANS_BACK,
        5,
        (int)p,
        (int)g_nfc_file_msg->handler,
        NFC_DATA_INFO_SYNC_BEGIN,
        (int)reason, 0);

    while (1) {
        os_time_dly(10);
    }
}

static int nfc_data_msg_modify(void *priv, u8 *data, u16 len)
{
    u8 ret = 0;
    u8 offset = 0;
    u8 *path = NULL;
    FILE *file = NULL;
    u8 file_index = data[offset++] * 10 + data[offset++];
    if (get_nfc_file_path(&path, file_index, NULL)) {
        ret = -1;
        goto __nfc_data_msg_modify_end;
    }
    file = fopen(path, "r");
    if (file) {
        fclose(file);
        file = NULL;
    } else {
        ret = -1;
        printf("%s, file open err\n", __func__);
        goto __nfc_data_msg_modify_end;
    }

    file = fopen(path, "w+");
    if (NULL == file) {
        ret = -1;
        printf("%s, file open err\n", __func__);
        goto __nfc_data_msg_modify_end;
    }

    nfc_msg_head nfc_msg = {0};
    fread(file, &nfc_msg, sizeof(nfc_msg));

    u32 mask = data[offset++] << 24 | data[offset++] << 16 | data[offset++] << 8 | data[offset++];

    // 假如是时间戳
    if (mask & BIT(NFC_DATA_INFO_MODIFY_TIME)) {
        memcpy(&nfc_msg.update_time, data + offset, sizeof(nfc_msg.update_time));
        offset += sizeof(nfc_msg.update_time);
    }

    // 假如是别名
    if (mask & BIT(NFC_DATA_INFO_MODIFY_NICE_NAME)) {
        if (len - offset > sizeof(nfc_msg.nfc_nick_name)) {
            printf("err : remote nick name len is vaild\n");
            ret = -1;
            goto __nfc_data_msg_modify_end;
        }
        memset(nfc_msg.nfc_nick_name, 0, sizeof(nfc_msg.nfc_nick_name));
        memcpy(nfc_msg.nfc_nick_name, data + offset, len - offset);
    }

    int nfc_total_len = flen(file);
    fseek(file, 0, SEEK_SET);
    fwrite(file, &nfc_msg, sizeof(nfc_msg));
    fseek(file, nfc_total_len, SEEK_SET);

__nfc_data_msg_modify_end:
    if (path) {
        free(path);
        path = NULL;
    }

    if (file) {
        fclose(file);
        file = NULL;
    }

    rcsp_msg_post(
        USER_MSG_RCSP_NFC_FILE_TRANS_BACK,
        5,
        (int)priv,
        (int)g_nfc_file_msg->handler,
        NFC_DATA_INFO_MODIFY,
        (int)ret,
        (int)file_index);
    return 0;
}

static int nfc_data_msg_default_id_get(u16 *default_nfc_id)
{
    int ret = 0;
    u8 *file_index_record = zalloc(WATCH_NFC_ID_MAX);
    if (NULL == file_index_record) {
        printf("%s, no enough memory!!\n", __func__);
        ret = -1;
        goto __nfc_data_msg_default_id_get_end;
    }

    for (u8 i = 0; i < WATCH_NFC_ID_MAX && i < sizeof(g_nfc_file_index_record); i++) {
        file_index_record[i] = !!(g_nfc_file_index_record & BIT(i));
    }
    u16 tmp_nfc_id = nfc_file_exist_check(file_index_record, WATCH_NFC_ID_MAX);
    if (0 == tmp_nfc_id || (u16) - 1 == tmp_nfc_id) {
        ret = -1;
        goto __nfc_data_msg_default_id_get_end;
    }
    *default_nfc_id = tmp_nfc_id - 1;

__nfc_data_msg_default_id_get_end:
    if (file_index_record) {
        free(file_index_record);
    }
    return ret;
}

static int nfc_data_msg_delete(void *priv, u8 *data, u16 len)
{
    u8 ret = 0;
    u8 offset = 0;
    u8 file_index = data[offset++] * 10 + data[offset++];
    u8 *path = NULL;
    FILE *file = NULL;
    if (get_nfc_file_path(&path, file_index, NULL)) {
        ret = -1;
        goto __nfc_data_msg_delete_end;
    }

    if ((file = fopen(path, "r"))) {
        if (fdelete(file)) {
            //错误
            printf("%s, delete fail\n", __func__);
            ret = -1;
            goto __nfc_data_msg_delete_end;
        }
        file = NULL;
        g_nfc_file_index_record &= ~BIT(file_index);
    }

__nfc_data_msg_delete_end:
    if (path) {
        free(path);
        path = NULL;
    }

    if (file) {
        fclose(file);
        file = NULL;
    }

    rcsp_msg_post(
        USER_MSG_RCSP_NFC_FILE_TRANS_BACK,
        5,
        (int)priv,
        (int)g_nfc_file_msg->handler,
        NFC_DATA_INFO_DELETE,
        (int)ret,
        (int)file_index);
    return 0;
}

static int nfc_data_msg_insert(void *priv, u8 *data, u16 len)
{
    int ret = 0;
    u8 offset = 0;
    u8 *path = NULL;
    FILE *file = NULL;
    u8 file_index = 0;
    // 可以用于判断FLASH/SD卡容量是否足够
    u32 file_size = data[offset++] << 24 | data[offset++] << 16 | data[offset++] << 8 | data[offset++];
    u8 *nfc_file_name = zalloc(len - offset + 1);
    if (NULL == nfc_file_name) {
        printf("%s, no enough memory!!\n", __func__);
        ret = -1;
        goto __nfc_data_msg_insert_end;
    }
    memcpy(nfc_file_name, data + offset, len - offset);
    file_index = (nfc_file_name[0] - '0') * 10 + nfc_file_name[1] - '0';
    ret = get_nfc_file_path(&path, 0, nfc_file_name);
    if (ret) {
        goto __nfc_data_msg_insert_end;
    }

    if ((file = fopen(path, "r"))) {
        if (fdelete(file)) {
            printf("%s, delete fail\n", __func__);
            ret = -1;
            goto __nfc_data_msg_insert_end;
        }
        file = NULL;
    }

__nfc_data_msg_insert_end:
    if (file) {
        fclose(file);
    }
    if (path) {
        free(path);
    }
    if (nfc_file_name) {
        free(nfc_file_name);
    }

    rcsp_msg_post(
        USER_MSG_RCSP_NFC_FILE_TRANS_BACK,
        5,
        (int)priv,
        (int)g_nfc_file_msg->handler,
        NFC_DATA_INFO_INSERT,
        (int)ret,
        (int)file_index);
    return 0;
}

static int nfc_data_msg_insert_finish(void *priv, u8 *data, u16 len)
{
    int ret = 0;
    u8 *path = NULL;
    FILE *file = NULL;
    u16 nfc_id = 0;
    u8 *nfc_file_name = zalloc(len + 1);
    if (NULL == nfc_file_name) {
        printf("%s, no enough memory!!\n", __func__);
        ret = -1;
        goto __nfc_data_msg_insert_finish_end;
    }
    memcpy(nfc_file_name, data, len);
    ret = get_nfc_file_path(&path, 0, nfc_file_name);
    if (ret) {
        goto __nfc_data_msg_insert_finish_end;
    }

    nfc_id = (nfc_file_name[0] - '0') * 10 + (nfc_file_name[1] - '0');
    if ((file = fopen(path, "r"))) {
        // 读出数据
        // 把数据写给nfc外设
        g_nfc_file_index_record |= BIT(nfc_id);
    }

__nfc_data_msg_insert_finish_end:
    if (file) {
        fclose(file);
    }
    if (path) {
        free(path);
    }
    if (nfc_file_name) {
        free(nfc_file_name);
    }
    rcsp_msg_post(
        USER_MSG_RCSP_NFC_FILE_TRANS_BACK,
        5,
        (int)priv,
        (int)g_nfc_file_msg->handler,
        NFC_DATA_INFO_INSERT_FINISH,
        (int)ret,
        (int)nfc_id);
    return ret;
}

static int nfc_id_default_opt(void *priv, u8 *data, u16 len, u8 OpCode_SN)
{
    u8 offset = 0;
    u8 flag = data[offset++];
    u8 resp_data[3] = {0};
    u8 resp_data_len = 0;
    u8 file_index = 0;
    u8 op = NFC_DATA_INFO_DEFAULT_ID;
    memcpy(resp_data + 1, data + offset, 2);
    switch (flag) {
    case NFC_DATA_INFO_DEFAULT_ID_GET:
        printf("NFC_DATA_INFO_DEFAULT_ID_GET\n");
        // 把id写入vm
        u16 default_nfc_id = 0;
        if (nfc_id_default_get_from_vm(&default_nfc_id, sizeof(default_nfc_id))) {
            resp_data[0] = -1;
            break;
        }
        resp_data[1] = ((u8 *)&default_nfc_id)[1];
        resp_data[2] = ((u8 *)&default_nfc_id)[0];
        resp_data_len = 3;
        op = -1;
        file_trans_back_response_send(resp_data, resp_data_len, 0, OpCode_SN);
        break;
    case NFC_DATA_INFO_DEFAULT_ID_SET:
        printf("NFC_DATA_INFO_DEFAULT_ID_SET\n");
        u16 nfc_id = data[offset++] << 8 | data[offset++];
        if (nfc_id > WATCH_NFC_ID_MAX - 1) {
            printf("%s, nfc id is vaild\n", __func__);
            resp_data[0] = -1;
        }
        nfc_id_default_set_to_vm(&nfc_id, sizeof(nfc_id));
        resp_data_len = 1;
        file_index = nfc_id;
        break;
    }
    rcsp_msg_post(
        USER_MSG_RCSP_NFC_FILE_TRANS_BACK,
        5,
        (int)priv,
        (int)g_nfc_file_msg->handler,
        op,
        (int)resp_data[0],
        (int)file_index);
    return 0;
}

static int nfc_data_opt_check(u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 offset = 0;
    u32 handle = data[offset++] << 24 | data[offset++] << 16 | data[offset++] << 8 | data[offset++];
    int ret = get_nfc_file_prepare(handle);
    if (0 == ret) {
        g_nfc_file_msg->op = OpCode;
        g_nfc_file_msg->OpCode_SN = OpCode_SN;
        g_nfc_file_msg->handler = handle;
    }
    return ret;
}

static int nfc_data_opt(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 op = 0;
    u8 offset = 0;
    int ret = nfc_data_opt_check(OpCode, OpCode_SN, data + offset, len - offset);
    if (ret) {
        goto __nfc_data_opt_end;
    }
    offset += 4;
    op = data[offset++];
    switch (op) {
    case NFC_DATA_INFO_SYNC_BEGIN:
        if (task_create(nfc_file_trans_back_task, priv, WATCH_NFC_MSG_TRANS_BACK)) {
            ret = -1;
            nfc_file_trans_back_close();
            goto __nfc_data_opt_end;
        }
        break;
    case NFC_DATA_INFO_MODIFY:
        nfc_data_msg_modify(priv, data + offset, len - offset);
        break;
    case NFC_DATA_INFO_DELETE:
        nfc_data_msg_delete(priv, data + offset, len - offset);
        break;
    case NFC_DATA_INFO_DEFAULT_ID:
        nfc_id_default_opt(priv, data + offset, len - offset, OpCode_SN);
        break;
    case NFC_DATA_INFO_INSERT:
        nfc_data_msg_insert(priv, data + offset, len - offset);
        break;
    case NFC_DATA_INFO_INSERT_FINISH:
        nfc_data_msg_insert_finish(priv, data + offset, len - offset);
        break;
    }

__nfc_data_opt_end:
    if (ret) {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, (u8 *)&ret, 1);
        file_trans_back_response_send(&ret, 1, 0, OpCode_SN);
    }
    return ret;
}

int nfc_id_default_notify(u32 devHeadler, u16 nfc_id)
{
    int ret = 0;
    u8 resp_data[] = {((u8 *)&devHeadler)[3], ((u8 *)&devHeadler)[2], ((u8 *)&devHeadler)[1], ((u8 *)&devHeadler)[0], NFC_DATA_INFO_DEFAULT_ID, NFC_DATA_INFO_DEFAULT_ID_NOTIFY, ((u8 *)&nfc_id)[1], ((u8 *)&nfc_id)[0]};
    if (nfc_id > WATCH_NFC_ID_MAX - 1) {
        ret = -1;
        goto __nfc_id_default_notify_end;
    }

    // 从vm中读取nfc_id
    u16 default_nfc_id = -1;
    ret = file_trans_back_response_send(resp_data, sizeof(resp_data), 2, 0);
    nfc_id_default_set_to_vm(&nfc_id, sizeof(nfc_id));
__nfc_id_default_notify_end:
    return ret;
}

static void nfc_peripherals_deal(u32 handler, u8 file_index)
{
    u8 *path = NULL;
    u8 *nfc_data = NULL;
    FILE *file = NULL;
    if (get_nfc_file_path(&path, file_index, NULL)) {
        goto __nfc_peripherals_deal_end;
    }
    file = fopen(path, "r");
    if (NULL == file) {
        printf("%s, file open fail\n", __func__);
        goto __nfc_peripherals_deal_end;
    }
    // 获取当前文件对应的数据
    nfc_msg_head nfc_msg = {0};
    fread(file, (u8 *)&nfc_msg, sizeof(nfc_msg));
    int nfc_id_crc_len = ((u8 *)&nfc_msg.data_len)[0] << 8 | ((u8 *)&nfc_msg.data_len)[1];
    int nfc_data_len = flen(file);
    nfc_data_len = nfc_data_len - (sizeof(nfc_msg) - 2) - nfc_id_crc_len;
    if (nfc_data_len <= 0) {
        goto __nfc_peripherals_deal_end;
    }

    nfc_data = zalloc(nfc_data_len);
    if (NULL == nfc_data) {
        printf("%s, file open fail\n", __func__);
        goto __nfc_peripherals_deal_end;
    }

    fseek(file, (sizeof(nfc_msg) - 2) + nfc_id_crc_len, SEEK_SET);
    fread(file, nfc_data, nfc_data_len);
    // 把数据写到nfc外设中

__nfc_peripherals_deal_end:
    if (path) {
        free(path);
    }
    if (nfc_data) {
        free(nfc_data);
    }
    if (file) {
        fclose(file);
    }
}

void nfc_file_trans_back_end(void *priv, u32 handler, u8 op, int result, int param)
{
    u8 ret = (u8)result;
    u8 file_index = (u8) param;
    if (NFC_DATA_INFO_SYNC_BEGIN != op && (u8) - 1 != op) {
        file_trans_back_response_send(&ret, sizeof(ret), 0, g_nfc_file_msg->OpCode_SN);
    }

    switch (op) {
    case NFC_DATA_INFO_SYNC_BEGIN:
        printf("NFC_DATA_INFO_SYNC_BEGIN end\n");
        u8 resp_data[2] = {NFC_DATA_INFO_SYNC_END, ret};
        file_trans_back_response_send(resp_data, sizeof(resp_data), 2, 0);
        break;
    case NFC_DATA_INFO_MODIFY:
        printf("NFC_DATA_INFO_MODIFY end\n");
        break;
    case NFC_DATA_INFO_DELETE:
        printf("NFC_DATA_INFO_DELETE end\n");
        // 获取默认id
        u16 default_nfc_id = 0;
        if (0 == ret && 0 == nfc_data_msg_default_id_get(&default_nfc_id)) {
            nfc_id_default_notify(handler, default_nfc_id);
        }
        break;
    case NFC_DATA_INFO_DEFAULT_ID:
        printf("NFC_DATA_INFO_DEFAULT_ID end\n");
        if (0 == ret) {
            // 写入外设
            nfc_peripherals_deal(handler, file_index);
        }
        break;
    case NFC_DATA_INFO_INSERT:
        printf("NFC_DATA_INFO_INSERT end\n");
        // 插入开始，可以做对应得初始化动作
        break;
    case NFC_DATA_INFO_INSERT_FINISH:
        printf("NFC_DATA_INFO_INSERT_FINISH end\n");
        // 插入结束，这个时候已经存在文件，可以读出来写入外设
        if (0 == ret) {
            // 写入外设
        }
        break;
    }
    nfc_file_trans_back_close();
}

int JL_rcsp_nfc_data_funciton(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    int ret = -1;
    switch (OpCode) {
    case JL_OPCODE_SENSOR_NFC_FUNCTION_OPT:
        ret = 0;
        nfc_data_opt(priv, OpCode, OpCode_SN, data, len);
        break;
    }
    return ret;
}

#else

int JL_rcsp_nfc_data_funciton(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    return -1;
}

void nfc_file_trans_back_end(void *priv, u32 handler, u8 op, int result, int param)
{

}

int nfc_id_default_notify(u32 devHeadler, u16 nfc_id)
{
    return 0;
}
#endif
