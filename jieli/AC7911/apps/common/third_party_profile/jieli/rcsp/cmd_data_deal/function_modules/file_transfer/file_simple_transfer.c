#include "rcsp_config.h"
#include "file_simple_transfer.h"
#include "btstack_3th_protocol_user.h"
#include "system/includes.h"
#include "fs/fs.h"
#include "dev_manager.h"///RCSP TODO


#if TCFG_NOR_VM
#include "ui_vm/ui_vm.h"
#endif



#if (RCSP_MODE && TCFG_DEV_MANAGER_ENABLE && JL_RCSP_SIMPLE_TRANSFER)
/* #include "message_vm_cfg.h" *////RCSP TODO

#define FILE_SIMPLE_TRANSFER_VERSION	0

static simple_trans_opt *g_simple_trans_opt = NULL;
#define __this g_simple_trans_opt

enum {
    FILE_SIMPLE_OPT_QUERY,
    FILE_SIMPLE_OPT_READ,
    FILE_SIMPLE_OPT_INSERT,
    FILE_SIMPLE_OPT_UPDATE,
    FILE_SIMPLE_OPT_DELETE,
};


static int file_simple_opt_get_id_table(u8 type, u8 *data[], u8 reserved_len)
{
    int data_len = 0;
    if (NULL == __this) {
        printf("err : please specify the simple file opt interface\n");
        goto __file_simple_opt_get_id_table_end;
    }
    if (!__this->get_id_table_len || !__this->get_id_table) {
        printf("err : get_id_table_len or get_id_table interface is NULL\n");
        goto __file_simple_opt_get_id_table_end;
    }

    data_len = __this->get_id_table_len(type);
    if (!data_len) {
        printf("err : id table is empty\n");
        goto __file_simple_opt_get_id_table_end;
    }

    *data = zalloc(data_len + reserved_len);
    if (NULL == *data) {
        printf("%s is no ram\n", __func__);
        data_len = 0;
        goto __file_simple_opt_get_id_table_end;
    }

    if (!__this->get_id_table(type, *data + reserved_len, data_len)) {
        printf("err : get id table fail\n");
        data_len = 0;
        goto __file_simple_opt_get_id_table_end;
    }

__file_simple_opt_get_id_table_end:
    return data_len;
}

static void file_simple_opt_query(void *priv, u8 OpCode_SN, u8 *data, u16 len)
{
    u16 index = 0;
    u8 file_type = data[0];
    u8 *resp_data = NULL;

    u16 reserved_len = 2;
    int resp_len = file_simple_opt_get_id_table(file_type, &resp_data, reserved_len);
    if (!resp_len) {
        goto __file_simple_opt_query_end;
    }

    index = reserved_len;
    for (u16 i = reserved_len; i < resp_len + reserved_len; i += 4) {
        if (0 == (resp_data[i + 2] | resp_data[i + 3])) {
            continue;
        }

        if (index != i) {
            memcpy(resp_data + index, resp_data + i, 4);
        }

        // id变成大端
        resp_data[index] ^= resp_data[index + 1];
        resp_data[index + 1] ^= resp_data[index];
        resp_data[index] ^= resp_data[index + 1];

        // size变成大端
        resp_data[index + 2] ^= resp_data[index + 3];
        resp_data[index + 3] ^= resp_data[index + 2];
        resp_data[index + 2] ^= resp_data[index + 3];

        index += 4;
    }

__file_simple_opt_query_end:
    if (index > reserved_len) {
        resp_data[0] = FILE_SIMPLE_OPT_QUERY;
        resp_data[1] = FILE_SIMPLE_TRANSFER_VERSION;
        JL_CMD_response_send(JL_OPCODE_SIMPLE_FILE_TRANS, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp_data, index);
    } else {
        // 防止使用resp_data时是空
        data[0] = FILE_SIMPLE_OPT_QUERY;
        data[1] = FILE_SIMPLE_TRANSFER_VERSION;
        JL_CMD_response_send(JL_OPCODE_SIMPLE_FILE_TRANS, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, 2);
    }

    if (resp_data) {
        free(resp_data);
    }
}

static void file_simple_opt_read(void *priv, u8 OpCode_SN, u8 *data, u16 len)
{
    int index = 0;
    u16 offset = 0;
    u8 file_type = data[offset++];
    u16 file_id = data[offset++] << 8 | data[offset++];
    u16 file_offset = data[offset++] << 8 | data[offset++];
    u16 data_len = data[offset++] << 8 | data[offset++];
    u8 package_flag = data[offset++];
    static u16 data_crc = 0;
    // 通过回复把数据返回
    u8 *resp_data = NULL;
    int resp_len = 0;
    if (package_flag) {
        data_crc = 0;
    }
    if (data_len + 1 > (JL_packet_get_tx_max_mtu() - 8)) {
        printf("err : data_len is bigger then resp_max_len\n");
        goto __file_simple_opt_read_end;
    }
    resp_len = file_simple_opt_get_id_table(file_type, &resp_data, 0);
    if (0 == resp_len) {
        goto __file_simple_opt_read_end;
    }

    for (index = 0; index < resp_len; index++) {
        if (0 == memcmp(resp_data + index * 4, &file_id, sizeof(file_id))) {
            break;
        }
    }

    if (index == resp_len) {
        printf("file id : %x is vaild\n", file_id);
        resp_len = 0;
        goto __file_simple_opt_read_end;
    }

    if (0 == (resp_data[index * 4 + 2] | resp_data[index * 4 + 3])) {
        resp_len = 0;
        printf("file type : %x, file id : %d, file data is empty\n", file_type, file_id);
        goto __file_simple_opt_read_end;
    }

    if (__this->read_file_by_id) {
        resp_len = __this->read_file_by_id(file_type, file_id, file_offset, data + 4, data_len);
        if (!resp_len) {
            printf("err : read data fail %d\n", data_len);
            goto __file_simple_opt_read_end;
        } else {
            // 计算crc
            data_len = resp_len;
            printf("===================%s======%d=========yuring==\n\r", __func__, __LINE__);
#if 0 ///RCSP TODO
            data_crc = CRC16_with_initval(data + 4, data_len, data_crc);//calc_crc16_with_init_val(data_crc, data + 4, data_len);
            data[2] = ((u8 *)&data_crc)[1];
            data[3] = ((u8 *)&data_crc)[0];
#endif
        }
    }

__file_simple_opt_read_end:
    data[0] = FILE_SIMPLE_OPT_READ;
    if (resp_len) {
        data[1] = 0;
        JL_CMD_response_send(JL_OPCODE_SIMPLE_FILE_TRANS, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, data_len + 4);
    } else {
        data[1] = 1;
        JL_CMD_response_send(JL_OPCODE_SIMPLE_FILE_TRANS, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, 2);
    }

    if (resp_data) {
        free(resp_data);
    }
}

static void file_simple_opt_insert(void *priv, u8 OpCode_SN, u8 *data, u16 len)
{
    int index = 0;
    u16 offset = 0;
    u8 file_type = data[offset++];
    u16 file_offset = data[offset++] << 8 | data[offset++];
    u16 file_size = data[offset++] << 8 | data[offset++];
    u16 verify_crc = data[offset++] << 8 | data[offset++];
    u16 file_id = 0;
    u8 *file_data = data + offset;
    u8 *resp_data = NULL;

    static u16 data_crc = 0;
    if (0 == file_offset) {
        data_crc = 0;
    }
    printf("===================%s======%d=========yuring==\n\r", __func__, __LINE__);
#if 0 ///RCSP TODO
    data_crc = CRC16_with_initval(file_data, len - offset, data_crc);//calc_crc16_with_init_val(data_crc, file_data, len - offset);
    if (data_crc != verify_crc) {
        printf("err : crc verify is err\n");
        data[1] = 3;
        goto __file_simple_opt_insert_end;
    }
#endif
    if (__this->insert_file_by_id) {
        if (__this->insert_file_by_id(file_type, &file_id, file_offset, file_data, len - offset, file_size)) {
            printf("err : insert data fail\n");
            data[1] = 1;
            goto __file_simple_opt_insert_end;
        }
    }

    data[1] = 0;
    int resp_len = 2;
    if ((file_offset + len - offset) == file_size) {
        // 成功且最后一包
        data[2] = ((u8 *)&file_id)[1];
        data[3] = ((u8 *)&file_id)[0];
        resp_len += 2;
    }

__file_simple_opt_insert_end:
    data[0] = FILE_SIMPLE_OPT_INSERT;
    if (data[1]) {
        resp_len = 2;
    }
    JL_CMD_response_send(JL_OPCODE_SIMPLE_FILE_TRANS, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, resp_len);
    if (resp_data) {
        free(resp_data);
    }
    return;
}

static void file_simple_opt_update(void *priv, u8 OpCode_SN, u8 *data, u16 len)
{
    int index = 0;
    u16 offset = 0;
    u8 file_type = data[offset++];
    u16 file_id = data[offset++] << 8 | data[offset++];
    u16 file_offset = data[offset++] << 8 | data[offset++];
    u16 file_size = data[offset++] << 8 | data[offset++];
    u16 verify_crc = data[offset++] << 8 | data[offset++];
    u8 *file_data = data + offset;
    u8 *resp_data = NULL;
    int resp_len = 0;

    static u16 data_crc = 0;
    if (0 == file_offset) {
        data_crc = 0;
    }

    resp_len = file_simple_opt_get_id_table(file_type, &resp_data, 0);
    if (0 == resp_len) {
        goto __file_simple_opt_update_end;
    }

    for (index = 0; index < resp_len; index++) {
        if (0 == memcmp(resp_data + index * 4, &file_id, sizeof(file_id)))	{
            break;
        }
    }

    if (index == resp_len) {
        printf("file id : %x is vaild fail\n", file_id);
        resp_len = 0;
        goto __file_simple_opt_update_end;
    }

    if (0 == (resp_data[index * 4 + 2] | resp_data[index * 4 + 3])) {
        printf("file type : %x, file id : %d, file data is empty\n", file_type, file_id);
        resp_len = 0;
        goto __file_simple_opt_update_end;
    }
    printf("===================%s======%d=========yuring==\n\r", __func__, __LINE__);
#if 0 ///RCSP TODO
    data_crc = CRC16_with_initval(file_data, len - offset, data_crc);//calc_crc16_with_init_val(data_crc, file_data, len - offset);
    if (data_crc != verify_crc) {
        printf("err : crc verify is err\n");
        data[1] = 3;
        goto __file_simple_opt_update_end;
    }
#endif

    if (__this->update_file_by_id) {
        if (__this->update_file_by_id(file_type, file_id, file_offset, file_data, len - offset, file_size)) {
            printf("err : update file fail\n");
            resp_len = 0;
            goto __file_simple_opt_update_end;
        }
    }

__file_simple_opt_update_end:
    data[0] = FILE_SIMPLE_OPT_UPDATE;
    if (resp_len) {
        data[1] = 0;
    } else {
        data[1] = 1;
    }
    JL_CMD_response_send(JL_OPCODE_SIMPLE_FILE_TRANS, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, 2);
    if (resp_data) {
        free(resp_data);
    }
    return;
}

static void file_simple_opt_delete(void *priv, u8 OpCode_SN, u8 *data, u16 len)
{
    int index = 0;
    u16 offset = 0;
    u8 file_type = data[offset++];
    u16 file_id = data[offset++] << 8 | data[offset++];
    u8 *resp_data = NULL;
    int resp_len = 0;
    resp_len = file_simple_opt_get_id_table(file_type, &resp_data, 0);
    if (0 == resp_len) {
        goto __file_simple_opt_delete_end;
    }
    for (index = 0; index < resp_len; index++) {
        if (0 == memcmp(resp_data + index * 4, &file_id, sizeof(file_id)))	 {
            break;
        }
    }

    if (index == resp_len) {
        printf("file type : %x, file id : %d, file data is null\n", file_type, file_id);
        resp_len = 0;
        goto __file_simple_opt_delete_end;
    }

    if (0 == (resp_data[index * 4 + 2] | resp_data[index * 4 + 3])) {
        printf("file type : %x, file id : %d, file data is empty\n", file_type, file_id);
        resp_len = 0;
        goto __file_simple_opt_delete_end;
    }

    if (__this->delete_file_by_id) {
        if (__this->delete_file_by_id(file_type, file_id)) {
            printf("err : delete file fail\n");
            resp_len = 0;
            goto __file_simple_opt_delete_end;
        }
    }

__file_simple_opt_delete_end:
    data[0] = FILE_SIMPLE_OPT_DELETE;
    if (resp_len) {
        data[1] = 0;
    } else {
        data[1]	 = 1;
    }
    JL_CMD_response_send(JL_OPCODE_SIMPLE_FILE_TRANS, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, 2);
    if (resp_data) {
        free(resp_data);
    }
    return;
}

void file_simple_transfer_for_small_file(void *priv, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 op = data[0];
    switch (op) {
    case FILE_SIMPLE_OPT_QUERY:
        file_simple_opt_query(priv, OpCode_SN, data + 1, len - 1);
        break;
    case FILE_SIMPLE_OPT_READ:
        file_simple_opt_read(priv, OpCode_SN, data + 1, len - 1);
        break;
    case FILE_SIMPLE_OPT_INSERT:
        file_simple_opt_insert(priv, OpCode_SN, data + 1, len - 1);
        break;
    case FILE_SIMPLE_OPT_UPDATE:
        file_simple_opt_update(priv, OpCode_SN, data + 1, len - 1);
        break;
    case FILE_SIMPLE_OPT_DELETE:
        file_simple_opt_delete(priv, OpCode_SN, data + 1, len - 1);
        break;
    }
}

int register_file_simple_transfer_interface(simple_trans_opt *file_simple_transfer_interface)
{
    g_simple_trans_opt = file_simple_transfer_interface;
    return 0;
}





#if 1

static int simple_trans_get_id_table_len(u8 file_type)
{

#if TCFG_NOR_VM
    int ret = 0;
    ret =  flash_common_get_id_table(get_flash_vm_hd(file_type), 0, NULL);
    return ret;
#endif
    return 0;

}

static int simple_trans_get_id_table(u8 file_type, u8 *table_data, u16 data_len)
{
#if TCFG_NOR_VM
    int ret = flash_common_get_id_table(get_flash_vm_hd(file_type), data_len, table_data);
    put_buf(table_data, data_len);
    return ret;
#endif
    return 0;

}

static int simple_trans_read_file_by_id(u8 file_type, u16 id, u32 data_offset, u8 *data, u16 data_len)
{
#if TCFG_NOR_VM
    printf("%s, %d, %d, %d\n", __func__, __LINE__, id, data_len);
    int len = flash_common_read_by_id(get_flash_vm_hd(file_type), id, data_offset, data_len, data);
    put_buf(data, len);
    printf("len =%d data_offset=%d\n", len, data_offset);
    return len;
#endif
    return 0;
}

static int simple_trans_insert_file_by_id(u8 file_type, u16 *id, u32 data_offset, u8 *data, u16 data_len, u16 file_total_size)
{

#if TCFG_NOR_VM

    printf("%s, %d, id %x, data_offset %x, data_len %x, file_total_size %x,type =%d\n", __func__, __LINE__, *id, data_offset, data_len, file_total_size, file_type);


    while (flash_common_get_total(get_flash_vm_hd(file_type)) == get_flash_vm_number_max(file_type)) {
        printf("simple_trans_insert_file_by_id warning\n");
        flash_common_delete_by_index(get_flash_vm_hd(file_type), 0);
    }

    *id = flash_common_write_file(get_flash_vm_hd(file_type), 0, data_len, data);
    if (!(*id)) {
        put_buf(data, data_len);
        return -1;
    }

    if (file_type == F_TYPE_MESSAGE) {
#if (TCFG_UI_ENABLE)
        if (UI_GET_WINDOW_ID() == PAGE_0) {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_0);
        } else {
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_UPGRADE);
        }
#endif /* #if (TCFG_UI_ENABLE) */
    }
    return 0;

#endif
    return 0;
}

static int simple_trans_update_file_by_id(u8 file_type, u16 id, u32 data_offset, u8 *data, u16 data_len, u16 file_total_size)
{
#if TCFG_NOR_VM
    printf("%s, %d, %x, %x, %x %d\n", __func__, __LINE__, id, data_offset, data_len, file_total_size);
    put_buf(data, data_len);
    flash_common_delete_by_id(get_flash_vm_hd(file_type), id);
    int ret =  flash_common_write_file(get_flash_vm_hd(file_type), id, data_len, data);
    if (ret) {
        return 0;
    } else {
        return -1;
    }
    if (file_type == F_TYPE_MESSAGE) {
#if (TCFG_UI_ENABLE)
        if (UI_GET_WINDOW_ID() == PAGE_0) {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_0);
        } else {
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_UPGRADE);
        }
#endif /* #if (TCFG_UI_ENABLE) */
    }

#endif
    return 0;
}

static int simple_trans_delete_file_by_id(u8 file_type, u16 id)
{
#if TCFG_NOR_VM
    printf("%s, %d, %x\n", __func__, __LINE__, id);
    int ret = 0;
    ret = flash_common_delete_by_id(get_flash_vm_hd(file_type), id);
    if (file_type == F_TYPE_MESSAGE) {
#if (TCFG_UI_ENABLE)
        if (UI_GET_WINDOW_ID() == PAGE_0) {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(PAGE_0);
        } else {
            UI_WINDOW_PREEMPTION_POP(ID_WINDOW_UPGRADE);
        }
#endif /* #if (TCFG_UI_ENABLE) */
    }


#endif
    return 0;
}

static simple_trans_opt g_test_trans_opt = {
    .get_id_table_len = simple_trans_get_id_table_len,
    .get_id_table = simple_trans_get_id_table,
    .read_file_by_id = simple_trans_read_file_by_id,
    .insert_file_by_id = simple_trans_insert_file_by_id,
    .update_file_by_id = simple_trans_update_file_by_id,
    .delete_file_by_id = simple_trans_delete_file_by_id,
};

int test_simple_trans(void)
{
    register_file_simple_transfer_interface(&g_test_trans_opt);
    return 0;
}

late_initcall(test_simple_trans);
#endif



#else
void file_simple_transfer_for_small_file(void *priv, u8 OpCode_SN, u8 *data, u16 len)
{

}

int register_file_simple_transfer_interface(simple_trans_opt *file_simple_transfer_interface)
{
    return 0;
}
#endif
